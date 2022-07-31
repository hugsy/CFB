#include "ConnectorManager.hpp"


namespace CFB::Broker
{

ConnectorManager::ConnectorManager() : m_StateChangedEvent(INVALID_HANDLE_VALUE)
{
}

ConnectorManager::~ConnectorManager()
{
}


} // namespace CFB::Broker
