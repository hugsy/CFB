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
	Irp(PINTERCEPTED_IRP_HEADER Header, PINTERCEPTED_IRP_BODY Body);
	~Irp();
	
	void Dispose();
	PVOID Data();

	json HeaderAsJson();
	std::string ExportHeaderAsJson();

	json BodyAsJson();
	std::string ExportBodyAsJson();

	json AsJson();
	std::string ExportAsJson();


private:
	INTERCEPTED_IRP_HEADER m_Header;
	byte* m_Body = nullptr;
	BOOL m_fShouldDelete = false;
};

