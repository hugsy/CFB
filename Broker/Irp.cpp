#include "Irp.h"


Irp::Irp(_In_ PINTERCEPTED_IRP_HEADER Header, _In_ PINTERCEPTED_IRP_BODY InputBuffer, _In_ PINTERCEPTED_IRP_BODY OutputBuffer)
{
	//dbg(L"new irp(header=%p, input=%p len=%d, output=%p len=%d)=%p\n", Header, InputBuffer, Header->InputBufferLength, OutputBuffer, Header->OutputBufferLength, this);

	PINTERCEPTED_IRP_HEADER hdr = &m_Header;
	::memcpy(hdr, Header, sizeof(INTERCEPTED_IRP_HEADER));

	for (DWORD i = 0; i < Header->InputBufferLength; i++)
		m_InputBuffer.push_back(((PBYTE)InputBuffer)[i]);

	for (DWORD i = 0; i < Header->OutputBufferLength; i++)
		m_OutputBuffer.push_back(((PBYTE)OutputBuffer)[i]);
}

Irp::Irp(const Irp& IrpOriginal) 
	:
	m_InputBuffer(IrpOriginal.m_InputBuffer),
	m_OutputBuffer(IrpOriginal.m_OutputBuffer)
{
	PINTERCEPTED_IRP_HEADER hdr = &m_Header;
	::memcpy(hdr, &IrpOriginal.m_Header, sizeof(INTERCEPTED_IRP_HEADER));
	//dbg(L"copy irp(%p -> %p)\n", &IrpOriginal, this);
}


Irp::~Irp()
{
	//dbg(L"del irp(%p)\n", this);
}


json Irp::IrpHeaderToJson()
{
	json header;
	header["TimeStamp"] = m_Header.TimeStamp.QuadPart;
	header["Irql"] = (uint32_t)m_Header.Irql;
	header["Type"] = (uint32_t)m_Header.Type;
	header["IsFastIo"] = (bool)(m_Header.Type & 0x80000000);
	header["IoctlCode"] = (uint32_t)m_Header.IoctlCode;
	header["ProcessId"] = (uint32_t)m_Header.Pid;
	header["ThreadId"] = (uint32_t)m_Header.Tid;
	header["Status"] = (uint32_t)m_Header.Status;
	header["InputBufferLength"] = m_Header.InputBufferLength;
	header["OutputBufferLength"] = m_Header.OutputBufferLength;
	header["DriverName"]  = Utils::WideStringToString(std::wstring(m_Header.DriverName));
	header["DeviceName"]  = Utils::WideStringToString(std::wstring(m_Header.DeviceName));
	header["ProcessName"] = Utils::WideStringToString(std::wstring(m_Header.ProcessName));
	return header;
}


json Irp::InputBufferToJson()
{
	//dbg(L"InputBufferToJson() -> %d\n", m_InputBuffer.size());
	json j(m_InputBuffer);
	return j;
}


json Irp::OutputBufferToJson()
{
	//dbg(L"OutputBufferToJson() -> %d\n", m_OutputBuffer.size());
	json j(m_OutputBuffer);
	return j;
}


json Irp::ToJson()
{
	json irp;
	irp["header"] = IrpHeaderToJson();
	irp["body"]["InputBuffer"] = InputBufferToJson();
	irp["body"]["OutputBuffer"] = OutputBufferToJson();
	return irp;
}

