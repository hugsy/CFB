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
	Irp(_In_ PINTERCEPTED_IRP_HEADER Header, _In_ PINTERCEPTED_IRP_BODY InputBuffer, _In_ PINTERCEPTED_IRP_BODY OutputBuffer);
	Irp(const Irp& Original);
	~Irp();
	
	json IrpHeaderToJson();
	json InputBufferToJson();
	json OutputBufferToJson();
	json ToJson();


private:
	INTERCEPTED_IRP_HEADER m_Header = { 0, };
	std::vector<byte> m_InputBuffer;
	std::vector<byte> m_OutputBuffer;
};

