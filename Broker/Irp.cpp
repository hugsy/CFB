#include "Irp.h"


Irp::Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY Body)
	: m_fShouldDelete(FALSE)
{
	PINTERCEPTED_IRP_HEADER hdr = &m_Header;
	::memcpy(hdr, Header, sizeof(INTERCEPTED_IRP_HEADER));

	m_InputBuffer = new byte[Header->InputBufferLength];
	::memcpy(m_InputBuffer, Body, Header->InputBufferLength);
}


Irp::~Irp() 
{
	if(m_fShouldDelete)
		delete[] m_InputBuffer;
}


void Irp::Dispose()
{
	m_fShouldDelete = TRUE;
}


PVOID Irp::Data()
{
	return (PVOID)m_InputBuffer;
}


json Irp::HeaderAsJson()
{
	json header;
	header["TimeStamp"] = m_Header.TimeStamp.QuadPart;
	header["Irql"] = m_Header.Irql;
	header["Type"] = m_Header.Type;
	header["IoctlCode"] = m_Header.IoctlCode;
	header["Pid"] = m_Header.Pid;
	header["Tid"] = m_Header.Tid;
	header["InputBufferLength"] = m_Header.InputBufferLength;
	header["OutputBufferLength"] = m_Header.OutputBufferLength;
	header["DriverName"] = std::wstring(m_Header.DriverName);
	header["DeviceName"] = std::wstring(m_Header.DeviceName);
	return header;
}


std::string Irp::ExportHeaderAsJson()
{
	return HeaderAsJson().dump();
}


json Irp::BodyAsJson()
{
	std::vector<byte> body;
	for (DWORD i = 0; i < m_Header.InputBufferLength; i++) body.push_back(m_InputBuffer[i]);
	json js_body(body);
	return js_body;
}


std::string Irp::ExportBodyAsJson()
{
	return BodyAsJson().dump();
}


json Irp::AsJson()
{
	json irp;
	irp["header"] = HeaderAsJson();
	irp["body"] = BodyAsJson();
	return irp;
}


std::string Irp::ExportAsJson()
{
	return BodyAsJson().dump();
}