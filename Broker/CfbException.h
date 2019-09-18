#pragma once

#include <Windows.h>

#include <exception>
#include <string>
#include <sstream>


#define RAISE_EXCEPTION(msg) throw GenericException(__FILE__,  __LINE__, __FUNCTION__, msg)


class GenericException :
	public std::exception
{

public:
	GenericException(const char* filename, unsigned int line, const char* funct, const char* msg);
	~GenericException() throw();
	const char* what() const throw();


private:
	std::string m_filename;
	std::string	m_msg;
	std::string m_function_name;
	std::string m_report;
	unsigned int m_line;

};