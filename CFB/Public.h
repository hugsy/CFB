/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_CFB,
    0x42754e41,0x9f89,0x4dce,0x95,0x8e,0x40,0xfd,0x9b,0x9f,0xee,0x36);
// {42754e41-9f89-4dce-958e-40fd9b9fee36}
