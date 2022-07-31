#pragma once

// clang-format off
#include "pch.hpp"

#include "Native.hpp"
#include "Common.hpp"
#include "CompileInfo.hpp"
// clang-format on

///
/// @brief Rust-like type of error handling
///
struct ErrorType
{
    enum class Code
    {
        GenericError,
        RuntimeError,
        InvalidInput,
        InvalidParameter,
        UnexpectedType,
        ArithmeticError,
        OverflowError,
        UnderflowError,
        IllegalValue,
        NotImplementedError,
        PendingIoError,
        ConnectionError,
        TerminationError,
        ServiceError,
        FilesystemError,
        InsufficientPrivilegeError,
    };

    Code code;
    u32 number;
};

template<class T>
using SuccessType = std::optional<T>;

template<class T>
using Result = std::variant<SuccessType<T>, ErrorType>;

struct Err : ErrorType
{
    Err(ErrorType::Code ErrCode = ErrorType::Code::GenericError) : ErrorType(ErrCode, ::GetLastError())
    {
    }

    bool
    operator==(const Err& rhs) const
    {
        return this->code == rhs.code;
    }

    bool
    operator==(ErrorType::Code code) const
    {
        return this->code == code;
    }
};

template<class T>
struct Ok : SuccessType<T>
{
    Ok(T value) : SuccessType<T>(value)
    {
    }
};

template<class T>
constexpr bool
Success(Result<T> const& f)
{
    if ( const SuccessType<T>* c = std::get_if<SuccessType<T>>(&f); c != nullptr )
    {
        return true;
    }
    return false;
}

template<class T>
constexpr bool
Failed(Result<T> const& f)
{
    if ( Success(f) )
    {
        return false;
    }

    if ( const ErrorType* c = std::get_if<ErrorType>(&f); c != nullptr )
    {
        return true;
    }

    throw std::bad_variant_access();
}

template<class T>
constexpr T const&
Value(Result<T> const& f)
{
    if ( const SuccessType<T>* c = std::get_if<SuccessType<T>>(&f); c != nullptr && c->has_value() )
    {
        return c->value();
    }
    throw std::bad_variant_access();
}

template<class T>
constexpr ErrorType const&
Error(Result<T> const& f)
{
    if ( const ErrorType* c = std::get_if<ErrorType>(&f); c != nullptr )
    {
        return *c;
    }
    throw std::bad_variant_access();
}


template<>
struct std::formatter<ErrorType, char> : std::formatter<std::string, char>
{
    auto
    format(ErrorType const a, format_context& ctx)
    {
        return formatter<string, char>::format(std::format("ERROR_{}", a), ctx);
    }
};
