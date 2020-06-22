#pragma once

#include "common.h"


template <class T>
class GenericHandle
{
public:
	GenericHandle(T h = nullptr) :_h(h) {}

	~GenericHandle()
	{
		Close();
	}

	GenericHandle(const GenericHandle&) = delete;

	GenericHandle& operator=(const GenericHandle&) = delete;

	GenericHandle(GenericHandle&& other) noexcept : _h(other._h)
	{
		other._h = nullptr;
	}

	GenericHandle& operator=(GenericHandle&& other) noexcept
	{
		if (this != &other)
		{
			Close();
			_h = other._h;
			other._h = nullptr;
		}
		return*this;
	}

	operator bool() const
	{
		return _h != nullptr && _h != INVALID_HANDLE_VALUE;
	}

	T Get() const
	{
		return _h;
	}

	virtual void Close()
	{
		if (bool(_h))
		{
			::CloseHandle(_h);
			_h = nullptr;
		}
	}

protected:
	T _h;
};


/**
 * Default handle class
 */
class SafeHandle : public GenericHandle<HANDLE>
{
public:
	using GenericHandle<HANDLE>::GenericHandle;
};

