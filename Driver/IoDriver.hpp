#include "Common.hpp"

namespace CFB::Driver::Ioctl
{
    ///
    /// @brief Hook a driver IOCTL functions
    ///
    /// @param Irp
    /// @param Stack
    /// @return NTSTATUS
    ///
    NTSTATUS HookDriver(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack);

    ///
    /// @brief Unhook a driver IOCTL functions
    ///
    /// @param Irp
    /// @param Stack
    /// @return NTSTATUS
    ///
    NTSTATUS UnhookDriver(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack);
}
