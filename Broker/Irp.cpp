#include "Irp.h"


Irp::Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY Body)
	: m_Header(Header), m_Body(Body), m_fShouldDelete(FALSE)
{

}


Irp::~Irp() 
{

	if(m_fShouldDelete)
	{
		delete m_Header;
		delete m_Body;
	}

}


void Irp::Dispose()
{
	m_fShouldDelete = TRUE;
}


PVOID Irp::Data()
{
	return (PVOID)m_Body;
}


std::string Irp::ToJson()
{
	std::ostringstream oss;

	return oss.str();
}