#include "Irp.h"


Irp::Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY InputBuffer, PINTERCEPTED_IRP_BODY OutputBuffer)
	: m_fShouldDelete(FALSE)
{
	PINTERCEPTED_IRP_HEADER hdr = &m_Header;
	::memcpy(hdr, Header, sizeof(INTERCEPTED_IRP_HEADER));

	//m_InputBuffer = new byte[Header->InputBufferLength];
	//::memcpy(m_InputBuffer, InputBuffer, Header->InputBufferLength);
	//
	//m_OutputBuffer = new byte[Header->OutputBufferLength];
	//::memcpy(m_OutputBuffer, OutputBuffer, Header->OutputBufferLength);

	dbg(L"new irp(header=%p, input=%p len=%d, output=%p len=%d)\n", Header, InputBuffer, Header->InputBufferLength, OutputBuffer, Header->OutputBufferLength);

	for (DWORD i = 0; i < Header->InputBufferLength; i++)
		m_InputBuffer.push_back(((PBYTE)InputBuffer)[i]);
	
	for (DWORD i = 0; i < Header->OutputBufferLength; i++)
		m_OutputBuffer.push_back(((PBYTE)OutputBuffer)[i]);

}


Irp::~Irp() 
{
	if (m_fShouldDelete)
	{
		//delete[] m_InputBuffer;
		//delete[] m_OutputBuffer;
	}
}


void Irp::Dispose()
{
	m_fShouldDelete = TRUE;
}


json Irp::IrpHeaderToJson()
{
	json header;
	header["TimeStamp"] = m_Header.TimeStamp.QuadPart;
	header["Irql"] = m_Header.Irql;
	header["Type"] = m_Header.Type;
	header["IoctlCode"] = m_Header.IoctlCode;
	header["Pid"] = m_Header.Pid;
	header["Tid"] = m_Header.Tid;
	header["Status"] = m_Header.Status;
	header["InputBufferLength"] = m_Header.InputBufferLength;
	header["OutputBufferLength"] = m_Header.OutputBufferLength;
	header["DriverName"] = std::wstring(m_Header.DriverName);
	header["DeviceName"] = std::wstring(m_Header.DeviceName);
	header["ProcessName"] = std::wstring(m_Header.ProcessName);
	return header;
}


json Irp::InputBufferToJson()
{
	//std::vector<byte> body;
	//for (DWORD i = 0; i < m_Header.InputBufferLength; i++) body.push_back(m_InputBuffer[i]);
	//json js_body(body);
	json js_body(m_InputBuffer);
	return js_body;
}


json Irp::OutputBufferToJson()
{
	//std::vector<byte> body;
	//for (DWORD i = 0; i < m_Header.OutputBufferLength; i++) body.push_back(m_OutputBuffer[i]);
	//json js_body(body);
	json js_body(m_OutputBuffer);
	return js_body;
}


json Irp::ToJson()
{
	json irp;
	irp["header"] = IrpHeaderToJson();
	irp["body"]["input"] = InputBufferToJson();
	irp["body"]["output"] = OutputBufferToJson();
	return irp;
}

