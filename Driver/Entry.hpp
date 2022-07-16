#include "Common.hpp"
#include "Log.hpp"

#include "DriverUtils.hpp"

using namespace CFB::Driver::Utils;

#define DEVICE_NAME  CFB_DEVICE_NAME
#define DEVICE_PATH  CFB_DEVICE_PATH
#define DOS_DEVICE_PATH  CFB_DOS_DEVICE_PATH


struct GlobalContext
{
    ///
    /// @brief A pointer to the device object
    ///
    PDEVICE_OBJECT DeviceObject = nullptr;

    ///
    /// @brief A pointer to the EPROCESS of the broker. Not more than one handle to the
    /// device is allowed.
    ///
    PEPROCESS Owner = nullptr;

    ///
    /// @brief
    ///
    KQueuedSpinLock OwnerSpinLock;

    ///
    /// @brief
    ///
    ///
    ULONG SessionId = -1;

    ///
    /// @brief A pointer to the head of hooked drivers
    ///
    LIST_ENTRY HookedDriverHead = { 0 };

    ///
    /// @brief
    ///
    KQueuedSpinLock HookedDriverSpinLock;

    void
    Setup();

    void
    Cleanup();
};

extern struct GlobalContext Globals;
