#pragma once

#include <Windows.h>

#include <exception>
#include <string>
#include <sstream>


#define RAISE_EXCEPTION(excpt, msg) throw excpt(__FILE__,  __LINE__, __FUNCTION__, msg)
#define RAISE_GENERIC_EXCEPTION(msg) throw BaseException(__FILE__,  __LINE__, __FUNCTION__, msg)


class BaseException :
	public std::exception
{

public:
	BaseException(const char* filename, unsigned int line, const char* funct, const char* msg);
	const char* what() const throw();


protected:
	std::string m_exception;
	std::string m_filename;
	std::string	m_msg;
	std::string m_function_name;
	std::string m_report;
	unsigned int m_line;
};


class BrokenPipeException :
	public BaseException
{
public:
	BrokenPipeException(const char* filename, unsigned int line, const char* funct, const char* msg)
		: BaseException(filename, line, funct, msg)
	{}
};