#include "IoDriver.hpp"


namespace CFB::Driver::Ioctl
{

NTSTATUS HookDriver(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack)
{
    NTSTATUS Status = STATUS_SUCCESS;
    return Status;
}

}
