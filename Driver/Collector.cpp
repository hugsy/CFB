#include "Collector.hpp"

namespace CFB::Driver
{


template<typename T>
bool
DataCollector<T>::Push(T*)
{
    return false;
}


template<typename T>
T*
DataCollector<T>::Pop()
{
    return nullptr;
}
} // namespace CFB::Driver
