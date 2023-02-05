#include "CapturedIrp.hpp"

#include "Context.hpp"
#include "Native.hpp"
#include "Utils.hpp"

namespace CFB::Driver
{

CapturedIrp::CapturedIrp(const CapturedIrp::IrpType Type, PDEVICE_OBJECT DeviceObject) :
    m_Valid {false},
    m_Type {Type},
    m_Pid {::HandleToULong(::PsGetCurrentProcessId())},
    m_Tid {::HandleToULong(::PsGetCurrentThreadId())},
    m_Driver {nullptr},
    m_DeviceObject {nullptr},
    m_DriverName {},
    m_DeviceName {},
    m_ProcessName {},
    m_MajorFunction {0},
    m_MinorFunction {0},
    m_IoctlCode {0},
    m_InputBuffer {0},
    m_OutputBuffer {0},
    Next {}
{
    auto FilterByDeviceAddress = [&DeviceObject](const HookedDriver* h)
    {
        for ( PDEVICE_OBJECT CurrentDevice = h->OriginalDriverObject->DeviceObject; CurrentDevice;
              CurrentDevice                = CurrentDevice->NextDevice )
        {
            if ( CurrentDevice == DeviceObject )
            {
                return true;
            }
        }
        return false;
    };

    m_Driver = Globals->DriverManager.Items().Find(FilterByDeviceAddress);
    if ( m_Driver == nullptr )
    {
        //
        // This is really bad: if here the interception routine got called by a non-hooked driver
        // Could be a bad pointer restoration. Anyway, we log and fail for now.
        //
        err("Failed to find a HookedDriver object associated to the received IRP.\n"
            "This could indicates a corruption of the hooked driver list, you should probably reboot...");
        return;
    }

    m_DeviceObject = DeviceObject;

    KeQuerySystemTime(&m_TimeStamp);

    //
    // Set the Device name
    //
    {
        ULONG ReturnLength;
        Utils::KAlloc<POBJECT_NAME_INFORMATION> DeviceNameInfo;

        //
        // Query the exact size
        //
        NTSTATUS Status = ::ObQueryNameString(DeviceObject, nullptr, 0, &ReturnLength);
        if ( Status != STATUS_INFO_LENGTH_MISMATCH )
        {
            err("CapturedIrp() failed with %#08x", Status);
            return;
        }

        //
        // Get the value, store it in the member
        //
        DeviceNameInfo = Utils::KAlloc<POBJECT_NAME_INFORMATION>(sizeof(OBJECT_NAME_INFORMATION) + ReturnLength);
        Status         = ::ObQueryNameString(DeviceObject, DeviceNameInfo.get(), DeviceNameInfo.size(), &ReturnLength);
        if ( !NT_SUCCESS(Status) )
        {
            err("CapturedIrp() failed with %#08x", Status);
            return;
        }


        const PUNICODE_STRING pUsDevice = &DeviceNameInfo.get()->Name;
        m_DeviceName                    = Utils::KUnicodeString(pUsDevice);
    }

    //
    // Set the driver name
    //
    {
        const PUNICODE_STRING pUsDriver = m_Driver->Path.get();
        m_DriverName                    = Utils::KUnicodeString(pUsDriver);
    }

    //
    // Set the process name
    //
    {
        PEPROCESS Process = nullptr;
        NTSTATUS Status   = ::PsLookupProcessByProcessId(UlongToHandle(m_Pid), &Process);
        if ( !NT_SUCCESS(Status) )
        {
            return;
        }

        PSTR lpProcessName = ::PsGetProcessImageFileName(Process);
        if ( !lpProcessName )
        {
            return;
        }

        CANSI_STRING aStr;
        Status = ::RtlInitAnsiStringEx(&aStr, lpProcessName);
        if ( !NT_SUCCESS(Status) )
        {
            return;
        }

        UNICODE_STRING uStr;
        Status = ::RtlAnsiStringToUnicodeString(&uStr, &aStr, true);
        if ( !NT_SUCCESS(Status) )
        {
            return;
        }

        m_ProcessName = Utils::KUnicodeString(uStr.Buffer, uStr.Length);
    }

    //
    // Mark the object as valid
    //
    m_Valid = true;
}

CapturedIrp::~CapturedIrp()
{
    dbg("CapturedIrp::~CapturedIrp ");
}


NTSTATUS
CapturedIrp::CapturePreCallData(_In_ PIRP Irp)
{
    if ( !m_Valid )
    {
        warn("CapturedIRP was insufficiently initialized");
        return STATUS_ACCESS_DENIED;
    }

    NTSTATUS Status          = STATUS_SUCCESS;
    ULONG Method             = -1;
    PIO_STACK_LOCATION Stack = ::IoGetCurrentIrpStackLocation(Irp);

    ULONG InputBufferLength  = 0;
    ULONG OutputBufferLength = 0;

    dbg("CapturedIrp::CapturePreCallData(%p)", Irp);

    m_MajorFunction = Stack->MajorFunction;
    m_MinorFunction = Stack->MinorFunction;

    //
    // Determine & allocate the input/output buffer sizes from the IRP for "normal" IOCTLs
    //
    switch ( m_MajorFunction )
    {
    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        InputBufferLength  = Stack->Parameters.DeviceIoControl.InputBufferLength;
        OutputBufferLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;
        m_IoctlCode        = Stack->Parameters.DeviceIoControl.IoControlCode;
        m_InputBuffer      = Utils::KAlloc<u8*>(InputBufferLength);
        m_OutputBuffer     = Utils::KAlloc<u8*>(OutputBufferLength);
        Method             = METHOD_FROM_CTL_CODE(m_IoctlCode);
        break;

    case IRP_MJ_WRITE:
        InputBufferLength = Stack->Parameters.Write.Length;
        m_InputBuffer     = Utils::KAlloc<u8*>(InputBufferLength);
        break;

    case IRP_MJ_READ:
        OutputBufferLength = Stack->Parameters.Read.Length;
        m_OutputBuffer     = Utils::KAlloc<u8*>(OutputBufferLength);
        Method             = (m_DeviceObject->Flags & DO_BUFFERED_IO) ? METHOD_BUFFERED :
                             (m_DeviceObject->Flags & DO_DIRECT_IO)   ? METHOD_IN_DIRECT :
                                                                        -1;
        break;

    default:
        return STATUS_SUCCESS;
    }

    //
    // Now, copy the input/output buffer content to the CapturedIrp object
    //
    do
    {
        if ( (m_MajorFunction == IRP_MJ_DEVICE_CONTROL || m_MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
             Method == METHOD_NEITHER )
        {
            if ( Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16) )
                RtlCopyMemory(
                    m_InputBuffer.get(),
                    Stack->Parameters.DeviceIoControl.Type3InputBuffer,
                    m_InputBuffer.size());
            else
                Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if ( Method == METHOD_BUFFERED )
        {
            if ( Irp->AssociatedIrp.SystemBuffer )
                ::RtlCopyMemory(m_InputBuffer.get(), Irp->AssociatedIrp.SystemBuffer, m_InputBuffer.size());
            else
                Status = STATUS_INVALID_PARAMETER_1;
            break;
        }

        if ( Method == METHOD_IN_DIRECT || Method == METHOD_OUT_DIRECT )
        {
            if ( !Irp->MdlAddress )
            {
                Status = STATUS_INVALID_PARAMETER_2;
                break;
            }

            PVOID pDataAddr = ::MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
            if ( !pDataAddr )
            {
                Status = STATUS_INVALID_PARAMETER_3;
                break;
            }

            RtlCopyMemory(m_InputBuffer.get(), pDataAddr, m_InputBuffer.size());

            ok("Capturing input data:");
            CFB::Utils::Hexdump(m_InputBuffer.get(), m_InputBuffer.size());
        }

    } while ( false );

    return Status;
}


NTSTATUS
CapturedIrp::CapturePostCallData(_In_ PIRP Irp, _In_ NTSTATUS ReturnedIoctlStatus)
{
    if ( !m_Valid )
    {
        warn("CapturedIRP was insufficiently initialized");
        return STATUS_ACCESS_DENIED;
    }

    PVOID UserBuffer = nullptr;
    m_Status         = ReturnedIoctlStatus;

    //
    // If there's nothing to capture, leave now
    //
    if ( m_OutputBuffer.size() == 0 )
    {
        return STATUS_SUCCESS;
    }

    //
    // Check if the operation supports having output buffer
    //
    usize Count {m_OutputBuffer.size()}, Offset {0};
    switch ( m_MajorFunction )
    {
    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        UserBuffer = Irp->UserBuffer;
        break;

    case IRP_MJ_READ:
        if ( Irp->MdlAddress )
        {
            UserBuffer = ::MmGetSystemAddressForMdlSafe(
                Irp->MdlAddress,
                NormalPagePriority | MdlMappingNoWrite | MdlMappingNoExecute);
            Count  = Irp->MdlAddress->ByteCount;
            Offset = Irp->MdlAddress->ByteOffset;
        }
        else
        {
            UserBuffer = Irp->UserBuffer;
        }
        break;

    default:
        return STATUS_SUCCESS;
    }

    //
    // Check if there's actual data to be copied
    //
    if ( UserBuffer == nullptr )
    {
        return STATUS_SUCCESS;
    }

    dbg("Copying %p <- %p (%luB)", m_OutputBuffer.get() + Offset, UserBuffer, Count);

    //
    // If here, just copy the buffer
    //
    RtlCopyMemory(m_OutputBuffer.get() + Offset, UserBuffer, Count);

#ifdef _DEBUG
    ok("Capturing output data:");
    CFB::Utils::Hexdump(m_OutputBuffer.get(), m_OutputBuffer.size());
#endif // _DEBUG

    return STATUS_SUCCESS;
}


NTSTATUS
CapturedIrp::CapturePreCallFastIoData(_In_ PVOID InputBuffer, _In_ ULONG IoControlCode)
{
    if ( !m_Valid )
    {
        warn("CapturedIRP was insufficiently initialized");
        return STATUS_ACCESS_DENIED;
    }

    m_IoctlCode = IoControlCode;
    ::RtlCopyMemory(m_InputBuffer.get(), InputBuffer, m_InputBuffer.size());

    return STATUS_SUCCESS;
}


NTSTATUS
CapturedIrp::CapturePostCallFastIoData(_In_ PVOID OutputBuffer)
{
    if ( !m_Valid )
    {
        warn("CapturedIRP was insufficiently initialized");
        return STATUS_ACCESS_DENIED;
    }

    ::RtlCopyMemory(m_OutputBuffer.get(), OutputBuffer, m_OutputBuffer.size());

    return STATUS_SUCCESS;
}

usize const
CapturedIrp::Size() const
{
    return sizeof(Comms::CapturedIrpHeader) + DataSize();
}

usize const
CapturedIrp::DataSize() const
{
    return InputDataSize() + OutputDataSize();
}


usize const
CapturedIrp::InputDataSize() const
{
    return m_InputBuffer.size();
}

u8*
CapturedIrp::InputBuffer() const
{
    return m_InputBuffer.get();
}

u8*
CapturedIrp::OutputBuffer() const
{
    return m_OutputBuffer.get();
}


usize const
CapturedIrp::OutputDataSize() const
{
    return m_OutputBuffer.size();
}

HookedDriver* const
CapturedIrp::AssociatedDriver() const
{
    return m_Driver;
}

Comms::CapturedIrpHeader
CapturedIrp::ExportHeader() const
{
    Comms::CapturedIrpHeader out {};
    out.TimeStamp          = (u64)(m_TimeStamp.QuadPart);
    out.Irql               = m_Irql;
    out.Type               = (u8)m_Type;
    out.MajorFunction      = m_MajorFunction;
    out.MinorFunction      = m_MinorFunction;
    out.IoctlCode          = m_IoctlCode;
    out.Pid                = m_Pid;
    out.Tid                = m_Tid;
    out.Status             = m_Status;
    out.InputBufferLength  = InputDataSize();
    out.OutputBufferLength = OutputDataSize();

    ::memcpy(out.DriverName, m_DriverName.data(), m_DriverName.length());
    ::memcpy(out.DeviceName, m_DeviceName.data(), m_DeviceName.length());
    ::memcpy(out.ProcessName, m_ProcessName.data(), m_ProcessName.length());

    return out;
}

bool
CapturedIrp::IsValid() const
{
    return m_Valid;
}

} // namespace CFB::Driver
