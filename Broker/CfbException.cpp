#include "CfbException.h"


GenericException::GenericException(const char* filename, unsigned int line, const char* funct, const char* msg) 
	: m_filename(filename), m_msg(msg),	m_function_name(funct),	m_line(line)
{
	std::ostringstream oss;
	oss << "[EXCEPTION REPORT]:" << std::endl;
	oss << "- Raised in " << m_filename << ":" << m_line << std::endl;
	oss << "- More precisely in " << m_function_name << std::endl;
	oss << "- Further infos: " << m_msg;
	oss << "- GetLastError: " << ::GetLastError();
	m_report = oss.str();
}


GenericException::~GenericException() 
{ 
}


const char* GenericException::what() const throw()
{
	return m_report.c_str();
}
