#pragma once

#include "common.h"
#include <string>
#include <sstream>

class Irp
{
public:
	Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY Body);
	~Irp();
	
	void Dispose();
	PVOID Data();
	std::string ToJson();


private:
	PINTERCEPTED_IRP_HEADER m_Header;
	PINTERCEPTED_IRP_BODY m_Body;
	BOOL m_fShouldDelete;
};

