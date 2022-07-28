#include "CapturedIrpManager.hpp"
namespace CFB::Driver
{


CapturedIrpManager::CapturedIrpManager() : m_Event(nullptr), m_Count(0), m_Entries(), m_Mutex()
{
}

CapturedIrpManager::~CapturedIrpManager()
{
    if ( m_Event )
    {
        //
        // Free the Event object
        //
        ::KeResetEvent(m_Event);
        ObDereferenceObject(m_Event);
    }
}


Utils::LinkedList<CapturedIrp>&
CapturedIrpManager::Items()
{
    return m_Entries;
}

NTSTATUS
CapturedIrpManager::SetEvent(const HANDLE hEvent)
{
    NTSTATUS Status  = STATUS_UNSUCCESSFUL;
    PKEVENT NewEvent = nullptr;

    auto lock = Utils::ScopedLock(m_Mutex);

    //
    // Get a reference to the Event object
    //
    Status =
        ::ObReferenceObjectByHandle(hEvent, EVENT_ALL_ACCESS, *ExEventObjectType, UserMode, (PVOID*)&NewEvent, nullptr);
    if ( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    //
    // If an event object was already assigned, replace it
    //
    if ( m_Event != nullptr )
    {
        ObDereferenceObject(m_Event);
        m_Event = nullptr;
    }

    m_Event = NewEvent;

    return Status;
}


bool
CapturedIrpManager::Push(CapturedIrp* Item)
{
    Utils::ScopedLock lock(m_Mutex);

    //
    // Push the item to the back of the queue
    //
    m_Entries.PushBack(Item);
    m_Count++;

    //
    // Set the event to notify the broker some data is ready
    //
    if ( m_Event )
    {
        ::KeSetEvent(m_Event, 2, false);
    }

    return true;
}


CapturedIrp*
CapturedIrpManager::Pop()
{
    Utils::ScopedLock lock(m_Mutex);

    //
    // Stored data are treated as a FIFO queue
    //
    CapturedIrp* Item = m_Entries.PopFront();
    if ( Item == nullptr )
    {
        return nullptr;
    }

    m_Count--;

    //
    // Unset the event is no data is ready
    //
    if ( m_Count == 0 && m_Event )
    {
        ::KeClearEvent(m_Event);
    }

    return Item;
}


void
CapturedIrpManager::Clear()
{
    do
    {
        auto Item = Pop();
        if ( !Item )
        {
            break;
        }
    } while ( true );
}


} // namespace CFB::Driver
