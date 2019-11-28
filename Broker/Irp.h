#pragma once

#include "common.h"
#include <string>
#include <sstream>

#include "json.hpp"
using json = nlohmann::json;

#include "Utils.h"


class Irp
{
public:
	Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY InputBuffer, PINTERCEPTED_IRP_BODY OutputBuffer);
	~Irp();
	
	void Dispose();

	json IrpHeaderToJson();
	json InputBufferToJson();
	json OutputBufferToJson();
	json ToJson();


private:
	INTERCEPTED_IRP_HEADER m_Header = { 0, };
	//byte* m_InputBuffer = nullptr;
	//byte* m_OutputBuffer = nullptr;
	std::vector<byte> m_InputBuffer;
	std::vector<byte> m_OutputBuffer;
	BOOL m_fShouldDelete = false;
};

