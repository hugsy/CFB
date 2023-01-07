#pragma once

// clang-format off
#include "Common.hpp"
#include "Comms.hpp"

#include "DriverUtils.hpp"
#include "HookedDriver.hpp"
// clang-format on

namespace Comms = CFB::Comms;
namespace Utils = CFB::Driver::Utils;

namespace CFB::Driver
{

///
///@brief `CapturedIrp` contains all the information stored from the IRP hijacking
///
class CapturedIrp
{
public:
    enum class IrpType : u8
    {
        Irp          = 0x00,
        FastIo_Ioctl = 0x80,
        FastIo_Read  = 0x81,
        FastIo_Write = 0x82,
    };

    enum class Flags : u32
    {
        UseOutputBuffer  = (1 << 0),
        UseInputBuffer   = (1 << 1),
        InitQueueMessage = (1 << 2),
    };


    ///
    ///@brief Construct a new Captured Irp object
    ///
    ///@param Type
    ///@param DeviceObject
    ///
    CapturedIrp(const IrpType Type, const PDEVICE_OBJECT DeviceObject);

    ///
    ///@brief Destroy the Captured Irp object
    ///
    ///
    ~CapturedIrp();

    ///
    ///@brief CapturedIrp memory allocator
    ///
    ///@param sz
    ///@return void*
    ///
    static void*
    operator new(usize sz)
    {
        void* Memory = ::ExAllocatePoolWithTag(NonPagedPoolNx, sz, CFB_DEVICE_TAG);
        if ( Memory )
        {
            dbg("Allocating CapturedIrp at %p", Memory);
            ::RtlSecureZeroMemory(Memory, sz);
        }
        else
        {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }
        return Memory;
    }

    ///
    ///@brief CapturedIrp memory destructor
    ///
    ///@param Memory
    ///
    static void
    operator delete(void* Memory)
    {
        dbg("Deallocating CapturedIrp at %p", Memory);
        return ::ExFreePoolWithTag(Memory, CFB_DEVICE_TAG);
    }

    ///
    ///@brief Capture the meta-data and content of the IRP *before* passing it down to the legit driver
    ///
    ///@param Irp
    ///@return NTSTATUS
    ///
    NTSTATUS
    CapturePreCallData(_In_ PIRP Irp);

    ///
    ///@brief Capture the meta-data and content of the IRP *after* passing it down to the legit driver
    ///
    ///@param Irp
    ///@param ReturnedIoctlStatus
    ///@return NTSTATUS
    ///
    NTSTATUS
    CapturePostCallData(_In_ PIRP Irp, _In_ NTSTATUS ReturnedIoctlStatus);

    ///
    ///@brief Capture the meta-data and content of the Fast IRP *before* passing it down to the legit driver
    ///
    ///@param InputBuffer
    ///@param IoControlCode
    ///@return NTSTATUS
    ///
    NTSTATUS
    CapturePreCallFastIoData(_In_opt_ PVOID InputBuffer, _In_ ULONG IoControlCode);

    ///
    ///@brief Capture the meta-data and content of the Fast IRP *after* passing it down to the legit driver
    ///
    ///@param OutputBuffer
    ///@return NTSTATUS
    ///
    NTSTATUS
    CapturePostCallFastIoData(_Out_opt_ PVOID OutputBuffer);

    ///
    ///@brief Total size of of captured data (i.e. sizeof(in) + sizeof(out))
    ///
    ///@return usize const
    ///
    usize const
    DataSize();

    ///
    ///@brief Size of of captured input data
    ///
    ///@return usize const
    ///
    usize const
    InputDataSize() const;

    ///
    ///@brief Size of of captured output data
    ///
    ///@return usize const
    ///
    usize const
    OutputDataSize() const;

    ///
    ///@brief Return a raw pointer to the hooked driver
    ///
    ///@return HookedDriver* const
    ///
    HookedDriver* const
    AssociatedDriver() const;

    ///
    ///@brief Entry to the next catpured IRP
    ///
    ///
    LIST_ENTRY Next;

    ///
    ///@brief Generate an exportable CapturedIrp header object
    ///
    ///@return Comms::CapturedIrpHeader
    ///
    Comms::CapturedIrpHeader
    ExportHeader() const;

    ///
    ///@brief Raw pointer to the captured input buffer
    ///
    ///@return u8*
    ///
    u8*
    InputBuffer() const;

    ///
    ///@brief Raw pointer to the captured output buffer
    ///
    ///@return u8*
    ///
    u8*
    OutputBuffer() const;

    ///
    ///@brief Indicates whether the CapturedIrp was successfully initialized
    ///
    ///@return true
    ///@return false
    ///
    bool
    IsValid() const;

private:
    bool m_Valid;
    LARGE_INTEGER m_TimeStamp;
    u8 m_Irql;
    IrpType m_Type;
    u8 m_MajorFunction;
    u8 m_MinorFunction;
    u32 m_IoctlCode;
    u32 m_Pid;
    u32 m_Tid;
    NTSTATUS m_Status;
    Utils::KAlloc<u8*> m_InputBuffer;
    Utils::KAlloc<u8*> m_OutputBuffer;
    PDEVICE_OBJECT m_DeviceObject;
    Utils::KUnicodeString m_DriverName;
    Utils::KUnicodeString m_DeviceName;
    Utils::KUnicodeString m_ProcessName;
    HookedDriver* m_Driver;
};

} // namespace CFB::Driver
