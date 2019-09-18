#include "CfbException.h"


BaseException::BaseException(const char* filename, unsigned int line, const char* funct, const char* msg)
	: m_filename(filename), m_msg(msg),	m_function_name(funct),	m_line(line)
{
	std::ostringstream oss;
	oss << "[" << __func__ << " in " << m_filename << ":" << m_line << " - " << m_function_name <<"()]" << std::endl;
	oss << "Reason: " << m_msg;
	oss << "GetLastError: 0x" << std::hex << ::GetLastError();
	m_report = oss.str();
}



const char* BaseException::what() const throw()
{
	return m_report.c_str();
}



