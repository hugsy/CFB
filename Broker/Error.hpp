#pragma once

#include <format>
#include <optional>
#include <variant>

#include "Common.hpp"


enum class ErrorCode
{
    GenericError,
    RuntimeError,
    UnexpectedStateError,
    InvalidInput,
    InvalidParameter,
    InitializationError,
    UnexpectedType,
    ArithmeticError,
    OverflowError,
    UnderflowError,
    IllegalValue,
    NotImplementedError,
    PendingIoError,
    NetworkError,
    ConnectionError,
    TerminationError,
    ServiceError,
    FilesystemError,
    InsufficientPrivilegeError,
    SocketInitializationFailed,
    LookupError,
};

///
/// @brief Rust-like type of error handling
///
struct ErrorType
{
    const ErrorCode code;
    const u32 number;
};


template<class T>
using SuccessType = std::optional<T>;

template<class T>
using Result = std::variant<SuccessType<T>, ErrorType>;

struct Err : ErrorType
{
#ifdef _WIN32
    Err(ErrorCode ErrCode = ErrorCode::GenericError) : ErrorType(ErrCode, ::GetLastError())
#else
    Err(ErrorCode ErrCode = ErrorCode::GenericError) : ErrorType(ErrCode, errno)
#endif // _WIN32
    {
    }

    bool
    operator==(const Err& rhs) const
    {
        return this->code == rhs.code;
    }

    bool
    operator==(ErrorCode code) const
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
        return formatter<string, char>::format(std::format("ErrorCode::{}", a), ctx);
    }
};


namespace CFB::Broker::Utils
{
#define CaseToString(x)                                                                                                \
    {                                                                                                                  \
    case (x):                                                                                                          \
        return #x;                                                                                                     \
    }

constexpr const char*
ToString(ErrorType const& x)
{
    switch ( x.code )
    {
        CaseToString(ErrorCode::GenericError);
        CaseToString(ErrorCode::RuntimeError);
        CaseToString(ErrorCode::UnexpectedStateError);
        CaseToString(ErrorCode::InvalidInput);
        CaseToString(ErrorCode::InvalidParameter);
        CaseToString(ErrorCode::UnexpectedType);
        CaseToString(ErrorCode::ArithmeticError);
        CaseToString(ErrorCode::OverflowError);
        CaseToString(ErrorCode::UnderflowError);
        CaseToString(ErrorCode::IllegalValue);
        CaseToString(ErrorCode::NotImplementedError);
        CaseToString(ErrorCode::PendingIoError);
        CaseToString(ErrorCode::ConnectionError);
        CaseToString(ErrorCode::TerminationError);
        CaseToString(ErrorCode::ServiceError);
        CaseToString(ErrorCode::FilesystemError);
        CaseToString(ErrorCode::InsufficientPrivilegeError);
        CaseToString(ErrorCode::SocketInitializationFailed);
        CaseToString(ErrorCode::NetworkError);
    default:
        throw std::invalid_argument("Unimplemented item");
    }
}
#undef CaseAsString
} // namespace CFB::Broker::Utils
