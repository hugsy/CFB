#pragma once

namespace CFB::Broker
{
enum class State : int
{
    Uninitialized = 1,
    ServiceManagerReady,
    DriverManagerReady,
    ConnectorManagerReady,
    Running,
    ConnectorManagerDone,
    DriverManagerDone,
    ServiceManagerDone,
    Finished
};
} // namespace CFB::Broker


namespace CFB::Broker::Utils
{
#define CaseToString(x)                                                                                                \
    {                                                                                                                  \
    case (x):                                                                                                          \
        return #x;                                                                                                     \
    }

constexpr const char*
ToString(State x)
{
    switch ( x )
    {
        CaseToString(State::Uninitialized);
        CaseToString(State::ServiceManagerReady);
        CaseToString(State::DriverManagerReady);
        CaseToString(State::ConnectorManagerReady);
        CaseToString(State::Running);
        CaseToString(State::ConnectorManagerDone);
        CaseToString(State::DriverManagerDone);
        CaseToString(State::ServiceManagerDone);
        CaseToString(State::Finished);
    default:
        throw std::invalid_argument("Unimplemented item");
    }
}
#undef CaseAsString
} // namespace CFB::Broker::Utils
