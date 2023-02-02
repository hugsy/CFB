#pragma once

namespace CFB::Broker
{
enum class State : unsigned int
{
    Uninitialized         = 1,
    ServiceManagerReady   = 2,
    IrpManagerReady       = 3,
    ConnectorManagerReady = 4,
    AllManagerReady       = ConnectorManagerReady,
    Running               = 5,
    ConnectorManagerDone  = 6,
    IrpManagerDone        = 7,
    ServiceManagerDone    = 8,
    AllManagerDone        = ServiceManagerDone,
    Finished              = 9
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
        CaseToString(State::IrpManagerReady);
        CaseToString(State::ConnectorManagerReady);
        CaseToString(State::Running);
        CaseToString(State::ConnectorManagerDone);
        CaseToString(State::IrpManagerDone);
        CaseToString(State::ServiceManagerDone);
        CaseToString(State::Finished);
    default:
        throw std::invalid_argument("Unimplemented item");
    }
}
#undef CaseAsString
} // namespace CFB::Broker::Utils
