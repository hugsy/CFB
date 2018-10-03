/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/* harness-copyright.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/*

Module Name:
    
    SDV harness


Abstract:
    
    Functions as representation of Windows OS for SDV.  It boots the 
    device driver and puts it into various states via its entry 
    DriverEntry routine and dispatch routines, etc.


Environment:

    Symbolic execution in Static Driver Verifier.

*/
/* harness-copyright.c end */

/* malloc.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

char * malloc(int);


char sdv_alloc_dummy;

char* malloc(
    int i
    )
{
    return &sdv_alloc_dummy;
}/* malloc.c end */

/* slic_data.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/


#if defined(_NTIFS_INCLUDED_) || defined(SDV_Include_NTIFS)
#include <ntifs.h>
#include <ntdddisk.h>
#elif defined(_NTDDK_) || defined(SDV_Include_NTDDK)
#include <ntddk.h>
#include <ntdddisk.h>
#else
#include <wdm.h>
#include <ntdddisk.h>
#endif




#include <slic_types_macros.h>


#include "dispatch_routines.h"
#include "sdv_stubs.h"

#ifdef SDV_RULE_NULLCHECK
BOOLEAN sdv_rule_NullCheck = TRUE;
#else
BOOLEAN sdv_rule_NullCheck = FALSE;
#endif    


BOOLEAN sdv_dpc_io_registered = FALSE;

BOOLEAN sdv_apc_disabled = FALSE;



IRP sdv_ControllerIrp;
PIRP sdv_ControllerPirp = &sdv_ControllerIrp; 

IRP sdv_StartIoIrp;
PIRP sdv_StartIopirp = &sdv_StartIoIrp; 


IRP sdv_PowerIrp;
PIRP sdv_power_irp = &sdv_PowerIrp; 


IRP sdv_harnessIrp;
PIRP sdv_irp = &sdv_harnessIrp; 


IO_STACK_LOCATION sdv_harnessStackLocation;
IO_STACK_LOCATION sdv_harnessStackLocation_next;

IRP sdv_other_harnessIrp;
PIRP sdv_other_irp = &sdv_other_harnessIrp;
IO_STACK_LOCATION sdv_other_harnessStackLocation;
IO_STACK_LOCATION sdv_other_harnessStackLocation_next;

IRP sdv_IoMakeAssociatedIrp_harnessIrp;
PIRP sdv_IoMakeAssociatedIrp_irp = &sdv_IoMakeAssociatedIrp_harnessIrp;
IO_STACK_LOCATION sdv_IoMakeAssociatedIrp_harnessStackLocation;
IO_STACK_LOCATION sdv_IoMakeAssociatedIrp_harnessStackLocation_next;

IRP sdv_IoBuildDeviceIoControlRequest_harnessIrp;
PIRP sdv_IoBuildDeviceIoControlRequest_irp = &sdv_IoBuildDeviceIoControlRequest_harnessIrp;
IO_STACK_LOCATION sdv_IoBuildDeviceIoControlRequest_harnessStackLocation;
IO_STACK_LOCATION sdv_IoBuildDeviceIoControlRequest_harnessStackLocation_next;
IO_STATUS_BLOCK sdv_harness_IoBuildDeviceIoControlRequest_IoStatusBlock;
PIO_STATUS_BLOCK sdv_IoBuildDeviceIoControlRequest_IoStatusBlock=&sdv_harness_IoBuildDeviceIoControlRequest_IoStatusBlock;





IRP sdv_IoBuildSynchronousFsdRequest_harnessIrp;
PIRP sdv_IoBuildSynchronousFsdRequest_irp = &sdv_IoBuildSynchronousFsdRequest_harnessIrp;
IO_STACK_LOCATION sdv_IoBuildSynchronousFsdRequest_harnessStackLocation;
IO_STACK_LOCATION sdv_IoBuildSynchronousFsdRequest_harnessStackLocation_next;
IO_STATUS_BLOCK sdv_harness_IoBuildSynchronousFsdRequest_IoStatusBlock;
PIO_STATUS_BLOCK sdv_IoBuildSynchronousFsdRequest_IoStatusBlock=&sdv_harness_IoBuildSynchronousFsdRequest_IoStatusBlock;


IRP sdv_IoBuildAsynchronousFsdRequest_harnessIrp;
PIRP sdv_IoBuildAsynchronousFsdRequest_irp = &sdv_IoBuildAsynchronousFsdRequest_harnessIrp;
IO_STACK_LOCATION sdv_IoBuildAsynchronousFsdRequest_harnessStackLocation;
IO_STACK_LOCATION sdv_IoBuildAsynchronousFsdRequest_harnessStackLocation_next;
IO_STATUS_BLOCK sdv_harness_IoBuildAsynchronousFsdRequest_IoStatusBlock;
PIO_STATUS_BLOCK sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock=&sdv_harness_IoBuildAsynchronousFsdRequest_IoStatusBlock;



IRP sdv_IoInitializeIrp_harnessIrp;
PIRP sdv_IoInitializeIrp_irp = &sdv_IoInitializeIrp_harnessIrp;
IO_STACK_LOCATION sdv_IoInitializeIrp_harnessStackLocation;
IO_STACK_LOCATION sdv_IoInitializeIrp_harnessStackLocation_next;


int sdv_harnessDeviceExtension_val;
int sdv_harnessDeviceExtension_two_val;


PVOID sdv_harnessDeviceExtension;
PVOID sdv_harnessDeviceExtension_two;




int sdv_io_create_device_called = 0;

int sdv_context_val ;
PVOID sdv_context;
int sdv_start_info_val ;
ULONG_PTR sdv_start_info;
int sdv_end_info_val ;
ULONG_PTR sdv_end_info;


/* SDV OS Model IRQL Stack:

The OS Model contains a Bounded IRQL Stack.

The Bounded IRQL Stack should only be changed in OS Model DDIs and
only using 1) the operation SDV_IRQL_PUSH for pushing onto the stack
and 2) SDV_IRQL_POP and SDV_IRQL_POPTO for popping from the stack.

The stack is used in a number of rules that specify proper stack
behaviour for pairs of DDIs such as for example KeAcquireSpinLock and
KeReleaseSpinLock.

Pushing is used in the OS Model when a DDI sets/increases the IRQL to
a new and weakly higher IRQL.  An example of such a DDI is
KeAcquireSpinLock.

Popping is used in the OS Model when a DDI restores/lowers the IRQL to
a new and weakly lower IRQL.  An example of such a DDI is
KeReleaseSpinLock.

The stack is represented by the global variables sdv_irql_current,
sdv_irql_previous, sdv_irql_previous_2, sdv_irql_previous_3, sdv_irql_previous_4 and sdv_irql_previous_5.
sdv_irql_current is considered the top of the stack, sdv_irql_previous
is the second element of the stack and so on.

The stack is bounded.  Currently to height three.  This means that the
OS Model will not correctly capture the meaning of more than three
pushes to the stack.  Certain rules that check proper stack behaviour
takes this into account by counting current depth of the stack.

*/

/* Global IRQL stack, 6 levels high: */
KIRQL sdv_irql_current = PASSIVE_LEVEL;
KIRQL sdv_irql_previous = PASSIVE_LEVEL;
KIRQL sdv_irql_previous_2 = PASSIVE_LEVEL;
KIRQL sdv_irql_previous_3 = PASSIVE_LEVEL;
KIRQL sdv_irql_previous_4 = PASSIVE_LEVEL;
KIRQL sdv_irql_previous_5 = PASSIVE_LEVEL;

int sdv_maskedEflags = 0;

struct _KDPC sdv_kdpc_val;
struct _KDPC * sdv_kdpc;

PKDPC sdv_pkdpc;

struct _KDPC sdv_kdpc_val3;
struct _KDPC * sdv_kdpc3 = &sdv_kdpc_val3;


int sdv_DpcContext;
PVOID sdv_pDpcContext;

int sdv_IoDpcContext;
PVOID sdv_pIoDpcContext;

int sdv_IoCompletionContext;
PVOID sdv_pIoCompletionContext;


PVOID sdv_pv1;
PVOID sdv_pv2;
PVOID sdv_pv3;

DRIVER_OBJECT sdv_driver_object;

DEVICE_OBJECT sdv_devobj_fdo;
PDEVICE_OBJECT sdv_p_devobj_fdo = &sdv_devobj_fdo;


BOOLEAN sdv_inside_init_entrypoint = FALSE;



DEVICE_OBJECT sdv_devobj_pdo;
PDEVICE_OBJECT sdv_p_devobj_pdo = &sdv_devobj_pdo;


DEVICE_OBJECT sdv_devobj_child_pdo;
PDEVICE_OBJECT sdv_p_devobj_child_pdo = &sdv_devobj_child_pdo;


int sdv_kinterrupt_val;
struct _KINTERRUPT *sdv_kinterrupt = (struct _KINTERRUPT *) &sdv_kinterrupt_val;

int sdv_MapRegisterBase_val;
PVOID sdv_MapRegisterBase = (PVOID) &sdv_MapRegisterBase_val;



BOOLEAN sdv_invoke_on_success = FALSE;


BOOLEAN sdv_invoke_on_error = FALSE;


BOOLEAN sdv_invoke_on_cancel = FALSE;


#if (NTDDI_VERSION >= NTDDI_WIN8)

PO_FX_DEVICE sdv_fx_dev_object;
PPO_FX_DEVICE p_sdv_fx_dev_object = &sdv_fx_dev_object;
#endif


PVOID sdv_PoRuntime_Context;
PVOID sdv_PoRuntime_InBuffer;
PVOID sdv_PoRuntime_OutBuffer;
BOOLEAN sdv_PoRuntime_Active;
BOOLEAN sdv_PoRuntime_Suspended;
BOOLEAN sdv_PoRuntime_PowerRequired;
ULONG sdv_PoRuntime_Component;
ULONG sdv_PoRuntime_State;
PVOID sdv_PoRuntime_DeviceContext;
LPCGUID sdv_PoRuntime_PowerControlCode;
SIZE_T sdv_PoRuntime_InBufferSize;
SIZE_T sdv_PoRuntime_OutBufferSize;
PSIZE_T sdv_PoRuntime_BytesReturned;
/* slic_data.c end */

/* dispatch-with-callbacks-harness.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/*****************************************************************************

    dispatch-with-callbacks-harness.c provides harness that matches IO control flow for exercising a driver.

    The following variations of the harness are available:
        SDV_FLAT_DISPATCH_HARNESS_WITH_LINKED_CALLBACKS

    The harnesses exercises the driver as follows:
        SDV_FLAT_DISPATCH_HARNESS =
            sdv_RunDispatchFunction and all related workitems,completion,powercompletion,kernel dpcs,kernel and non MSI interrupts

        

*****************************************************************************/
     
#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_LINKED_CALLBACKS)

#define SDV_FLAT_HARNESS_MODIFIER_NO_UNLOAD
#define SDV_HARNESS_COMPLETION_ROUTINE
#define SDV_HARNESS_POWER_COMPLETION_ROUTINE
#define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
#define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
#define SDV_RUN_KE_DPC_ROUTINES
#define SDV_RUN_KE_ISR_ROUTINES  


void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_COMPLETION_NO_CANCEL)

#define SDV_HARNESS_COMPLETION_ROUTINE
#define SDV_HARNESS_POWER_COMPLETION_ROUTINE
#define SDV_FLAT_HARNESS_MODIFIER_NO_DRIVER_CANCEL

void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif

#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_NO_CANCEL)

#define SDV_HARNESS_COMPLETION_ROUTINE
#define SDV_HARNESS_POWER_COMPLETION_ROUTINE
#define SDV_FLAT_HARNESS_MODIFIER_NO_DRIVER_CANCEL

void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_SET_QUERY_POWER_IRPS_ONLY)

  #define SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_MN_FUNCTIONS
  #define SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_IRP_MN_SET_POWER
  #define SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_IRP_MN_QUERY_POWER
  #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
  #define SDV_HARNESS_COMPLETION_ROUTINE
   

void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchPower(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_PNP_IRPS_EXCLUDING_QUERY_AND_START)
 
   #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_QUERY
   #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_START_DEVICE
   #define SDV_HARNESS_COMPLETION_ROUTINE
   
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchPnp(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_COMPLETION_QUEUE_STARTIO_DRIVER_CONTROL_CALLBACKS)

     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE


void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif

#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_COMPLETION_QUEUE_STARTIO_DRIVER_CONTROL_CALLBACKS_NO_UNLOAD_OR_REMOVE)

  #define SDV_FLAT_HARNESS_MODIFIER_NO_UNLOAD
  #define SDV_HARNESS_COMPLETION_ROUTINE
  #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_MN_REMOVE_DEVICE
  #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
  #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
  #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
  #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE

void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif

#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_CLEANUP_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchCleanup(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_CLOSE_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchClose(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_CREATE_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchCreate(sdv_p_devobj_fdo, sdv_irp);

}

#endif



#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_DEVICE_CONTROL_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchDeviceControl(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_INTERNAL_DEVICE_CONTROL_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
    
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchInternalDeviceControl(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_PNP_LINKED_CALLBACKS_WITHOUT_REMOVE_DEVICE)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_MN_REMOVE_DEVICE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchPnp(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_POWER_LINKED_CALLBACKS)

     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchPower(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_READ_LINKED_CALLBACKS)
    
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX 
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
    
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchRead(sdv_p_devobj_fdo, sdv_irp);

}

#endif

#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_SHUTDOWN_LINKED_CALLBACKS)
 
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
    
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchShutdown(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_SYSTEM_CONTROL_LINKED_CALLBACKS)

     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE
     
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchSystemControl(sdv_p_devobj_fdo, sdv_irp);

}

#endif

#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_WRITE_LINKED_CALLBACKS)
     
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
     #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
     #define SDV_HARNESS_DRIVERSTARTIO_ROUTINE
     #define SDV_HARNESS_DRIVER_CONTROL_ROUTINE

void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchWrite(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_COMPLETION_QUEUE_CALLBACKS)

#define SDV_HARNESS_COMPLETION_ROUTINE
#define SDV_HARNESS_POWER_COMPLETION_ROUTINE
#define SDV_HARNESS_POWER_COMPLETION_ROUTINE
#define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
#define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
#define SDV_HARNESS_DRIVER_CONTROL_ROUTINE


void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

}

#endif


#if (SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_PNP_IRPS_EXCLUDING_QUERY_AND_START_WITH_DEVICE_RELATIONS_ONLY)

  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_QUERY
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_START_DEVICE
  #define SDV_FLAT_HARNESS_MODIFIER_IRP_MN_QUERY_DEVICE_RELATIONS_ONLY
  #define SDV_HARNESS_COMPLETION_ROUTINE
  
void sdv_main() 
{
    sdv_stub_driver_init();
    sdv_RunDispatchPnp(sdv_p_devobj_fdo, sdv_irp);

}

#endif

/* harness-parts.c begin */
#define SDV_IS_FLAT_SIMPLE_HARNESS() \
    ( \
        SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS \
    )


#define SDV_FLAT_SIMPLE_HARNESS() \
    ( \
       SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL \
    )

#define SDV_FLAT_SIMPLE_HARNESS() \
    ( \
       SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL || \
	   SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_NO_DEVICE_IRPS || \
	   SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_ONLY || \
	   SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_NO_CANCEL || \
	   SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_PNP_POWER_IRPS || \
	   SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_WMI_ONLY \
    )



#define SDV_IS_FLAT_HARNESS() \
    ( \
        SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS || \
        SDV_HARNESS==SDV_FLAT_DISPATCH_STARTIO_HARNESS || \
        SDV_HARNESS==SDV_FLAT_HARNESS || \
        SDV_IS_FLAT_SIMPLE_HARNESS() \
    )

#define SDV_IS_XFLAT_HARNESS_CANCEL() \
    ( \
        SDV_HARNESS==SDV_XFLAT_HARNESS_CANCEL || \
        SDV_HARNESS==SDV_XFLAT_SIMPLE_HARNESS_CANCEL \
    )

#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL
	#define SDV_FLAT_HARNESS_MODIFIER_NO_DRIVER_CANCEL
	#define SDV_HARNESS_COMPLETION_ROUTINE
	#define SDV_NO_DEBUGGER_ATTACHED_OR_ENABLED
#endif

#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_NO_CANCEL
	#define SDV_FLAT_HARNESS_MODIFIER_NO_DRIVER_CANCEL
	#define SDV_NO_DEBUGGER_ATTACHED_OR_ENABLED
#endif


#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_NO_DEVICE_IRPS
  #define SDV_FLAT_HARNESS_NO_DPC
  #define SDV_FLAT_HARNESS_NO_ISR
  #define SDV_FLAT_HARNESS_NO_KE_DPC
  #define SDV_FLAT_HARNESS_NO_KE_WORK_ITEMS 
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_DEVICE_CONTROL
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_INTERNAL_DEVICE_CONTROL
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_READ
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_WRITE
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_POWER
  #define SDV_HARNESS_COMPLETION_ROUTINE
#endif


#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_ONLY
  #define SDV_FLAT_HARNESS_NO_DPC
  #define SDV_FLAT_HARNESS_NO_ISR
  #define SDV_FLAT_HARNESS_NO_KE_DPC
  #define SDV_FLAT_HARNESS_NO_KE_WORK_ITEMS 
  #define SDV_HARNESS_COMPLETION_ROUTINE
#endif

#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_PNP_POWER_IRPS
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CLEANUP
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CREATE
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_DEVICE_CONTROL
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FLUSH_BUFFERS
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FLUSH_BUFFER
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_INTERNAL_DEVICE_CONTROL
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_LOCK_CONTROL
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_QUERY_INFORMATION
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_READ
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SET_INFORMATION
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SHUTDOWN
  #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_WRITE
#endif


#if SDV_HARNESS==SDV_FLAT_SIMPLE_HARNESS_WITH_WMI_ONLY
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CLEANUP
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CREATE
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_DEVICE_CONTROL
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FLUSH_BUFFERS
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FLUSH_BUFFER
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_INTERNAL_DEVICE_CONTROL
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_LOCK_CONTROL
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_QUERY_INFORMATION
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_READ
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SET_INFORMATION
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_WRITE
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_POWER
	#define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SHUTDOWN
    #define SDV_HARNESS_COMPLETION_ROUTINE
#endif

#define SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION()\
sdv_harnessStackLocation_next.CompletionRoutine=NULL;\
sdv_other_harnessStackLocation_next.CompletionRoutine=NULL;\
sdv_harnessStackLocation.CompletionRoutine=NULL;\


#define SDV_MACRO_COPYCURRENTIRPSTACKLOCATIONTONEXT(arg1)\
    if (arg1 == &sdv_harnessIrp) {\
        sdv_harnessStackLocation_next.MinorFunction =\
            sdv_harnessStackLocation.MinorFunction;\
        sdv_harnessStackLocation_next.MajorFunction =\
            sdv_harnessStackLocation.MajorFunction;\
    }\
    if (arg1 == &sdv_other_harnessIrp) {\
        sdv_other_harnessStackLocation_next.MinorFunction =\
            sdv_other_harnessStackLocation.MinorFunction;\
        sdv_other_harnessStackLocation_next.MajorFunction =\
            sdv_other_harnessStackLocation.MajorFunction;\
    }\

#define SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(arg1)\
(arg1->Tail.Overlay.CurrentStackLocation)\

#define SDV_MACRO_STUB_CANCEL_BEGIN(arg1)\
IoAcquireCancelSpinLock(&(arg1->CancelIrql))\

#define SDV_MACRO_STUB_CANCEL_END(arg1)\
arg1->CancelRoutine=NULL\


VOID 
sdv_SetIrpMinorFunctionNonBusDriver(
    PIRP pirp
    );
    
PIO_STACK_LOCATION
sdv_SetPowerRequestIrpMinorFunction(
    PIRP pirp
    );


VOID
sdv_SetIrpMinorFunctionBusDriver(
    PIRP pirp
    );


VOID
sdv_SetMajorFunction(
    PIRP pirp,
    UCHAR fun
    );

VOID
sdv_SetPowerIrpMinorFunction(
    PIRP pirp
    );
    
VOID
sdv_SetPowerIrpMinorFunctionSetPower(
    PIRP pirp
    );

VOID
sdv_SetStatus(
    PIRP pirp
    );


NTSTATUS
sdv_DoNothing(
    );

NTSTATUS         
sdv_RunDispatchFunction(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchWrite(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchRead(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchInternalDeviceControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchDeviceControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchCreate(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS         
sdv_RunDispatchClose(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunDispatchPower(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunDispatchPnp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );


NTSTATUS         
sdv_RunDispatchSystemControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );




NTSTATUS         
sdv_RunDispatchShutdown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );



NTSTATUS         
sdv_RunDispatchCleanup(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );



VOID         
sdv_RunCancelFunction(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );



NTSTATUS 
sdv_RunSurpriseRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );


NTSTATUS 
sdv_RunRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );


NTSTATUS 
sdv_RunQueryRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );
    
NTSTATUS         
sdv_RunQueryDeviceRelations(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunQueryCapRequirements(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );
    
NTSTATUS 
sdv_RunResRequirements(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunQueryDeviceState(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunQueryPowerUp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );
    
NTSTATUS 
sdv_RunQueryPowerUpDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );


NTSTATUS 
sdv_RunQueryPowerDown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunSetPowerUpDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunSetPowerUp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

NTSTATUS 
sdv_RunSetPowerDown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

PIRP             
sdv_MakeAbstractIrp(PIRP pirp
    );


PIRP             
sdv_MakeStartDeviceIrp(PIRP pirp
    );


PIRP             
sdv_MakeRemoveDeviceIrp(PIRP pirp
    );

LONG
SdvMakeChoice();

LONG
SdvKeepChoice();

/*POWER_STATE
sdv_Make_POWER_STATE();*/

VOID
sdv_RunStartIo(
    PDEVICE_OBJECT po, 
    PIRP pirp
    );

void
sdv_RunExQueueWorkItems(
    PWORKER_THREAD_ROUTINE WorkerRoutine,
    PVOID Context
    );

void
sdv_RunIoQueueWorkItems(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );

void
sdv_RunIoQueueWorkItemsEx(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE_EX WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );

NTSTATUS
sdv_RunIoCompletionRoutines(
    __in PDEVICE_OBJECT DeviceObject, 
    __in PIRP Irp, 
    __in_opt PVOID Context,
    BOOLEAN* Completion_Routine_Called
    );


BOOLEAN
sdv_RunPowerCompletionRoutines(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus,
    PREQUEST_POWER_COMPLETE CompletionFunction
    );



VOID
sdv_RunISRRoutines(
    struct _KINTERRUPT *ki, 
    PVOID pv1
    );

VOID
sdv_RunKeDpcRoutines(
    struct _KDPC *kdpc, 
    PVOID pDpcContext, 
    PVOID pv2, 
    PVOID pv3
    );
    
VOID
sdv_RunIoDpcRoutines(
    IN PKDPC  Dpc,    
    IN struct _DEVICE_OBJECT  *DeviceObject,    
    IN struct _IRP  *Irp,    
    IN PVOID  Context
    );

BOOLEAN
sdv_RunKSynchronizeRoutines(
    PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID  Context
    );


VOID
sdv_RunUnload(
    PDRIVER_OBJECT pdrivo
    );

NTSTATUS sdv_RunAddDevice(
    PDRIVER_OBJECT p1,
    PDEVICE_OBJECT p2
    );

/*NTSTATUS sdv_RunDriverentry(
    _DRIVER_OBJECT  *DriverObject,
    PUNICODE_STRING  RegistryPath
    );*/

BOOLEAN 
sdv_CheckDispatchRoutines(
    );

BOOLEAN 
sdv_CheckStartIoRoutines(
    );

BOOLEAN 
sdv_CheckDpcRoutines(
    );

BOOLEAN 
sdv_CheckIsrRoutines(
    );

BOOLEAN 
sdv_CheckCancelRoutines(
    );

BOOLEAN 
sdv_CheckCancelRoutines1(
    );


BOOLEAN 
sdv_CheckIoDpcRoutines(
    );

BOOLEAN 
sdv_IoCompletionRoutines(
    );

BOOLEAN 
sdv_CheckWorkerRoutines(
    );

BOOLEAN 
sdv_CheckAddDevice(
    );

BOOLEAN 
sdv_CheckIrpMjPnp(
    );

BOOLEAN 
sdv_CheckIrpMjPower(
    );


BOOLEAN 
sdv_CheckDriverUnload(
    );



VOID SdvExit();

VOID SdvAssume(int e);

VOID SdvAssumeSoft(int e);





int sdv_start_irp_already_issued = 0;
int sdv_remove_irp_already_issued = 0;
int sdv_Io_Removelock_release_wait_returned = 0;


int sdv_compFset = 0;

BOOLEAN
sdv_isr_dummy (
    IN struct _KINTERRUPT * Interrupt,
    IN PVOID ServiceContext
    )
{
    return FALSE;
}

PKSERVICE_ROUTINE  sdv_isr_routine = sdv_isr_dummy;




VOID
sdv_dpc_dummy (
    IN struct _KDPC * Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
}

PKDEFERRED_ROUTINE  sdv_ke_dpc=sdv_dpc_dummy;

BOOLEAN sdv_dpc_ke_registered=FALSE;



VOID
sdv_io_dpc_dummy (
    IN PKDPC  Dpc,    
    IN struct _DEVICE_OBJECT  *DeviceObject,    
    IN struct _IRP  *Irp,    
    IN PVOID  Context
    )
{
}

PIO_DPC_ROUTINE  sdv_io_dpc=sdv_io_dpc_dummy;

/* Operations on the global IRQL stack: */

/*
   SDV_IRQL_PUSH(new_irql):
     Change to new_irql IRQL, pushing old levels on the stack
*/

#define SDV_IRQL_PUSH(new_irql) \
    sdv_irql_previous_5 = sdv_irql_previous_4; \
    sdv_irql_previous_4 = sdv_irql_previous_3; \
    sdv_irql_previous_3 = sdv_irql_previous_2; \
    sdv_irql_previous_2 = sdv_irql_previous; \
    sdv_irql_previous = sdv_irql_current; \
    sdv_irql_current = new_irql

/*
   SDV_IRQL_POP():
     Change to previous IRQL, popping it from the stack
*/

#define SDV_IRQL_POP() \
    sdv_irql_current = sdv_irql_previous; \
    sdv_irql_previous = sdv_irql_previous_2; \
    sdv_irql_previous_2 = sdv_irql_previous_3; \
    sdv_irql_previous_3 = sdv_irql_previous_4; \
    sdv_irql_previous_4 = sdv_irql_previous_5

/*
   SDV_IRQL_POPTO(new_irql):
     Change to new_irql IRQL,
     popping (and ignoring) an IRQL from the stack
*/

#define SDV_IRQL_POPTO(new_irql) \
    sdv_irql_current = new_irql; \
    sdv_irql_previous = sdv_irql_previous_2; \
    sdv_irql_previous_2 = sdv_irql_previous_3; \
    sdv_irql_previous_3 = sdv_irql_previous_4; \
    sdv_irql_previous_4 = sdv_irql_previous_5
    

VOID sdv_Trap ( VOID ) { ; }


VOID
sdv_NullDereferenceTrap (
    PVOID p
    )
{
  if ( p == 0 ) { sdv_Trap();  }
}



#define SDV_MAIN_INIT()\
		SdvAssume(sdv_harnessDeviceExtension != 0);\
    SdvAssume(sdv_harnessDeviceExtension_two != 0);\
    sdv_devobj_pdo.DeviceExtension = sdv_harnessDeviceExtension;\
    sdv_devobj_fdo.DeviceExtension = sdv_harnessDeviceExtension_two;\
    sdv_irp->Tail.Overlay.CurrentStackLocation = &sdv_harnessStackLocation;\
    sdv_other_irp->Tail.Overlay.CurrentStackLocation = &sdv_other_harnessStackLocation
    


VOID 
sdv_SetPowerIrpMinorFunction(
    PIRP pirp
    )  
/*

Routine Description:

    Sets the MN IRP fields to the possible values.  

Arguments:

    pirp - The IRP to set.

Notes:
    Note how we're using non-determinism here with "x" and "y".  "x", for
    example, could take on any value.

*/
{
    PIO_STACK_LOCATION r = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG x = SdvMakeChoice();
    LONG y = SdvMakeChoice();

    switch (x) {
        case 0:
        r->MinorFunction = IRP_MN_SET_POWER;
        switch (y) {
            case 0:
            r->Parameters.Power.Type = SystemPowerState;
            break;

            case 1:
            default:
            r->Parameters.Power.Type = DevicePowerState;
            break;
        }
              break;

        case 1:
        r->MinorFunction = IRP_MN_QUERY_POWER;
        switch (y) {

            case 0:
            r->Parameters.Power.Type = SystemPowerState;
            break;

            case 1:
            default:
            r->Parameters.Power.Type = DevicePowerState;
            break;
        }
        break;


        case 2:
        r->MinorFunction = IRP_MN_POWER_SEQUENCE;
        break;      


        case 3:
        default:
        r->MinorFunction = IRP_MN_WAIT_WAKE;
        break;
        }
}



VOID 
sdv_SetPowerIrpMinorFunctionSetPower(
    PIRP pirp
    )  
/*

Routine Description:

    Sets the MN IRP fields to the possible values.  

Arguments:

    pirp - The IRP to set.

Notes:
    Note how we're using non-determinism here with "x" and "y".  "x", for
    example, could take on any value.

*/
{
    PIO_STACK_LOCATION r = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG nd_state = SdvMakeChoice();
    r->MinorFunction = IRP_MN_SET_POWER;
    r->Parameters.Power.Type = SystemPowerState;
    switch (nd_state)
    { 
      case 0:
      r->Parameters.Power.State.SystemState=PowerSystemWorking;
      break;
      case 1:
      r->Parameters.Power.State.SystemState=PowerSystemSleeping1;
      break;
      case 2:
      r->Parameters.Power.State.SystemState=PowerSystemSleeping2;
      break;
      case 3:
      r->Parameters.Power.State.SystemState=PowerSystemSleeping3;
      break;
      case 4:
      r->Parameters.Power.State.SystemState=PowerSystemHibernate;
      break;
      case 5:
      r->Parameters.Power.State.SystemState=PowerSystemShutdown;
      break;
      default:
      r->Parameters.Power.State.SystemState=PowerSystemMaximum;
      break;
    }
}





PIRP 
sdv_MakeRemoveDeviceIrp(PIRP pirp
    )
{
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);

    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_REMOVE_DEVICE;
    pirp->CancelRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);

    return pirp;
}




PIRP 
sdv_MakeStartDeviceIrp(PIRP pirp
    )
{
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);

    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_START_DEVICE;
    pirp->CancelRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);

    return pirp;
}



VOID
sdv_SetMajorFunction(
    PIRP pirp,
    UCHAR fun
    )
{

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = fun;   

}




VOID 
sdv_SetStatus(
    PIRP pirp
    )
{
    LONG choice = SdvMakeChoice();

    switch(choice) {
        case 0:
            pirp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            break;
        case 1:
        default:
            pirp->IoStatus.Status = STATUS_SUCCESS;
            break;
    }
}


        

NTSTATUS 
sdv_DoNothing(
    )
{
    return STATUS_UNSUCCESSFUL;
}





LONG 
SdvMakeChoice(
    )
/*

Routine Description:

    Non-deterministically chooses a value and returns it:
    to full cl we return 0, but then assignment x = SdvKeepChoice
    is eliminated - so that x remains uninitialized

Arguments:

Notes:
    Note how we're using non-determinism here:  "x" can be any value.

    If you wanted to take this harness and actually execute it, you need
    to implement the non-determinism.  Changing this function would be a
    start, but you would also need to change the places where IRPs and other
    types are non-deterministically choosen.

*/
{
    return 0;
}

LONG 
SdvKeepChoice(
    )
/*

Routine Description:

    Non-deterministically chooses a value and returns it:
    to full cl we return 0, but then assignment x = SdvMakeChoice
    is eliminated - so that x remains uninitialized

Arguments:

Notes:
    Note how we're using non-determinism here:  "x" can be any value.

    If you wanted to take this harness and actually execute it, you need
    to implement the non-determinism.  Changing this function would be a
    start, but you would also need to change the places where IRPs and other
    types are non-deterministically choosen.

*/
{
    return 0;
}

/*POWER_STATE
sdv_Make_POWER_STATE(
    )
{
  POWER_STATE x;
  int y = SdvMakeChoice();
  switch(y) {
    case 1 : x.SystemState = SdvMakeChoice(); break;
    default: x.DeviceState = SdvMakeChoice(); break;
  }
  return x;
}*/


NTSTATUS 
sdv_RunDispatchFunction(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_CREATE" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    
    int x = SdvMakeChoice();
    UCHAR minor_function = (UCHAR) SdvKeepChoice();

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) pirp->IoStatus.Information;
 
    sdv_SetStatus(pirp);

    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    


    ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    
    sdv_dpc_io_registered = FALSE;
    
    sdv_stub_dispatch_begin();

    switch (x) { 

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CLEANUP
        case IRP_MJ_CLEANUP:
        ps->MajorFunction = IRP_MJ_CLEANUP;
#ifdef fun_IRP_MJ_CLEANUP
        status = fun_IRP_MJ_CLEANUP(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CLOSE
        case IRP_MJ_CLOSE:
        ps->MajorFunction = IRP_MJ_CLOSE;
#ifdef fun_IRP_MJ_CLOSE
        status = fun_IRP_MJ_CLOSE(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_CREATE
        case IRP_MJ_CREATE:
        ps->MajorFunction = IRP_MJ_CREATE;
#ifdef fun_IRP_MJ_CREATE
        status = fun_IRP_MJ_CREATE(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_DEVICE_CONTROL
        case IRP_MJ_DEVICE_CONTROL:
        ps->MajorFunction = IRP_MJ_DEVICE_CONTROL;
#ifdef fun_IRP_MJ_DEVICE_CONTROL
        status = fun_IRP_MJ_DEVICE_CONTROL(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FILE_SYSTEM_CONTROL
        case IRP_MJ_FILE_SYSTEM_CONTROL:
        ps->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
#ifdef fun_IRP_MJ_FILE_SYSTEM_CONTROL
        status = fun_IRP_MJ_FILE_SYSTEM_CONTROL(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_FLUSH_BUFFERS
        case IRP_MJ_FLUSH_BUFFERS:
        ps->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
#ifdef fun_IRP_MJ_FLUSH_BUFFERS
        status = fun_IRP_MJ_FLUSH_BUFFERS(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_INTERNAL_DEVICE_CONTROL
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        ps->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
#ifdef fun_IRP_MJ_INTERNAL_DEVICE_CONTROL
        status = fun_IRP_MJ_INTERNAL_DEVICE_CONTROL(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
           sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
           sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif


#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_LOCK_CONTROL
        case IRP_MJ_LOCK_CONTROL:
              ps->MajorFunction = IRP_MJ_LOCK_CONTROL;
#ifdef fun_IRP_MJ_LOCK_CONTROL
        status = fun_IRP_MJ_LOCK_CONTROL(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_QUERY_INFORMATION
        case IRP_MJ_QUERY_INFORMATION:
        ps->MajorFunction = IRP_MJ_QUERY_INFORMATION;
#ifdef fun_IRP_MJ_QUERY_INFORMATION
        status = fun_IRP_MJ_QUERY_INFORMATION(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_READ
        case IRP_MJ_READ:
        ps->MajorFunction = IRP_MJ_READ;
#ifdef fun_IRP_MJ_READ
        status = fun_IRP_MJ_READ(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SET_INFORMATION
        case IRP_MJ_SET_INFORMATION:
        ps->MajorFunction = IRP_MJ_SET_INFORMATION;
#ifdef fun_IRP_MJ_SET_INFORMATION
        status = fun_IRP_MJ_SET_INFORMATION(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SYSTEM_CONTROL
        case IRP_MJ_SYSTEM_CONTROL:
        ps->MajorFunction = IRP_MJ_SYSTEM_CONTROL;
#ifdef fun_IRP_MJ_SYSTEM_CONTROL
        status = fun_IRP_MJ_SYSTEM_CONTROL(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_WRITE
        case IRP_MJ_WRITE:
        ps->MajorFunction = IRP_MJ_WRITE;
#ifdef fun_IRP_MJ_WRITE
        status = fun_IRP_MJ_WRITE(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP
        case IRP_MJ_PNP:
        ps->MajorFunction = IRP_MJ_PNP;


 
        if (ps->MinorFunction == IRP_MN_START_DEVICE) 
        {
            SdvAssume(!sdv_start_irp_already_issued);
        }

        if (ps->MinorFunction == IRP_MN_CANCEL_REMOVE_DEVICE) 
	{
	   SdvAssume(!sdv_remove_irp_already_issued);
	}
	if (ps->MinorFunction == IRP_MN_REMOVE_DEVICE) 
	{
            sdv_remove_irp_already_issued = 1;
#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_MN_REMOVE_DEVICE
            SdvExit();
#endif
        }

#ifdef fun_IRP_MJ_PNP
        
#ifdef SDV_NON_BUS_MN_FUNCTIONS
        sdv_SetIrpMinorFunctionNonBusDriver(pirp);
#endif

#ifdef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_PNP_WITH_IRP_MN_QUERY_DEVICE_RELATIONS
     sdv_SetIrpMinorFunctionBusDriver(pirp);
#endif


#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_QUERY
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_INTERFACE);
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_STOP_DEVICE);
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_REMOVE_DEVICE);
#endif


#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_START_DEVICE
    SdvAssume(ps->MinorFunction != IRP_MN_START_DEVICE);
#endif

        status = fun_IRP_MJ_PNP(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if ((SDV_HARNESS==SDV_PNP_HARNESS)||(SDV_HARNESS==SDV_POWER_DOWN_PNP_HARNESS_HARNESS)||(SDV_HARNESS==SDV_SMALL_START_SEQUENCE_HARNESS))
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_POWER
        case IRP_MJ_POWER:
        ps->MajorFunction = IRP_MJ_POWER;
        
#ifdef fun_IRP_MJ_POWER
#ifndef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_MN_FUNCTIONS
        sdv_SetPowerIrpMinorFunction(pirp);
#endif
#ifdef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_IRP_MN_SET_POWER
       sdv_SetPowerIrpMinorFunctionSetPower(pirp);
#endif

        status = fun_IRP_MJ_POWER(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if ((SDV_HARNESS==SDV_PNP_HARNESS)||(SDV_HARNESS==SDV_POWER_DOWN_PNP_HARNESS_HARNESS)||(SDV_HARNESS==SDV_SMALL_START_SEQUENCE_HARNESS))
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif


#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_SHUTDOWN
        case IRP_MJ_SHUTDOWN:
        ps->MajorFunction = IRP_MJ_SHUTDOWN;
#ifdef fun_IRP_MJ_SHUTDOWN
        status = fun_IRP_MJ_SHUTDOWN(po,pirp);
        #if ( SDV_IS_XFLAT_HARNESS_CANCEL() )
            sdv_RunCancelFunction(po,pirp);
        #endif
        #if (SDV_HARNESS==SDV_PNP_HARNESS)
            sdv_RunISRRoutines(sdv_kinterrupt,sdv_pDpcContext);
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,sdv_pv1,sdv_pv2);
            sdv_RunIoDpcRoutines(sdv_kdpc,po,pirp,sdv_pIoDpcContext);
        #endif
#else
        status = sdv_DoNothing();
#endif
        break;
#endif


        default:
        status = sdv_DoNothing();
        break;
    }
    
    sdv_stub_dispatch_end(status,pirp);

    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}




NTSTATUS 
sdv_RunDispatchPnp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_PNP" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    
    
    UCHAR minor_function = (UCHAR) SdvKeepChoice();

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);

    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    


    ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    
    sdv_dpc_io_registered = FALSE;
    
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_PNP;
 
    if (ps->MinorFunction == IRP_MN_START_DEVICE) 
    {
        SdvAssume(!sdv_start_irp_already_issued);
    }

    if (ps->MinorFunction == IRP_MN_CANCEL_REMOVE_DEVICE) 
	{
	   SdvAssume(!sdv_remove_irp_already_issued);
	}
	if (ps->MinorFunction == IRP_MN_REMOVE_DEVICE) 
	{
            sdv_remove_irp_already_issued = 1;
#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_MN_REMOVE_DEVICE
            SdvExit();
#endif
    }

#ifdef fun_IRP_MJ_PNP
        
#ifdef SDV_NON_BUS_MN_FUNCTIONS
        sdv_SetIrpMinorFunctionNonBusDriver(pirp);
#endif

#ifdef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_PNP_WITH_IRP_MN_QUERY_DEVICE_RELATIONS
     sdv_SetIrpMinorFunctionBusDriver(pirp);
#endif


#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_QUERY
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_INTERFACE);
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_STOP_DEVICE);
    SdvAssume(ps->MinorFunction != IRP_MN_QUERY_REMOVE_DEVICE);
#endif 


#ifdef SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_WITH_IRP_MN_START_DEVICE
    SdvAssume(ps->MinorFunction != IRP_MN_START_DEVICE);
#endif 

        status = fun_IRP_MJ_PNP(po,pirp);

#endif
    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}





NTSTATUS 
sdv_RunDispatchPower(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_POWER" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
        
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
 
    sdv_SetStatus(pirp);

    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    


    ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    
    sdv_dpc_io_registered = FALSE;
    
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_POWER;
    
#ifdef fun_IRP_MJ_POWER
#ifndef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_MN_FUNCTIONS
        sdv_SetPowerIrpMinorFunction(pirp);
#endif
#ifdef SDV_FLAT_HARNESS_MODIFIER_IRP_MJ_POWER_WITH_IRP_MN_SET_POWER
       sdv_SetPowerIrpMinorFunctionSetPower(pirp);
#endif
        status = fun_IRP_MJ_POWER(po,pirp);
#endif
    
    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}










NTSTATUS 
sdv_RunDispatchWrite(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_WRITE" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_WRITE;
#ifdef fun_IRP_MJ_WRITE
        status = fun_IRP_MJ_WRITE(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);

    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
		return status;
}



NTSTATUS 
sdv_RunDispatchRead(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_READ" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    
	/* Valid Minor function codes for file system and filter drivers

	
		IRP_MN_COMPLETE
		 
		IRP_MN_COMPLETE_MDL
		 
		IRP_MN_COMPLETE_MDL_DPC
		 
		IRP_MN_COMPRESSED
		 
		IRP_MN_DPC
		 
		IRP_MN_MDL
		 
		IRP_MN_MDL_DPC
		 
		IRP_MN_NORMAL

    */

    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_READ;
#ifdef fun_IRP_MJ_READ
        status = fun_IRP_MJ_READ(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
		return status;
}



NTSTATUS 
sdv_RunDispatchInternalDeviceControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_INTERNAL_DEVICE_CONTROL" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
#ifdef fun_IRP_MJ_INTERNAL_DEVICE_CONTROL
        status = fun_IRP_MJ_INTERNAL_DEVICE_CONTROL(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}


NTSTATUS 
sdv_RunDispatchDeviceControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_DEVICE_CONTROL" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_DEVICE_CONTROL;
#ifdef fun_IRP_MJ_DEVICE_CONTROL
        status = fun_IRP_MJ_DEVICE_CONTROL(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}



NTSTATUS 
sdv_RunDispatchCreate(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_CREATE" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_CREATE;
#ifdef fun_IRP_MJ_CREATE
        status = fun_IRP_MJ_CREATE(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}


NTSTATUS 
sdv_RunDispatchClose(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_CLOSE" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_CLOSE;
#ifdef fun_IRP_MJ_CLOSE
        status = fun_IRP_MJ_CLOSE(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
		return status;
}


NTSTATUS 
sdv_RunDispatchSystemControl(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_SYSTEM_CONTROL" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_SYSTEM_CONTROL;
#ifdef fun_IRP_MJ_SYSTEM_CONTROL
        status = fun_IRP_MJ_SYSTEM_CONTROL(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}




NTSTATUS 
sdv_RunDispatchShutdown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_SHUTDOWN" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_SHUTDOWN;
#ifdef fun_IRP_MJ_SHUTDOWN
        status = fun_IRP_MJ_SHUTDOWN(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}





NTSTATUS 
sdv_RunDispatchCleanup(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate dispatch function, and then applies it to the
    IRP argument.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_CLEANUP" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    UCHAR minor_function = (UCHAR) SdvKeepChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    pirp->PendingReturned = 0;
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;

	ps->MinorFunction = minor_function;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_dpc_io_registered = FALSE;
    sdv_stub_dispatch_begin();
    ps->MajorFunction = IRP_MJ_CLEANUP;
#ifdef fun_IRP_MJ_CLEANUP
        status = fun_IRP_MJ_CLEANUP(po,pirp);
#endif

    sdv_stub_dispatch_end(status,pirp);
    sdv_end_info = (ULONG_PTR) (pirp->IoStatus.Information);
    return status;
}




VOID 
sdv_RunCancelFunction(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

    Finds the appropriate Cancel function for the IRP, and then call that 
    cancelation routine in the driver.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct Cancel routine for the driver.
    The Cancel routine is set by calling IoSetCancelationRoutine on the Irp.
    After the Cancelation routine has pirp->CancelRoutine should be set to NULL
    This is done in SDV_MACRO_STUB_CANCEL_END;.

*/
{
   
#ifdef fun_DRIVER_CANCEL_1    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_1)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_1(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_2    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_2)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_2(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_3    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_3)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_3(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_4    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_4)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_4(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_5    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_5)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_5(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_6    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_6)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_6(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
#ifdef fun_DRIVER_CANCEL_7    
    if(pirp->CancelRoutine!=0&&pirp->CancelRoutine==fun_DRIVER_CANCEL_7)
	{
	  SDV_MACRO_STUB_CANCEL_BEGIN(pirp);
	  pirp->Cancel = TRUE;
	  fun_DRIVER_CANCEL_7(po, pirp);
	  pirp->Cancel = FALSE;
	  SDV_MACRO_STUB_CANCEL_END(pirp);
	}
#endif
}








BOOLEAN
sdv_RunPowerCompletionRoutines(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus,
    PREQUEST_POWER_COMPLETE CompletionFunction
    )
    
/*

Routine Description:

    Finds the appropriate power completion function for the IRP, and then calls that 
    power completion routine in the driver.

Arguments:

    DeviceObject       - pointer to a device object.
    MinorFunction      - IRP_MN_SET_POWER,IRP_MN_QUERY_POWER or IRP_MN_WAIT_WAKE
	  PowerState         - Devive or System Power State determined by driver in Call to PoRequestPowerIrp.    
    Context            - context pointer.
    IoStatus           - IoStatus block associated with power Irp
    CompletionFunction - PowerCompletion routine passed to PoRequestPowerIrp.

Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct power completion routine for the driver.
    Completion routine us set by calling PoRequestPowerIrp.
    

*/
{
BOOLEAN CompletionFunction_run=FALSE;
#ifdef fun_REQUEST_POWER_COMPLETE_1
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_1)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_1(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_2
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_2)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_2(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_3
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_3)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_3(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_4
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_4)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_4(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_5
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_5)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_5(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_6
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_6)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_6(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_7
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_7)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_7(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_8
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_8)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_8(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_9
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_9)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_9(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction=TRUE;
}
#endif
#ifdef fun_REQUEST_POWER_COMPLETE_10
if(CompletionFunction == fun_REQUEST_POWER_COMPLETE_10)
{
    sdv_stub_power_completion_begin();
    fun_REQUEST_POWER_COMPLETE_10(DeviceObject,MinorFunction,PowerState,Context,IoStatus);   
    SDV_IRQL_POP();
    CompletionFunction_run=TRUE;
}
#endif
return CompletionFunction_run;
}


void
sdv_RunIoQueueWorkItems(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    )
/*

Routine Description:

    Finds the appropriate worker routine for the workitem, and then calls that 
    routine in the driver.

Arguments:

    IoWorkItem - IO Work Item object.
	  WorkerRoutine - pointer to the work item routine.    
    WORK_QUEUE_TYPE - Queue type that stipulates the type of system worker thread to run the work item on.
    Context - driver specific information for the workitem.

Return value:

    we have no return value for this function.

Notes:
    
    
    The worker routine is set by calling IoQueueWorkItem.
    

*/

{
  
  #ifdef fun_IO_WORKITEM_ROUTINE_1
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_1)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_1(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_2
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_2)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_2(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_3
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_3)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_3(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_4
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_4)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_4(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_5
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_5)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_5(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_6
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_6)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_6(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_7
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_7)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_7(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_8
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_8)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_8(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_9
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_9)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_9(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_10
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_10)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_10(sdv_p_devobj_fdo,Context);  
      SDV_IRQL_POP();
  }
  #endif
}


void
sdv_RunExQueueWorkItems(
    PWORKER_THREAD_ROUTINE WorkerRoutine,
    PVOID Context
    )
/*

Routine Description:

    Finds the appropriate worker routine for the workitem, and then calls that 
    routine in the driver.

Arguments:

	  WorkerRoutine - pointer to the work item routine.    
    Context - driver specific information for the workitem.

Return value:

    we have no return value for this function.

Notes:
    
    
    The worker routine is set by calling ExInitializeWorkItem.
    

*/

{
  
  #ifdef fun_WORKER_THREAD_ROUTINE_1
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_1)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_1(Context);  
      SDV_IRQL_POP();
  }
  #endif

  #ifdef fun_WORKER_THREAD_ROUTINE_2
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_2)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_2(Context);  
      SDV_IRQL_POP();
  }
  #endif

  #ifdef fun_WORKER_THREAD_ROUTINE_3
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_3)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_3(Context);  
      SDV_IRQL_POP();
  }
  #endif

  #ifdef fun_WORKER_THREAD_ROUTINE_4
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_4)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_4(Context);  
      SDV_IRQL_POP();
  }
  #endif

  #ifdef fun_WORKER_THREAD_ROUTINE_5
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_5)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_5(Context);  
      SDV_IRQL_POP();
  }
  #endif

  #ifdef fun_WORKER_THREAD_ROUTINE_6
  if(WorkerRoutine == fun_WORKER_THREAD_ROUTINE_6)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_WORKER_THREAD_ROUTINE_6(Context);  
      SDV_IRQL_POP();
  }
  #endif
  
}










NTSTATUS
sdv_RunIoCompletionRoutines(
    __in PDEVICE_OBJECT DeviceObject, 
    __in PIRP Irp, 
    __in_opt PVOID Context,
    BOOLEAN* Completion_Routine_Called
    )
/*

Routine Description:

    Finds the appropriate completion function for the IRP, and then calls that 
    completion routine in the driver.

Arguments:

    DeviceObject - pointer to a device object.
	po - pointer to the device object.    
    Context - context pointer.

Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct completion routine for the driver.
    The completion routine is set by calling IoSetConpletionRoutine on the Irp.
    

*/
{
PIO_STACK_LOCATION irpsp;    
NTSTATUS Status;
irpsp = sdv_IoGetNextIrpStackLocation(Irp);
Status=STATUS_SUCCESS;
#ifdef fun_IO_COMPLETION_ROUTINE_1
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_1)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_1(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_2
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_2)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_2(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_3
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_3)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_3(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_4
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_4)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_4(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_5
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_5)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_5(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_6
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_6)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_6(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_7
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_7)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_7(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_8
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_8)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_8(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_9
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_9)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_9(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_10
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_10)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_10(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_11
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_11)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_11(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
#ifdef fun_IO_COMPLETION_ROUTINE_12
if(irpsp->CompletionRoutine == fun_IO_COMPLETION_ROUTINE_12)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    Status=fun_IO_COMPLETION_ROUTINE_12(DeviceObject,Irp,Context);  
    SDV_IRQL_POP();
    *Completion_Routine_Called=TRUE;
}
#endif
return Status;
}


void
sdv_RunIoQueueWorkItemsEx(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE_EX WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    )
/*

Routine Description:

    Finds the appropriate worker routine for the workitem, and then calls that 
    routine in the driver.

Arguments:

    IoWorkItem - IO Work Item object.
	  WorkerRoutine - pointer to the work item routine.    
    WORK_QUEUE_TYPE - Queue type that stipulates the type of system worker thread to run the work item on.
    Context - driver specific information for the workitem.

Return value:

    we have no return value for this function.

Notes:
    
    
    The worker routine is set by calling IoQueueWorkItemEx.
    

*/

{
  PVOID  IoObject=(PVOID)sdv_p_devobj_fdo;
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_1
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_1)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_1(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_2
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_2)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_2(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_3
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_3)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_3(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_3
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_3)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_3(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_4
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_4)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_4(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_5
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_5)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_5(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_6
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_6)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_6(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_7
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_7)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_7(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_8
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_8)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_8(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_9
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_9)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_9(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
  #ifdef fun_IO_WORKITEM_ROUTINE_EX_10
  if(WorkerRoutine == fun_IO_WORKITEM_ROUTINE_EX_10)
  {
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      fun_IO_WORKITEM_ROUTINE_EX_10(IoObject,Context,IoWorkItem);  
      SDV_IRQL_POP();
  }
  #endif
}


VOID
sdv_RunISRRoutines(
    struct _KINTERRUPT *ki, 
    PVOID pv1
    )
/*

Routine Description:

    Finds the appropriate ISR function and then calls that 
    ISR routine in the driver.

Arguments:

    ki - Points to the Interrupt object for this device.
	  pv1 - Context.    
    
Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct ISR routine for the driver.
    The ISR routine is set by calling IoConnectInterrupt function.
    

*/
{
#ifdef fun_KSERVICE_ROUTINE_1
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_1)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_1(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KSERVICE_ROUTINE_2
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_2)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_2(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KSERVICE_ROUTINE_3
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_3)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_3(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KSERVICE_ROUTINE_4
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_4)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_4(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KSERVICE_ROUTINE_5
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_5)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_5(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KSERVICE_ROUTINE_6
       if(sdv_isr_routine==fun_KSERVICE_ROUTINE_6)
       {
           SDV_IRQL_PUSH(SDV_DIRQL);
           fun_KSERVICE_ROUTINE_6(ki, pv1);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif

}


VOID
sdv_RunKeDpcRoutines(
    IN struct _KDPC *kdpc, 
    IN PVOID pDpcContext, 
    IN PVOID pv2, 
    IN PVOID pv3
    )
/*

Routine Description:

    Finds the appropriate Dpc function and then calls that 
    Dpc routine in the driver.

Arguments:

    kdpc - Pointer to the KDPC structure for the DPC object.
	  pDpcContext -  Caller-supplied pointer to driver-defined context information that was specified in a previous call to KeInitializeDpc.
	  pv2 -  Caller-supplied pointer to driver-supplied information that was specified in a previous call to KeInsertQueueDpc.  
	  pv3 -  Caller-supplied pointer to driver-supplied information that was specified in a previous call to KeInsertQueueDpc.
    
Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct DPC routine for the driver.

*/
{
#ifdef fun_KDEFERRED_ROUTINE_1
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_1&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_1(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_2
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_2&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_2(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_3
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_3&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_3(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_4
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_4&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_4(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_5
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_5&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_5(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_6
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_6&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_6(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_7
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_7&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_7(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_8
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_8&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_8(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
#ifdef fun_KDEFERRED_ROUTINE_9
       if(kdpc->DeferredRoutine==fun_KDEFERRED_ROUTINE_9&&sdv_dpc_ke_registered)
       {
           SDV_IRQL_PUSH(DISPATCH_LEVEL);
           fun_KDEFERRED_ROUTINE_9(kdpc,pDpcContext,sdv_pv2,sdv_pv3);
           SDV_IRQL_POPTO(PASSIVE_LEVEL);
       }
#endif
}




VOID
sdv_RunIoDpcRoutines(
    IN PKDPC  Dpc,    
    IN struct _DEVICE_OBJECT  *DeviceObject,    
    IN struct _IRP  *Irp,    
    IN PVOID  Context
    )
/*

Routine Description:

    Finds the appropriate Dpc function and then calls that 
    Dpc routine in the driver.

Arguments:

    dpc          -  Pointer to the KDPC structure for the DPC object.
	  DeviceObject -  Caller-supplied pointer to a DEVICE_OBJECT structure. This is the device object for the target device, previously created by the driver's AddDevice routine
	  Irp          -  Caller-supplied pointer to an IRP structure that describes the I/O operation. 
	  Context      -  Caller-supplied pointer to driver-defined context information, specified in a previous call to IoRequestDpc.
    
Return value:

    we have no return value for this function.

Notes:
    
    We will execute the correct IO DPC routine for the driver.

*/
{
#ifdef fun_IO_DPC_ROUTINE_1
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_1&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_1(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
#ifdef fun_IO_DPC_ROUTINE_2
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_2&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_2(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
#ifdef fun_IO_DPC_ROUTINE_3
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_3&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_3(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
#ifdef fun_IO_DPC_ROUTINE_4
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_4&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_4(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
#ifdef fun_IO_DPC_ROUTINE_5
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_5&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_5(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
#ifdef fun_IO_DPC_ROUTINE_6
if(sdv_io_dpc==fun_IO_DPC_ROUTINE_6&&sdv_dpc_io_registered)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    fun_IO_DPC_ROUTINE_6(Dpc,DeviceObject,Irp,Context); 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}
#endif 
}


BOOLEAN
sdv_RunKSynchronizeRoutines(
    PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID  Context
    )
/*

Routine Description:

    Finds the appropriate KSYNCHRONIZE function and then calls that 
    KSYNCHRONIZE routine in the driver.

Arguments:

    Context      -  Caller-supplied pointer to driver-defined context information, specified in a previous call to KeSynchronizeExecution.
    
Return value:

    we have FALSE in case no KSYNCHRONIZE declared otherwise will return value from KSYNCHRONIZE routine.

Notes:
    
    We will execute the correct KSYNCHRONIZE routine for the driver.

*/
{

#ifdef fun_KSYNCHRONIZE_ROUTINE_1
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_1)
{
    return fun_KSYNCHRONIZE_ROUTINE_1(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_2
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_2)
{
    return fun_KSYNCHRONIZE_ROUTINE_2(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_3
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_3)
{
    return fun_KSYNCHRONIZE_ROUTINE_3(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_4
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_4)
{
    return fun_KSYNCHRONIZE_ROUTINE_4(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_5
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_5)
{
    return fun_KSYNCHRONIZE_ROUTINE_5(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_6
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_6)
{
    return fun_KSYNCHRONIZE_ROUTINE_6(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_7
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_7)
{
    return fun_KSYNCHRONIZE_ROUTINE_7(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_8
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_8)
{
    return fun_KSYNCHRONIZE_ROUTINE_8(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_9
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_9)
{
    return fun_KSYNCHRONIZE_ROUTINE_9(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_10
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_10)
{
    return fun_KSYNCHRONIZE_ROUTINE_10(Context); 
}
#endif 

#ifdef fun_KSYNCHRONIZE_ROUTINE_11
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_11)
{
    return fun_KSYNCHRONIZE_ROUTINE_11(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_12
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_12)
{
    return fun_KSYNCHRONIZE_ROUTINE_12(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_13
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_13)
{
    return fun_KSYNCHRONIZE_ROUTINE_13(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_14
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_14)
{
    return fun_KSYNCHRONIZE_ROUTINE_14(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_15
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_15)
{
    return fun_KSYNCHRONIZE_ROUTINE_15(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_16
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_16)
{
    return fun_KSYNCHRONIZE_ROUTINE_16(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_17
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_17)
{
    return fun_KSYNCHRONIZE_ROUTINE_17(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_18
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_18)
{
    return fun_KSYNCHRONIZE_ROUTINE_18(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_19
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_19)
{
    return fun_KSYNCHRONIZE_ROUTINE_19(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_20
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_20)
{
    return fun_KSYNCHRONIZE_ROUTINE_20(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_21
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_21)
{
    return fun_KSYNCHRONIZE_ROUTINE_21(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_22
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_22)
{
    return fun_KSYNCHRONIZE_ROUTINE_22(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_23
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_23)
{
    return fun_KSYNCHRONIZE_ROUTINE_23(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_24
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_24)
{
    return fun_KSYNCHRONIZE_ROUTINE_24(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_25
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_25)
{
    return fun_KSYNCHRONIZE_ROUTINE_25(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_26
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_26)
{
    return fun_KSYNCHRONIZE_ROUTINE_26(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_27
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_27)
{
    return fun_KSYNCHRONIZE_ROUTINE_27(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_28
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_28)
{
    return fun_KSYNCHRONIZE_ROUTINE_28(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_29
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_29)
{
    return fun_KSYNCHRONIZE_ROUTINE_29(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_30
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_30)
{
    return fun_KSYNCHRONIZE_ROUTINE_30(Context); 
}
#endif 

#ifdef fun_KSYNCHRONIZE_ROUTINE_31
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_31)
{
    return fun_KSYNCHRONIZE_ROUTINE_31(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_32
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_32)
{
    return fun_KSYNCHRONIZE_ROUTINE_32(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_33
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_33)
{
    return fun_KSYNCHRONIZE_ROUTINE_33(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_34
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_34)
{
    return fun_KSYNCHRONIZE_ROUTINE_34(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_35
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_35)
{
    return fun_KSYNCHRONIZE_ROUTINE_35(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_36
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_36)
{
    return fun_KSYNCHRONIZE_ROUTINE_36(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_37
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_37)
{
    return fun_KSYNCHRONIZE_ROUTINE_37(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_38
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_38)
{
    return fun_KSYNCHRONIZE_ROUTINE_38(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_39
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_39)
{
    return fun_KSYNCHRONIZE_ROUTINE_39(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_40
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_40)
{
    return fun_KSYNCHRONIZE_ROUTINE_40(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_41
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_41)
{
    return fun_KSYNCHRONIZE_ROUTINE_41(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_42
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_42)
{
    return fun_KSYNCHRONIZE_ROUTINE_42(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_43
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_43)
{
    return fun_KSYNCHRONIZE_ROUTINE_43(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_44
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_44)
{
    return fun_KSYNCHRONIZE_ROUTINE_44(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_45
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_45)
{
    return fun_KSYNCHRONIZE_ROUTINE_45(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_46
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_46)
{
    return fun_KSYNCHRONIZE_ROUTINE_46(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_47
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_47)
{
    return fun_KSYNCHRONIZE_ROUTINE_47(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_48
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_48)
{
    return fun_KSYNCHRONIZE_ROUTINE_48(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_49
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_49)
{
    return fun_KSYNCHRONIZE_ROUTINE_49(Context); 
}
#endif 
#ifdef fun_KSYNCHRONIZE_ROUTINE_50
if(SynchronizeRoutine==fun_KSYNCHRONIZE_ROUTINE_50)
{
    return fun_KSYNCHRONIZE_ROUTINE_50(Context); 
}
#endif 
return FALSE;
}



NTSTATUS 
sdv_RunQueryDeviceRelations(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Call PNP dispatch with IRP_MN_QUERY_DEVICE_RELATIONS

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    

*/
{
    NTSTATUS status=STATUS_SUCCESS;
    long type_choice = SdvMakeChoice();
    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    sdv_end_info = sdv_start_info = (ULONG_PTR) (pirp->IoStatus.Information);
    sdv_SetStatus(pirp);
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    ps->MajorFunction = IRP_MJ_PNP;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
#ifdef BUS_RELATIONS
    switch(type_choice)
    {
    case 0:
      ps->Parameters.QueryDeviceRelations.Type  = BusRelations;
      break;
    case 0:
      ps->Parameters.QueryDeviceRelations.Type  = TargetDeviceRelation;
      break;
    default:
      ps->Parameters.QueryDeviceRelations.Type  = EjectionRelations;  
      break;
    }
#else
    ps->Parameters.QueryDeviceRelations.Type  = RemovalRelations;
#endif

#ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
#endif

	return status;
}


NTSTATUS 
sdv_RunStartDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the start device function

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

Notes:
    
    We're interacting with SDV meta-data here.  If "fun_IRP_MJ_CREATE" is
    defined in the meta-data, then we're calling it.  Otherwise: we call
    the sdv_DoNothing() function.

*/
{
    NTSTATUS status=STATUS_SUCCESS;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_START_DEVICE;
    pirp->IoStatus.Status = STATUS_SUCCESS;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    
    sdv_SetStatus(pirp);
    sdv_start_irp_already_issued = 1;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();

    sdv_stub_dispatch_begin();
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    sdv_stub_dispatch_end(status,pirp);

    return status;
}


NTSTATUS 
sdv_RunRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the remove device function

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status=STATUS_SUCCESS;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_REMOVE_DEVICE;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    
    sdv_stub_dispatch_begin();


    sdv_SetStatus(pirp);

    
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    sdv_stub_dispatch_end(status,pirp);

    return status;

}


NTSTATUS 
sdv_RunQueryCapRequirements(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_QUERY_CAPABILITIES irp thru the driver
   The PnP manager sends this IRP to get the capabilities of a device

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_stub_dispatch_begin();
    sdv_SetStatus(pirp);
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    sdv_stub_dispatch_end(status,pirp);

    return status;

}


NTSTATUS 
sdv_RunQueryRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_QUERY_REMOVE_DEVICE irp thru the driver

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_QUERY_REMOVE_DEVICE;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    return status;

}



NTSTATUS 
sdv_RunSurpriseRemoveDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_SURPRISE_REMOVAL irp thru the driver

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction =IRP_MN_SURPRISE_REMOVAL;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    return status;

}


NTSTATUS 
sdv_RunResRequirements(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_FILTER_RESOURCE_REQUIREMENTS irp thru the driver
   The PnP manager sends this IRP to a device stack so the function driver can adjust the resources required by the device.
   BUS driver should not handle this IRP.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_FILTER_RESOURCE_REQUIREMENTS;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    return status;

}



NTSTATUS 
sdv_RunQueryDeviceState(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_QUERY_DEVICE_STATE irp thru the driver
   No MSDN documentation on this Irp.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_PNP;
    ps->MinorFunction = IRP_MN_QUERY_PNP_DEVICE_STATE;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    sdv_SetStatus(pirp);
   
    #ifdef fun_IRP_MJ_PNP
        status = fun_IRP_MJ_PNP(po,pirp);
    #endif
    return status;

}



NTSTATUS 
sdv_RunQueryPowerUp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_QUERY_POWER irp thru the driver
   This IRP queries a device to determine whether the system power state or the device power state can be changed.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_QUERY_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = SystemPowerState;
    sdv_SetStatus(pirp);
    ps->Parameters.Power.State.SystemState=PowerSystemWorking;
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}


NTSTATUS 
sdv_RunQueryPowerUpDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_QUERY_POWER irp thru the driver
   This IRP queries a device to determine whether the system power state or the device power state can be changed.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_QUERY_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = DevicePowerState;
    sdv_SetStatus(pirp);
    ps->Parameters.Power.State.DeviceState=PowerDeviceD0;
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}




NTSTATUS 
sdv_RunQueryPowerDown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_SET_POWER irp thru the driver
   This IRP queries a device to determine whether the system power state or the device power state can be changed.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG nd_state = SdvMakeChoice();
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_QUERY_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = SystemPowerState;
    sdv_SetStatus(pirp);
    switch (nd_state)
    { 
      case 0:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping1;
      break;
      case 1:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping2;
      break;
      case 2:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping3;
      break;
      case 3:
      ps->Parameters.Power.State.SystemState=PowerSystemHibernate;
      break;
      case 4:
      ps->Parameters.Power.State.SystemState=PowerSystemShutdown;
      break;
      default:
      ps->Parameters.Power.State.SystemState=PowerSystemMaximum;
      break;
    }
       
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}



NTSTATUS 
sdv_RunSetPowerUp(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_SET_POWER irp thru the driver
   This IRP notifies a driver of a change to the system power state or sets the device power state for a device.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_SET_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = SystemPowerState;
    sdv_SetStatus(pirp);
    ps->Parameters.Power.State.SystemState=PowerSystemWorking;
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}


NTSTATUS 
sdv_RunSetPowerDown(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_SET_POWER irp thru the driver
   This IRP notifies a driver of a change to the system power state or sets the device power state for a device.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG nd_state = SdvMakeChoice();
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_SET_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = SystemPowerState;
    sdv_SetStatus(pirp);
    switch (nd_state)
    { 
      case 0:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping1;
      break;
      case 1:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping2;
      break;
      case 2:
      ps->Parameters.Power.State.SystemState=PowerSystemSleeping3;
      break;
      case 3:
      ps->Parameters.Power.State.SystemState=PowerSystemHibernate;
      break;
      case 4:
      ps->Parameters.Power.State.SystemState=PowerSystemShutdown;
      break;
      default:
      ps->Parameters.Power.State.SystemState=PowerSystemMaximum;
      break;
    }
    
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}


NTSTATUS 
sdv_RunSetPowerUpDevice(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
/*

Routine Description:

   Run the IRP_MN_SET_POWER irp thru the driver
   This IRP notifies a driver of a change to the system power state or sets the device power state for a device.

Arguments:

    po - pointer to the device object.    
    pirp - pointer to the irp that we're using.

Return value:

    we're passing the dispatch's return value back.

*/
{

    NTSTATUS status;

    PIO_STACK_LOCATION ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    ps->MajorFunction = IRP_MJ_POWER;
    ps->MinorFunction = IRP_MN_SET_POWER;
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    ps->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
    ps->Parameters.Power.Type = DevicePowerState;
    sdv_SetStatus(pirp);
    ps->Parameters.Power.State.DeviceState=PowerDeviceD0;
    #ifdef fun_IRP_MJ_POWER
        status = fun_IRP_MJ_POWER(po,pirp);
    #endif
    return status;

}


VOID
sdv_RunStartIo(
    PDEVICE_OBJECT po, 
    PIRP pirp
    )
{
    sdv_stub_startio_begin();
#ifdef fun_DriverStartIo
    pirp->CancelRoutine = NULL;
    pirp->Cancel = FALSE;
    fun_DriverStartIo(po, pirp);
#else
    sdv_DoNothing();
#endif
    sdv_stub_startio_end();
}


VOID
sdv_RunUnload(
    PDRIVER_OBJECT pdrivo
    )
{
#ifdef fun_DriverUnload
    fun_DriverUnload(pdrivo);
#else
    sdv_DoNothing();
#endif
}



BOOLEAN
sdv_CheckDispatchRoutines()
{
#if defined(fun_IRP_MJ_CLEANUP) || \
	defined(fun_IRP_MJ_CLOSE) || \
	defined(fun_IRP_MJ_CREATE) ||\
	defined(fun_IRP_MJ_DEVICE_CONTROL) || \
	defined(fun_IRP_MJ_FILE_SYSTEM_CONTROL) || \
	defined(fun_IRP_MJ_FLUSH_BUFFERS) || \
	defined(fun_IRP_MJ_INTERNAL_DEVICE_CONTROL) || \
	defined(fun_IRP_MJ_LOCK_CONTROL) || \
	defined(fun_IRP_MJ_QUERY_INFORMATION) || \
	defined(fun_IRP_MJ_READ) || \
	defined(fun_IRP_MJ_SET_INFORMATION) || \
	defined(fun_IRP_MJ_SYSTEM_CONTROL) || \
	defined(fun_IRP_MJ_WRITE) || \
	defined(fun_IRP_MJ_PNP) || \
	defined(fun_IRP_MJ_POWER) 

	return TRUE;
#else
    return FALSE;
#endif
}

BOOLEAN
sdv_CheckStartIoRoutines()
{
#if defined(fun_DriverStartIo)
	return TRUE;
#else
    return FALSE;
#endif
}



BOOLEAN
sdv_CheckDpcRoutines()
{
#if defined(fun_KDEFERRED_ROUTINE_1) || \
	defined(fun_KDEFERRED_ROUTINE_2) || \
	defined(fun_KDEFERRED_ROUTINE_3) || \
	defined(fun_KDEFERRED_ROUTINE_4) || \
	defined(fun_KDEFERRED_ROUTINE_5) || \
	defined(fun_KDEFERRED_ROUTINE_6) || \
	defined(fun_KDEFERRED_ROUTINE_7) || \
	defined(fun_KDEFERRED_ROUTINE_8) || \
	defined(fun_KDEFERRED_ROUTINE_9) 
	return TRUE;
#else
    return FALSE;
#endif
}



BOOLEAN
sdv_CheckIsrRoutines()
{
#if defined(fun_KSERVICE_ROUTINE_1) || \
	defined(fun_KSERVICE_ROUTINE_2) || \
	defined(fun_KSERVICE_ROUTINE_3) ||\
	defined(fun_KSERVICE_ROUTINE_4) || \
	defined(fun_KSERVICE_ROUTINE_5) || \
	defined(fun_KSERVICE_ROUTINE_6) 
	return TRUE;
#else
    return FALSE;
#endif
}



BOOLEAN
sdv_CheckCancelRoutines()
{
#if defined(fun_DRIVER_CANCEL_1) || \
	defined(fun_DRIVER_CANCEL_2) || \
	defined(fun_DRIVER_CANCEL_3) || \
	defined(fun_DRIVER_CANCEL_4) || \
	defined(fun_DRIVER_CANCEL_5) || \
	defined(fun_DRIVER_CANCEL_6) || \
	defined(fun_DRIVER_CANCEL_7)
	return TRUE;
#else
    return FALSE;
#endif
}






BOOLEAN
sdv_CheckCancelRoutines1()
{
#if defined(fun_DRIVER_CANCEL_1) && \
	!defined(fun_DRIVER_CANCEL_2) && \
	!defined(fun_DRIVER_CANCEL_3) && \
	!defined(fun_DRIVER_CANCEL_4) && \
	!defined(fun_DRIVER_CANCEL_5) && \
	!defined(fun_DRIVER_CANCEL_6) && \
	!defined(fun_DRIVER_CANCEL_7) 
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_CheckIoDpcRoutines()
{
#if defined(fun_IO_DPC_ROUTINE_1) || \
	defined(fun_IO_DPC_ROUTINE_2) || \
	defined(fun_IO_DPC_ROUTINE_3) ||\
	defined(fun_IO_DPC_ROUTINE_4) || \
	defined(fun_IO_DPC_ROUTINE_5) || \
	defined(fun_IO_DPC_ROUTINE_6) 
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_IoCompletionRoutines()
{
#if defined(fun_IO_COMPLETION_ROUTINE_1) || \
	defined(fun_IO_COMPLETION_ROUTINE_2) || \
	defined(fun_IO_COMPLETION_ROUTINE_3) ||\
	defined(fun_IO_COMPLETION_ROUTINE_4) || \
	defined(fun_IO_COMPLETION_ROUTINE_5) || \
	defined(fun_IO_COMPLETION_ROUTINE_6) 
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_CheckWorkerRoutines()
{
#if defined(fun_WORKER_THREAD_ROUTINE_1) || \
	defined(fun_WORKER_THREAD_ROUTINE_2) || \
	defined(fun_WORKER_THREAD_ROUTINE_3) ||\
	defined(fun_WORKER_THREAD_ROUTINE_4) || \
	defined(fun_WORKER_THREAD_ROUTINE_5) || \
	defined(fun_WORKER_THREAD_ROUTINE_6) 
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_CheckAddDevice()
{
#if defined(fun_AddDevice)  
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_CheckIrpMjPnp()
{
#if defined(fun_IRP_MJ_PNP) 
	return TRUE;
#else
    return FALSE;
#endif
}



BOOLEAN
sdv_CheckIrpMjPower()
{
#if defined(fun_IRP_MJ_POWER) 
	return TRUE;
#else
    return FALSE;
#endif
}


BOOLEAN
sdv_CheckDriverUnload()
{
#if defined(fun_DriverUnload)
	return TRUE;
#else
    return FALSE;
#endif
}



NTSTATUS sdv_RunAddDevice(
    PDRIVER_OBJECT p1,
    PDEVICE_OBJECT p2
    )
{

    NTSTATUS status=STATUS_SUCCESS;
    sdv_stub_add_begin();
#ifdef fun_AddDevice
    status = fun_AddDevice(p1,p2);
#endif
    sdv_stub_add_end();
    return status;
}



/*NTSTATUS sdv_RunDriverentry(
    _DRIVER_OBJECT  *DriverObject,
    PUNICODE_STRING  RegistryPath
    )
{

   NTSTATUS status;
    sdv_stub_driver_entry_begin();
    status = fun_DriverEntry(&DriverObject, &RegistryPath);
    sdv_stub_driver_entry_begin();
    return status;
}*/



VOID 
sdv_SetIrpMinorFunctionNonBusDriver(
    PIRP pirp
    )  
{
    PIO_STACK_LOCATION r = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG x = SdvMakeChoice();
    
    switch (x) 
    {
        case 0:
        r->MinorFunction = IRP_MN_STOP_DEVICE;
        break;
        case 1:
        r->MinorFunction = IRP_MN_CANCEL_STOP_DEVICE;
        break;
#ifdef SDV_NO_MN_REMOVE_DEVICE 
        case 2:
        r->MinorFunction = IRP_MN_CANCEL_REMOVE_DEVICE;
        break;      
        case 3:
        r->MinorFunction = IRP_MN_SURPRISE_REMOVAL;
        break;           
#endif
        default:
        r->MinorFunction = IRP_MN_DEVICE_USAGE_NOTIFICATION;
        break;
    }

}

PIO_STACK_LOCATION
sdv_SetPowerRequestIrpMinorFunction(
    PIRP pirp
    )  

/*Routine Description:

    Sets the MN IRP fields to the possible values.  

Arguments:

    pirp - The IRP to set.

Notes:
     If IRP_MN_SET_POWER || IRP_MN_QUERY_POWER then this is a SystemPowerState.
     If IRP_MN_WAIT_WAKE then this is a DevicePowerState.
*/
{
    PIO_STACK_LOCATION r = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    LONG nd_minor = SdvMakeChoice();
    LONG nd_state_system_power1 = SdvMakeChoice();
    LONG nd_state_system_power2 = SdvMakeChoice();
    LONG nd_state_device_power1 = SdvMakeChoice();
       
    switch (nd_minor) 
    {
        case 0:
        switch (nd_state_system_power1)
        { 
          case 0:
          r->Parameters.Power.State.SystemState=PowerSystemWorking;
          break;
          case 1:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping1;
          break;
          case 2:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping2;
          break;
          case 3:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping3;
          break;
          case 4:
          r->Parameters.Power.State.SystemState=PowerSystemHibernate;
          break;
          case 5:
          r->Parameters.Power.State.SystemState=PowerSystemShutdown;
          break;
          default:
          r->Parameters.Power.State.SystemState=PowerSystemMaximum;
          break;
        }
        r->MinorFunction = IRP_MN_QUERY_POWER;
        r->Parameters.Power.Type = SystemPowerState;
        break;
        case 1:
        switch (nd_state_system_power2)
        { 
          case 0:
          r->Parameters.Power.State.SystemState=PowerSystemWorking;
          break;
          case 1:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping1;
          break;
          case 2:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping2;
          break;
          case 3:
          r->Parameters.Power.State.SystemState=PowerSystemSleeping3;
          break;
          case 4:
          r->Parameters.Power.State.SystemState=PowerSystemHibernate;
          break;
          case 5:
          r->Parameters.Power.State.SystemState=PowerSystemShutdown;
          break;
          default:
          r->Parameters.Power.State.SystemState=PowerSystemMaximum;
          break;
        }
        r->MinorFunction = IRP_MN_SET_POWER;
        r->Parameters.Power.Type = SystemPowerState;
        break; 
        default:
        switch (nd_state_device_power1)
        { 
          case 0:
          r->Parameters.Power.State.DeviceState=PowerDeviceD0;
          break;
          case 1:
          r->Parameters.Power.State.DeviceState=PowerDeviceD1;
          break;
          case 2:
          r->Parameters.Power.State.DeviceState=PowerDeviceD3;
          break;
          default:
          r->Parameters.Power.State.DeviceState=PowerDeviceMaximum;
          break;
        }
        r->MinorFunction = IRP_MN_WAIT_WAKE;
        r->Parameters.Power.Type =DevicePowerState;
        break;
    }
    return r;
}


VOID 
sdv_SetIrpMinorFunctionBusDriver(
    PIRP pirp
    )  
{
    PIO_STACK_LOCATION r = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(pirp);
    r->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    r->Parameters.QueryDeviceRelations.Type  = BusRelations;
    r->CompletionRoutine = NULL;
    SDV_MACRO_CLEAR_IRP_STACKLOCATIONS_COMPLETION();
}


VOID SdvAssume(int e)
/*
Routine Description:
    Acts like an ASSERT, but halts analysis if the assert fails.
*/
{
  if (!e) SdvExit();
}


VOID SdvAssumeSoft(int e)
/*
Routine Description:
   Acts like an SdvAssume.   
*/
{
  if (!e) SdvExit();
}

void assume ( int a )
{
}




/* 
    Disable for SdvExit: C4717: recursive on all control paths,
    function will cause runtime stack overflow.

    This is correctly flagged by the compiler, and would be a serious
    issue if the harness was to be executed rather than simulated.

    However in this case, this is per design in order to simulate
    non-progress:
*/
#pragma warning(disable:4717)

VOID SdvExit() 
/*

Routine Description:

    Acts like "exit()" within the context of model checking.

Notes:
    Since SdvExit contributes no extra reachable states, this serves as a
    dead-end of sorts to tools based on reachabilitity analysis.

*/
{ 
      SdvAssume( 0 );	
      SdvExit();

    /* Enable after SdvExit: C4717: */
    #pragma warning(default:4717)
}

/* harness-parts.c end */

/* flat-harness.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/*****************************************************************************

    flat-harness.c provides a flat harness for exercising a driver.

    The flat harness is useful for checking issues that are of a local
    nature, for example IRQL checking, simple resource allocation and
    synchronization primitives.

    The flat harness only calls the driver once.  For a more complete
    and realistic harness that calls the driver repeatedly see the PNP
    harness in pnp-harness.c

    The following variations of the flat harness are available:
        SDV_FLAT_DISPATCH_HARNESS
        SDV_FLAT_DISPATCH_STARTIO_HARNESS
        SDV_FLAT_SIMPLE_HARNESS
        SDV_FLAT_HARNESS

    The harnesses exercises the driver as follows:
        SDV_FLAT_DISPATCH_HARNESS =
            DoNothing ||
            sdv_RunDispatchFunction

        SDV_FLAT_DISPATCH_STARTIO_HARNESS =
            SDV_FLAT_DISPATCH_HARNESS ||
            sdv_RunStartIo

        SDV_FLAT_SIMPLE_HARNESS =
            SDV_FLAT_DISPATCH_STARTIO_HARNESS ||
            sdv_RunDPCs ||
            sdv_RunISRs ||
            sdv_RunWorkerThreads ||
            sdv_RunIoDpcs ||
            fun_DRIVER_CANCEL_* ||
            fun_IO_COMPLETION_ROUTINE_*||
            fun_IO_WORKITEM_ROUTINE_*||
            fun_IO_WORKITEM_ROUTINE_EX_* ||
            fun_PO_FX_COMPONENT_IDLE_STATE_CALLBACK ||
            fun_PO_FX_COMPONENT_ACTIVE_CONDITION_CALLBACK ||
            fun_PO_FX_COMPONENT_IDLE_CONDITION_CALLBACK ||
            fun_PO_FX_DEVICE_POWER_NOT_REQUIRED_CALLBACK ||
            fun_PO_FX_POWER_CONTROL_CALLBACK ||
            fun_PO_FX_COMPONENT_CRITICAL_TRANSITION_CALLBACK

            
            


        SDV_FLAT_HARNESS =
            SDV_FLAT_SIMPLE_HARNESS ||
            DriverEntry ||
            sdv_RunAddDevice ||
            sdv_RunStartDevice ||
            sdv_RunRemoveDevice ||
            sdv_RunUnload

*****************************************************************************/
      
 #if ( SDV_IS_FLAT_HARNESS() || SDV_IS_XFLAT_HARNESS_CANCEL() )

int sdv_is_flat_harness;


void sdv_main() 
{
    UNICODE_STRING u;
    NTSTATUS status;
    LONG choice;
    BOOLEAN sdv_dispatch_routines;
    BOOLEAN sdv_startio_routines;
    BOOLEAN sdv_dpc_routines;
    BOOLEAN sdv_isr_routines;
    BOOLEAN sdv_cancel_routines;
    BOOLEAN sdv_io_dpc_routines;
    BOOLEAN sdv_io_completion_routines;
    BOOLEAN sdv_io_worker_routines;
    BOOLEAN sdv_check_adddevice;
    BOOLEAN sdv_check_Irp_Mj_Pnp;
    BOOLEAN sdv_check_Irp_Mj_Power;
    BOOLEAN sdv_check_driver_unload;
    PIO_WORKITEM IoWorkItem;
    PIO_WORKITEM_ROUTINE WorkerRoutine;
    PIO_WORKITEM_ROUTINE_EX WorkerRoutineEx;
    WORK_QUEUE_TYPE QueueType;
	  PVOID  IoObject;
    PVOID Context;
    
#ifdef SDV_NO_DEBUGGER_ATTACHED_OR_ENABLED    
    KD_DEBUGGER_ENABLED=0;
    KD_DEBUGGER_NOT_PRESENT=1;
#endif


    /* Suppress C4101: Unreferenced local variable.
       Certain flawors of the OS Model does not reference u and status.
       Reference them explicitly to suppress warning: */
    u;
    status;
	  
	  #ifdef SDV_DEVICE_FLAGS
    sdv_p_devobj_fdo->Flags = DO_DEVICE_INITIALIZING;
    sdv_p_devobj_child_pdo->Flags = DO_DEVICE_INITIALIZING;
    #endif
	

    
    sdv_dispatch_routines=sdv_CheckDispatchRoutines();
    sdv_startio_routines=sdv_CheckStartIoRoutines();
    sdv_dpc_routines=sdv_CheckDpcRoutines();
    sdv_isr_routines=sdv_CheckIsrRoutines();
    sdv_cancel_routines=sdv_CheckCancelRoutines();
    
    sdv_io_dpc_routines=sdv_CheckIoDpcRoutines();
    sdv_io_completion_routines=sdv_IoCompletionRoutines();
    sdv_io_worker_routines=sdv_CheckWorkerRoutines();
    sdv_check_adddevice=sdv_CheckAddDevice();
    sdv_check_Irp_Mj_Pnp=sdv_CheckIrpMjPnp();
    sdv_check_Irp_Mj_Power=sdv_CheckIrpMjPower();
    sdv_check_driver_unload=sdv_CheckDriverUnload();


    choice = SdvMakeChoice();

    switch (choice) {

/*****************************************************************************
    For all the harness(es):
        SDV_FLAT_DISPATCH_HARNESS
        SDV_FLAT_DISPATCH_STARTIO_HARNESS
        SDV_FLAT_SIMPLE_HARNESS
        SDV_FLAT_HARNESS
    Exercise:
        sdv_RunDispatchFunction ||
*****************************************************************************/

    case 0: 
        if(sdv_dispatch_routines)
        {
          sdv_stub_driver_init();
          sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);
         }

        break;

/*****************************************************************************
    Additionally for the harness(es):
        SDV_FLAT_DISPATCH_STARTIO_HARNESS
        SDV_FLAT_SIMPLE_HARNESS
        SDV_FLAT_HARNESS
    Exercise:
        sdv_RunStartIo ||
*****************************************************************************/

#if (SDV_HARNESS!=SDV_FLAT_DISPATCH_HARNESS)

 
    case 1: 
        if(sdv_startio_routines)
        {
            sdv_stub_driver_init();
            sdv_RunStartIo(sdv_p_devobj_fdo, sdv_irp);
        }
        break;


/*****************************************************************************
    Additionally for the harness(es):
        SDV_FLAT_SIMPLE_HARNESS
        SDV_FLAT_HARNESS
    Exercise:
        sdv_RunDPC ||
        sdv_RunISR ||
        sdv_RunWorkerThreads ||
        sdv_RunIoDpcs
*****************************************************************************/

#if (SDV_HARNESS!=SDV_FLAT_DISPATCH_STARTIO_HARNESS)
#ifndef SDV_FLAT_HARNESS_NO_DPC
    case 2:
    #ifdef fun_KDEFERRED_ROUTINE_1
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_1(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    
    case 3: 
    #ifdef fun_KDEFERRED_ROUTINE_2
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_2(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    
    case 4: 
    #ifdef fun_KDEFERRED_ROUTINE_3
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_3(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    
    case 5: 
    #ifdef fun_KDEFERRED_ROUTINE_4
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_4(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    
    case 6: 
    #ifdef fun_KDEFERRED_ROUTINE_5
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_5(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 7: 
    #ifdef fun_KDEFERRED_ROUTINE_6
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_6(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 8: 
    #ifdef fun_KDEFERRED_ROUTINE_7
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_7(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 9: 
    #ifdef fun_KDEFERRED_ROUTINE_8
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_8(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 10: 
    #ifdef fun_KDEFERRED_ROUTINE_9
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_KDEFERRED_ROUTINE_9(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
#endif
#ifndef SDV_FLAT_HARNESS_NO_ISR
    case 11:
    #ifdef fun_KSERVICE_ROUTINE_1
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_1(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 12: 
    #ifdef fun_KSERVICE_ROUTINE_2
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_2(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 13: 
    #ifdef fun_KSERVICE_ROUTINE_3
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_3(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 14: 
    #ifdef fun_KSERVICE_ROUTINE_4
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_4(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 15: 
    #ifdef fun_KSERVICE_ROUTINE_5
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_5(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 16: 
    #ifdef fun_KSERVICE_ROUTINE_6
        SDV_IRQL_PUSH(SDV_DIRQL);
        fun_KSERVICE_ROUTINE_6(sdv_kinterrupt,sdv_pv1);
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 17:
    #ifdef fun_WORKER_THREAD_ROUTINE_1
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_1(sdv_pv2);
    #endif
        break;
    case 18:
    #ifdef fun_WORKER_THREAD_ROUTINE_2
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_2(sdv_pv2);
    #endif
        break;
    case 19:
    #ifdef fun_WORKER_THREAD_ROUTINE_3
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_3(sdv_pv2);
    #endif
        break;
    case 20:
    #ifdef fun_WORKER_THREAD_ROUTINE_4
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_4(sdv_pv2);
    #endif
        break;
    case 21:
    #ifdef fun_WORKER_THREAD_ROUTINE_5
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_5(sdv_pv2);
    #endif
        break;
    case 22:
    #ifdef fun_WORKER_THREAD_ROUTINE_6
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
        fun_WORKER_THREAD_ROUTINE_6(sdv_pv2);
    #endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_NO_KE_DPC
    case 23:
    #ifdef fun_IO_DPC_ROUTINE_1
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_1(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 24:
    #ifdef fun_IO_DPC_ROUTINE_2
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_2(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 25:
    #ifdef fun_IO_DPC_ROUTINE_3
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_3(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 26:
    #ifdef fun_IO_DPC_ROUTINE_4
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_4(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 27:
    #ifdef fun_IO_DPC_ROUTINE_5
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_5(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
    case 28:
    #ifdef fun_IO_DPC_ROUTINE_6
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_DPC_ROUTINE_6(sdv_kdpc,sdv_pDpcContext,sdv_pv2,sdv_pv3); 
        SDV_IRQL_POPTO(PASSIVE_LEVEL);
    #endif
        break;
#endif

#ifndef SDV_FLAT_HARNESS_CANCEL_ROUTINE
    case 29:
    #ifdef fun_DRIVER_CANCEL_1
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_1(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	  #endif
        break;
    case 30:
    #ifdef fun_DRIVER_CANCEL_2
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_2(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	#endif
	     break;
    case 31:
    #ifdef fun_DRIVER_CANCEL_3
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_3(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	  #endif
        break;
    case 32:
    #ifdef fun_DRIVER_CANCEL_4
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_4(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	  #endif
        break;
    case 33:
    #ifdef fun_DRIVER_CANCEL_5
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_5(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	  #endif
        break;
    case 34:
    #ifdef fun_DRIVER_CANCEL_6
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_6(sdv_p_devobj_fdo, sdv_irp);
		    SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	  #endif
        break;
    case 35:
    #ifdef fun_DRIVER_CANCEL_7
        SDV_MACRO_STUB_CANCEL_BEGIN(sdv_irp);
		    fun_DRIVER_CANCEL_7(sdv_p_devobj_fdo, sdv_irp);
		SDV_MACRO_STUB_CANCEL_END(sdv_irp);
	#endif
        break;
   #endif
   #ifndef SDV_HARNESS_COMPLETION_ROUTINE
    case 36:
    #ifdef fun_IO_COMPLETION_ROUTINE_1
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_1(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 37:
    #ifdef fun_IO_COMPLETION_ROUTINE_2
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_2(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 38:
    #ifdef fun_IO_COMPLETION_ROUTINE_3
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_3(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 39:
    #ifdef fun_IO_COMPLETION_ROUTINE_4
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_4(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 40:
    #ifdef fun_IO_COMPLETION_ROUTINE_5
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_5(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 41:
    #ifdef fun_IO_COMPLETION_ROUTINE_6
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_6(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 42:
    #ifdef fun_IO_COMPLETION_ROUTINE_7
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_7(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 43:
    #ifdef fun_IO_COMPLETION_ROUTINE_8
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_8(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
	    	SDV_IRQL_POP();
	  #endif
        break;
    case 44:
    #ifdef fun_IO_COMPLETION_ROUTINE_9
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_9(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 45:
    #ifdef fun_IO_COMPLETION_ROUTINE_10
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_10(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 46:
    #ifdef fun_IO_COMPLETION_ROUTINE_11
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_11(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
    case 47:
    #ifdef fun_IO_COMPLETION_ROUTINE_12
        SDV_IRQL_PUSH(DISPATCH_LEVEL);
        fun_IO_COMPLETION_ROUTINE_12(sdv_p_devobj_fdo,sdv_irp,sdv_pv2);  
		    SDV_IRQL_POP();
	  #endif
        break;
  
#endif

    case 48:
    #ifdef fun_REQUEST_POWER_COMPLETE_1
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_1(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;

    case 49:
    #ifdef fun_REQUEST_POWER_COMPLETE_2
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_2(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
    break;
    case 50:
    #ifdef fun_REQUEST_POWER_COMPLETE_3
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation=sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_3(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 51:
    #ifdef fun_REQUEST_POWER_COMPLETE_4
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_4(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 52:
    #ifdef fun_REQUEST_POWER_COMPLETE_5
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_5(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 53:
    #ifdef fun_REQUEST_POWER_COMPLETE_6
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_6(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 54:
	  #ifdef fun_REQUEST_POWER_COMPLETE_7
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_7(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 55:
    #ifdef fun_REQUEST_POWER_COMPLETE_8
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_8(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
	  #endif
        break;
    case 56:
    #ifdef fun_REQUEST_POWER_COMPLETE_9
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_9(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
    case 57:
    #ifdef fun_REQUEST_POWER_COMPLETE_10
        sdv_stub_power_completion_begin();
	      sdv_irp->Tail.Overlay.CurrentStackLocation =sdv_SetPowerRequestIrpMinorFunction(sdv_irp);
        fun_REQUEST_POWER_COMPLETE_10(sdv_p_devobj_fdo,sdv_irp->Tail.Overlay.CurrentStackLocation->MinorFunction,sdv_irp->Tail.Overlay.CurrentStackLocation->Parameters.Power.State,sdv_pv2,&sdv_irp->IoStatus);  
	      SDV_IRQL_POP();
    #endif
        break;
#ifndef SDV_FLAT_HARNESS_NO_KE_WORK_ITEMS 
    case 58:
    #ifdef fun_IO_WORKITEM_ROUTINE_1
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_1(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 59:
    #ifdef fun_IO_WORKITEM_ROUTINE_2
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_2(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 60:
    #ifdef fun_IO_WORKITEM_ROUTINE_3
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_3(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 61:
    #ifdef fun_IO_WORKITEM_ROUTINE_4
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_4(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 62:
    #ifdef fun_IO_WORKITEM_ROUTINE_5
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_5(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 63:
    #ifdef fun_IO_WORKITEM_ROUTINE_6
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_6(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 64:
    #ifdef fun_IO_WORKITEM_ROUTINE_7
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_7(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 65:
    #ifdef fun_IO_WORKITEM_ROUTINE_8
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_8(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 66:
    #ifdef fun_IO_WORKITEM_ROUTINE_9
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_9(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 67:
    #ifdef fun_IO_WORKITEM_ROUTINE_10
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_10(sdv_p_devobj_fdo,Context);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 68:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_1
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_1(IoObject,Context,IoWorkItem);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 69:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_2
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_2(IoObject,Context,IoWorkItem);  
      	SDV_IRQL_POP();
	  #endif
        break;
    case 70:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_3
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_3(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 71:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_4
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_4(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 72:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_5
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_5(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 73:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_6
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_6(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 74:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_7
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_7(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 75:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_8
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_8(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 76:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_9
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_9(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 77:
    #ifdef fun_IO_WORKITEM_ROUTINE_EX_10
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        fun_IO_WORKITEM_ROUTINE_EX_10(IoObject,Context,IoWorkItem);
      	SDV_IRQL_POP();
	  #endif
        break;
#endif
    case 78:
    #ifdef fun_PO_FX_COMPONENT_IDLE_STATE_CALLBACK
        sdv_stub_power_runtime_begin();
        fun_PO_FX_COMPONENT_IDLE_STATE_CALLBACK(Context,sdv_PoRuntime_Component,sdv_PoRuntime_State);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 79:
    #ifdef fun_PO_FX_COMPONENT_ACTIVE_CONDITION_CALLBACK
        sdv_stub_power_runtime_begin();
        fun_PO_FX_COMPONENT_ACTIVE_CONDITION_CALLBACK(Context,sdv_PoRuntime_Component,sdv_PoRuntime_State);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 80:
    #ifdef fun_PO_FX_COMPONENT_IDLE_CONDITION_CALLBACK
        sdv_stub_power_runtime_begin();
        fun_PO_FX_COMPONENT_IDLE_CONDITION_CALLBACK(Context,sdv_PoRuntime_Component);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 81:
    #ifdef fun_PO_FX_DEVICE_POWER_REQUIRED_CALLBACK
        sdv_stub_power_runtime_begin();
        fun_PO_FX_DEVICE_POWER_REQUIRED_CALLBACK(Context);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 82:
    #ifdef fun_PO_FX_POWER_CONTROL_CALLBACK
        sdv_stub_power_runtime_begin();
        status=fun_PO_FX_POWER_CONTROL_CALLBACK(sdv_PoRuntime_DeviceContext,
                                         sdv_PoRuntime_PowerControlCode,
                                         sdv_PoRuntime_InBuffer,
                                         sdv_PoRuntime_InBufferSize,
                                         sdv_PoRuntime_OutBuffer,
                                         sdv_PoRuntime_OutBufferSize,
                                         sdv_PoRuntime_BytesReturned);
      	SDV_IRQL_POP();
	  #endif
        break;    
    case 83:
    #ifdef fun_PO_FX_COMPONENT_CRITICAL_TRANSITION_CALLBACK
        sdv_stub_power_runtime_critical_begin();
        fun_PO_FX_COMPONENT_CRITICAL_TRANSITION_CALLBACK(Context,sdv_PoRuntime_Component,sdv_PoRuntime_Active);
      	SDV_IRQL_POP();
	  #endif
        break;
    case 84:
    #ifdef fun_PO_FX_DEVICE_POWER_NOT_REQUIRED_CALLBACK
        sdv_stub_power_runtime_begin();
        fun_PO_FX_DEVICE_POWER_NOT_REQUIRED_CALLBACK(Context);
      	SDV_IRQL_POP();
	  #endif
        break;

/*****************************************************************************
    Additionally for the harness(es):
        SDV_FLAT_HARNESS
    Exercise:
        DriverEntry ||
        sdv_RunAddDevice ||
        sdv_RunStartDevice ||
        sdv_RunRemoveDevice ||
        sdv_RunUnload
*****************************************************************************/

#if ( !SDV_IS_FLAT_SIMPLE_HARNESS() && SDV_HARNESS!=SDV_XFLAT_SIMPLE_HARNESS_CANCEL )

    case 85: 
        sdv_inside_init_entrypoint = TRUE;
        status = fun_DriverEntry(&sdv_driver_object, &u);
        sdv_inside_init_entrypoint = FALSE;
        break;
    #ifndef SDV_FLAT_HARNESS_MODIFIER_NO_ADDDEVICE
    #ifdef fun_AddDevice

    case 86: 
        status = sdv_RunAddDevice(&sdv_driver_object,sdv_p_devobj_pdo);
        break;
    #endif
    #endif


#ifdef fun_IRP_MJ_PNP
    case 87: 
        sdv_stub_driver_init();
        status = sdv_RunStartDevice(sdv_p_devobj_fdo, sdv_irp);
        break;
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_REMOVE_DEVICE
#ifdef fun_IRP_MJ_PNP
    case 88:
        status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
        break; 
#endif
#endif

#ifndef SDV_FLAT_HARNESS_MODIFIER_NO_UNLOAD
#ifdef fun_DriverUnload
    case 89:
        sdv_RunUnload(&sdv_driver_object);
        break;
#endif
#endif

#endif
#endif
#endif

    }
}

#endif
/* flat-harness.c end */

/* simple-harness.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/*****************************************************************************

    simple-harness.c provides a simple harness for exercising a driver.

    The simple harness is useful for checking issues cover entry points and IRP callbacks.

    The simple harness only calls the driver once.  For a more complete
    and realistic harness that calls the driver repeatedly see the PNP
    harness in pnp-harness.c

    This following variations of the flat harness are available:
        SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL
        

    This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL =
            DoNothing ||
            sdv_RunDispatchFunction && CompletionRoutines (No Cancelation)

    This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_NO_CANCEL =
            DoNothing ||
            sdv_RunDispatchFunction (No Cancelation)

    This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_NO_DEVICE_IRPS =
            DoNothing ||
            sdv_RunDispatchFunction (SYSTEM IRPS ONLY)

    This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_ONLY =
            DoNothing ||
            sdv_RunDispatchFunction && IoCompletion && (no ISRs, DPCs, WorkItems etc)

    This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_PNP_POWER_IRPS =
            DoNothing ||
            sdv_RunDispatchFunction && IoCompletion && (PNP && POWER)

	This harness exercises the driver as follows:
        SDV_FLAT_SIMPLE_HARNESS_WITH_WMI_ONLY =
            DoNothing ||
            sdv_RunDispatchFunction (WMI Request only)

*****************************************************************************/
      
 #if ( SDV_FLAT_SIMPLE_HARNESS())




void sdv_main() 
{



    UNICODE_STRING u;
    NTSTATUS status;
    LONG choice;
    PIO_WORKITEM IoWorkItem;
    PIO_WORKITEM_ROUTINE WorkerRoutine;
    PIO_WORKITEM_ROUTINE_EX WorkerRoutineEx;
    WORK_QUEUE_TYPE QueueType;
    PVOID  IoObject;
    PVOID Context;
    
	PVOID sdv_PoRuntime_Context;
    PVOID sdv_PoRuntime_InBuffer;
    PVOID sdv_PoRuntime_OutBuffer;
    BOOLEAN sdv_PoRuntime_Active;
    BOOLEAN sdv_PoRuntime_Suspended;
    BOOLEAN sdv_PoRuntime_PowerRequired;
    ULONG sdv_PoRuntime_Component;
    ULONG sdv_PoRuntime_State;
    PVOID sdv_PoRuntime_DeviceContext;
    LPCGUID sdv_PoRuntime_PowerControlCode;
    SIZE_T sdv_PoRuntime_InBufferSize;
    SIZE_T sdv_PoRuntime_OutBufferSize;
    PSIZE_T sdv_PoRuntime_BytesReturned;


#ifdef SDV_NO_DEBUGGER_ATTACHED_OR_ENABLED    
    KD_DEBUGGER_ENABLED=0;
    KD_DEBUGGER_NOT_PRESENT=1;
#endif


    /* Suppress C4101: Unreferenced local variable.
       Certain flawors of the OS Model does not reference u and status.
       Reference them explicitly to suppress warning: */
    u;
    status;
    #ifdef SDV_DEVICE_FLAGS
        sdv_p_devobj_fdo->Flags = DO_DEVICE_INITIALIZING;
        sdv_p_devobj_child_pdo->Flags = DO_DEVICE_INITIALIZING;
    #endif
    choice = SdvMakeChoice();
	switch (choice) 
	{
    case 0: 
        sdv_stub_driver_init();
        sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);
        break;
#ifdef fun_DriverEntry
    case 1: 
        sdv_inside_init_entrypoint = TRUE;
        status = fun_DriverEntry(&sdv_driver_object, &u);
        sdv_inside_init_entrypoint = FALSE;
        break;
#endif
#ifdef fun_AddDevice
    case 2: 
		status = sdv_RunAddDevice(&sdv_driver_object,sdv_p_devobj_pdo);
        break;
#endif
#ifdef fun_IRP_MJ_PNP
    case 3: 
        sdv_stub_driver_init();
        status = sdv_RunStartDevice(sdv_p_devobj_fdo, sdv_irp);
        break;
#endif
#ifdef fun_DriverUnload
    case 89:
        sdv_RunUnload(&sdv_driver_object);
        break;
#endif
    }
}

#endif
/* simple-harness.c end */

/* pnp-harness.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

/*****************************************************************************

    pnp-harness.c provides a PNP harness for exercising a driver.

    The PNP harness is useful for checking issues that inherently
    depends on proper initialization of the driver.  Examples are
    memory freeing, attachment to the device stack.

    The PNP harness calls the driver repeatedly.  For a simpler
    harness that only calls the driver once see the flat harness in
    flat-harness.c

    The following variations of the PNP harness are available:
        SDV_PNP_HARNESS_SMALL
        SDV_PNP_HARNESS_UNLOAD
        SDV_PNP_HARNESS

    The harnesses exercises the driver as follows:
        SDV_PNP_HARNESS_SMALL =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunDispatchFunction

        SDV_PNP_HARNESS_UNLOAD =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunRemoveDevice
            sdv_RunUnload

        SDV_PNP_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunDispatchFunction;
            sdv_RunStartDevice
            ( DoNothing ||
              sdv_RunStartIo );
            sdv_RunRemoveDevice;
            sdv_RunUnload
        
        SDV_PNP_POWER_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunQueryCapRequirements{For PDO only/Bus Drivers IRP_MJ_PNP with IRP_MN_QUERY_CAPABILITIES}
            sdv_RunResRequirements{IRP_MJ_PNP with IRP_MN_FILTER_RESOURCE_REQUIREMENTS}
            sdv_RunStartDevice {IRP_MJ_PNP + IRP_MN_START_DEVICE}
            sdv_RunQueryCapRequirements{FDO IRP_MJ_PNP with IRP_MN_QUERY_CAPABILITIES}
            sdv_RunQueryDeviceState {IRP_MJ_PNP + IRP_MN_PNP_DEVICE_STATE}
            sdv_RunDeviceRelationsBusRelations {IRP_MJ_PNP + IRP_MN_QUERY_DEVICE_RELATIONS (Bus Relations)}
            sdv_RunQueryPower {IRP_MJ_POWER + IRP_MN_QUERY_POWER (System Power, S0)}
            sdv_RunSetPower {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S0)}
            sdv_RunQueryPower {IRP_MJ_POWER + IRP_MN_QUERY_POWER (System Power, S3)}
            sdv_RunSetPower {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S3)}
            sdv_RunSirpCompletionRoutine {IRP_MJ_POWER + IRP_MN_QUERY_POWER (SIRP)} 
            PoRequestPowerirp may register a completion routine for DIRP this is now support in model see ntddk-po.c PoRequestPowerIrp.
            sdv_RunPowerCompletionRoutines for  { (DIRP)} if SDV_HARNESS_POWER_COMPLETION_ROUTINE is defined in rule header file
            
        SDV_POWER_UP_POWER_DOWN_HARNESS =
            sdv_RunQueryPower {IRP_MJ_POWER + IRP_MN_QUERY_POWER (System Power, S0)}
            sdv_RunSetPower {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S0)}
            sdv_RunQueryPower {IRP_MJ_POWER + IRP_MN_QUERY_POWER (System Power, S3)}
            sdv_RunSetPower {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S3)}
            sdv_RunSirpCompletionRoutine {IRP_MJ_POWER + IRP_MN_QUERY_POWER (SIRP)} 
            PoRequestPowerirp may register a completion routine for DIRP this is now support in model see ntddk-po.c PoRequestPowerIrp.
            sdv_RunPowerCompletionRoutines for  { (DIRP)} if SDV_HARNESS_POWER_COMPLETION_ROUTINE is defined in rule header file
            
       SDV_PNP_REMOVE_DEVICE_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunSurpriseRemoveDevice {IRP_MJ_PNP + IRP_MN_SURPRISE_REMOVAL}
            sdv_RunQueryRemoveDevice {IRP_MJ_PNP + IRP_MN_QUERY_REMOVE_DEVICE}
            sdv_RunRemoveDevice {IRP_MJ_PNP + IRP_MN_REMOVE_DEVICE}
            
            
       SDV_PNP_BASIC_REMOVE_DEVICE_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunRemoveDevice {IRP_MJ_PNP + IRP_MN_REMOVE_DEVICE}
                   
       SDV_POWER_DOWN_PNP_HARNESS =
            sdv_RunQueryPowerDown {IRP_MJ_POWER + IRP_MN_QUERY_POWER (System Power, S3)}
            sdv_RunSetPowerDown {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S3)}
              sdv_RunSirpCompletionRoutine {IRP_MJ_POWER + IRP_MN_QUERY_POWER (SIRP)} 
              PoRequestPowerirp may register a completion routine for DIRP this is now support in model see ntddk-po.c PoRequestPowerIrp.
              sdv_RunPowerCompletionRoutines for  { (DIRP)} if SDV_HARNESS_POWER_COMPLETION_ROUTINE is defined in rule header file
            sdv_RunSurpriseRemoveDevice {IRP_MJ_PNP + IRP_MN_SURPRISE_REMOVAL}
            sdv_RunQueryRemoveDevice {IRP_MJ_PNP + IRP_MN_QUERY_REMOVE_DEVICE}
            sdv_RunRemoveDevice {IRP_MJ_PNP + IRP_MN_REMOVE_DEVICE}
            
       SDV_FLAT_QUERY_DEVICE_RELATIONS_HARNESS =
            sdv_RunQueryDeviceRelations {IRP_MJ_PNP + IRP_MN_QUERY_DEVICE_RELATIONS (Bus Relations)} for FDO of bus
       
       
       SDV_SMALL_START_SEQUENCE_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunStartDevice {IRP_MJ_PNP + IRP_MN_START_DEVICE}
            sdv_RunSetPowerUp {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S0)}
            
            
       SDV_SMALL_POWERDOWN_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunSetPowerDown {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, SX)}

       SDV_SMALL_SMALL_POWERUP_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunSetPowerUp {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S0)}
            
      SDV_SMALL_POWERUP_POWERDOWN_HARNESS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunSetPowerUp {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, S0)}
            sdv_RunSetPowerDown {IRP_MJ_POWER + IRP_MN_SET_POWER (System Power, SX)}
            
            
      SDV_PNP_QUERY_REMOVE_DEVICE_HARNESS = 
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunQueryRemoveDevice {IRP_MJ_PNP + IRP_MN_QUERY_REMOVE_DEVICE}
            
      SDV_PNP_HARNESS_SMALL_WITH_LINKED_CALLBACKS =
            DriverEntry;
            sdv_RunAddDevice;
            sdv_RunDispatchFunction
            and Linked callbacks
            
*****************************************************************************/

#define SDV_IS_PNP_HARNESS() \
    ( \
        SDV_HARNESS==SDV_POWER_UP_POWER_DOWN_HARNESS || \
        SDV_HARNESS==SDV_PNP_POWER_HARNESS || \
        SDV_HARNESS==SDV_PNP_HARNESS || \
        SDV_HARNESS==SDV_PNP_HARNESS_UNLOAD || \
        SDV_HARNESS==SDV_PNP_HARNESS_SMALL || \
        SDV_HARNESS==SDV_PNP_REMOVE_DEVICE_HARNESS || \
        SDV_HARNESS==SDV_SMALL_START_SEQUENCE_HARNESS || \
        SDV_HARNESS==SDV_PNP_BASIC_REMOVE_DEVICE_HARNESS || \
        SDV_HARNESS==SDV_SMALL_POWERDOWN_HARNESS || \
        SDV_HARNESS==SDV_POWER_DOWN_PNP_HARNESS || \
        SDV_HARNESS==SDV_SMALL_SMALL_POWERUP_HARNESS || \
        SDV_HARNESS==SDV_FLAT_QUERY_DEVICE_RELATIONS_HARNESS || \
        SDV_HARNESS==SDV_SMALL_POWERUP_POWERDOWN_HARNESS || \
        SDV_HARNESS==SDV_PNP_QUERY_REMOVE_DEVICE_HARNESS || \
        SDV_HARNESS==SDV_PNP_HARNESS_SMALL_WITH_LINKED_CALLBACKS \
    )

#if SDV_IS_PNP_HARNESS()

void sdv_main()
{
    UNICODE_STRING u;
    NTSTATUS status;
    LONG choice;
    
    sdv_p_devobj_fdo->Flags = DO_DEVICE_INITIALIZING;
    sdv_p_devobj_child_pdo->Flags = DO_DEVICE_INITIALIZING;
    


    
#if (SDV_HARNESS!=SDV_POWER_UP_POWER_DOWN_HARNESS) 

    choice = SdvMakeChoice();

/*****************************************************************************
    For all the harness(es):
        SDV_PNP_HARNESS_UNLOAD
        SDV_PNP_HARNESS_SMALL
        SDV_PNP_HARNESS
    Exercise:
        DriverEntry;
        sdv_RunAddDevice;
*****************************************************************************/
#ifndef No_DriverEntry
    sdv_inside_init_entrypoint = TRUE;
    SDV_IRQL_PUSH(PASSIVE_LEVEL);
    status = fun_DriverEntry(&sdv_driver_object, &u);
    sdv_inside_init_entrypoint = FALSE;
    SdvAssume (NT_SUCCESS(status));
#endif
#ifdef fun_AddDevice
#ifndef No_AddDevice
    SDV_IRQL_PUSH(PASSIVE_LEVEL);
    status = sdv_RunAddDevice(&sdv_driver_object,sdv_p_devobj_pdo);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif

    sdv_stub_driver_init();

/*****************************************************************************
    Additionally For the harness(es):
        SDV_PNP_HARNESS_SMALL
    Exercise:
        sdv_RunDispatchFunction;
*****************************************************************************/
#ifndef SDV_PNP_HARNESS_ADDDEVICE_ONLY
#if (SDV_HARNESS==SDV_PNP_HARNESS_SMALL)

    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

#endif



#if (SDV_HARNESS==SDV_PNP_HARNESS_SMALL_WITH_LINKED_CALLBACKS)

    #define SDV_FLAT_HARNESS_MODIFIER_NO_UNLOAD
    #define SDV_HARNESS_COMPLETION_ROUTINE
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_RUN_KE_DPC_ROUTINES
    #define SDV_RUN_KE_ISR_ROUTINES  
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES  
  

    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);

#endif




/*****************************************************************************
    Additionally For the harness(es):
        SDV_PNP_HARNESS_UNLOAD
    Exercise:
        ( sdv_RunRemoveDevice ||
          sdv_RunUnload)
*****************************************************************************/

#if (SDV_HARNESS==SDV_PNP_HARNESS_UNLOAD)

#ifdef fun_IRP_MJ_PNP
        status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
#endif

#ifdef fun_DriverUnload
        SDV_IRQL_PUSH(PASSIVE_LEVEL);
        sdv_RunUnload(&sdv_driver_object);
#endif

#endif

/*****************************************************************************
    Additionally For the harness(es):
        SDV_PNP_HARNESS
    Exercise:
        sdv_RunStartDevice
		    sdv_RunDispatchFunction
		    ( sdv_RunStartIo ||
          sdv_RunDPC ||
          sdv_RunISR );
        sdv_RunRemoveDevice;
        sdv_RunUnload
*****************************************************************************/

#if (SDV_HARNESS==SDV_PNP_HARNESS)

#ifdef fun_IRP_MJ_PNP
    status = sdv_RunStartDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif

#ifndef SDV_NO_DISPATCH_ROUTINE

    sdv_RunDispatchFunction(sdv_p_devobj_fdo, sdv_irp);
#endif
     
    
#ifdef fun_DriverStartIo
    sdv_RunStartIo(sdv_p_devobj_fdo, sdv_irp);
#endif

#ifdef fun_IRP_MJ_PNP
    status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
#endif

#ifdef fun_DriverUnload
    if (sdv_io_create_device_called<2) {
        sdv_driver_object.DeviceObject = NULL;
    }
    sdv_RunUnload(&sdv_driver_object);
#endif

#endif
#endif


#if (SDV_HARNESS==SDV_PNP_POWER_HARNESS)

#ifdef fun_IRP_MJ_PNP
#ifdef IS_WDM_BUS_DRIVER
#ifndef SDV_IRP_MN_REMOVE_DEVICE_ONLY
#ifndef SDV_IRP_MN_SUPRISE_REMOVE_ONLY
#ifndef SDV_IRP_MN_QUERY_REMOVE_DEVICE_ONLY
    status = sdv_RunQueryCapRequirements(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif
#endif
#endif
#ifndef SDV_IRP_MN_REMOVE_DEVICE_ONLY
#ifndef SDV_IRP_MN_SUPRISE_REMOVE_ONLY
#ifndef SDV_IRP_MN_QUERY_REMOVE_DEVICE_ONLY
    status = sdv_RunResRequirements(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunStartDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif
#endif
#ifdef SDV_IRP_MN_SUPRISE_REMOVE
#ifndef SDV_IRP_MN_REMOVE_DEVICE_ONLY
#ifndef SDV_IRP_MN_QUERY_REMOVE_DEVICE_ONLY
    status = sdv_RunSurpriseRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif
#endif

#ifndef NO_SDV_IRP_MN_QUERY_REMOVE_DEVICE_ONLY
#ifdef SDV_IRP_MN_REMOVE_DEVICE
    status = sdv_RunQueryRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif
    status = sdv_RunQueryCapRequirements(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunQueryDeviceState(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#ifdef IS_WDM_BUS_DRIVER
      status = sdv_RunQueryDeviceRelations(sdv_p_devobj_fdo, sdv_irp);
      SdvAssume (NT_SUCCESS(status));
#endif

#endif
#ifdef fun_IRP_MJ_POWER
    #define SDV_HARNESS_COMPLETION_ROUTINE
    status = sdv_RunQueryPowerUp(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunSetPowerUp(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunQueryPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunSetPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
#endif
#endif

#if (SDV_HARNESS==SDV_POWER_DOWN_PNP_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES
    status = sdv_RunQueryPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunSetPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
    status = sdv_RunSurpriseRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (status==STATUS_SUCCESS);
    status = sdv_RunQueryRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (status==STATUS_SUCCESS);
    status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (status==STATUS_SUCCESS);
#endif
#if (SDV_HARNESS==SDV_PNP_REMOVE_DEVICE_HARNESS)
    status = sdv_RunSurpriseRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
#endif

#if (SDV_HARNESS==SDV_PNP_BASIC_REMOVE_DEVICE_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES
    status = sdv_RunRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (status==STATUS_SUCCESS);
#endif



#if (SDV_HARNESS==SDV_SMALL_START_SEQUENCE_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES
#ifdef fun_IRP_MJ_PNP
    status = sdv_RunStartDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif
    status = sdv_RunSetPowerUp(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif


#if (SDV_HARNESS==SDV_SMALL_POWERDOWN_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES

#ifdef fun_IRP_MJ_PNP
#endif
    status = sdv_RunSetPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif




#if (SDV_HARNESS==SDV_SMALL_POWERUP_POWERDOWN_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES
    status = sdv_RunSetPowerUp(sdv_p_devobj_fdo, sdv_irp);
    status = sdv_RunSetPowerDown(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif




#if (SDV_HARNESS==SDV_SMALL_SMALL_POWERUP_HARNESS)
    #define SDV_HARNESS_COMPLETION_ROUTINE 
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_RUN_KSYNC_ROUTINES
    status = sdv_RunSetPowerUp(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (NT_SUCCESS(status));
#endif

#if (SDV_HARNESS==SDV_FLAT_QUERY_DEVICE_RELATIONS_HARNESS)
#ifdef IS_WDM_BUS_DRIVER
      status = sdv_RunQueryDeviceRelations(sdv_p_devobj_fdo, sdv_irp);
      SdvAssume (NT_SUCCESS(status));
#endif
#endif


#if (SDV_HARNESS==SDV_PNP_QUERY_REMOVE_DEVICE_HARNESS)
    status = sdv_RunQueryRemoveDevice(sdv_p_devobj_fdo, sdv_irp);
    SdvAssume (status==STATUS_SUCCESS);
#endif



}

#endif
/* pnp-harness.c end */

/* sdv_stubs.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/


void __cdecl main() 
{
    SDV_MAIN_INIT(); 
    sdv_main();
}


void sdv_do_paged_code_check() { }


void sdv_do_assert_check(int exp) 
{
    if(!exp) SdvExit();
}



void sdv_stub_driver_init() { }

void sdv_stub_io_completion_begin()
{ 
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}

void sdv_stub_io_completion_end()
{ 
    SDV_IRQL_POP();
}
void sdv_stub_PowerQuery() { }

void sdv_stub_PowerUpOrDown() { }

void sdv_stub_startio_begin()
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}

void sdv_stub_startio_end()
{ 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}

void sdv_stub_driver_control_begin()
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}

void sdv_stub_driver_control_end()
{ 
    SDV_IRQL_POP();
}
void sdv_stub_dispatch_begin()
{
    SDV_IRQL_PUSH(PASSIVE_LEVEL);
}

void sdv_stub_dispatch_end(NTSTATUS s, PIRP pirp)
{
}

void sdv_stub_add_begin() 
{ 
    sdv_inside_init_entrypoint = TRUE;
}
void sdv_stub_add_end() 
{ 
    sdv_inside_init_entrypoint = FALSE;
}


void sdv_stub_ioctl_begin() 
{
    
}

void sdv_stub_ioctl_end() 
{ 
    
}

void sdv_stub_pnp_query_begin() 
{
    
}

void sdv_stub_pnp_query_end() 
{ 
    
}


/*void sdv_stub_driver_entry_begin() 
{ 
    sdv_inside_init_entrypoint = TRUE;
}
void sdv_stub_driver_entry_end() 
{ 
    sdv_inside_init_entrypoint = FALSE;
}*/



void sdv_stub_unload_begin() 
{ 
    SDV_IRQL_POPTO(PASSIVE_LEVEL);
}

void sdv_stub_unload_end() { }


void sdv_stub_MoreProcessingRequired(PIRP pirp) { }
void sdv_stub_WmiIrpProcessed(PIRP pirp) { }
void sdv_stub_WmiIrpNotCompleted(PIRP pirp) { }
void sdv_stub_WmiIrpForward(PIRP pirp) { }

void sdv_stub_power_completion_begin()
{
    long irql = SdvMakeChoice();
    switch(irql)
    {
    case 0:
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      break;
    default:
      SDV_IRQL_PUSH(DISPATCH_LEVEL);
      break;
    }
}

void sdv_stub_power_runtime_begin()
{
    long irql = SdvMakeChoice();
    switch(irql)
    {
    case 0:
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      break;
    default:
      SDV_IRQL_PUSH(DISPATCH_LEVEL);
      break;
    }
}

void sdv_stub_power_runtime_critical_begin()
{
    long irql = SdvMakeChoice();
    switch(irql)
    {
    case 0:
      SDV_IRQL_PUSH(PASSIVE_LEVEL);
      break;
    case 1:
      SDV_IRQL_PUSH(DISPATCH_LEVEL);
      break;
    default:
      SDV_IRQL_PUSH(SDV_DIRQL);
      break;
    }
}
/* sdv_stubs.c end */

/* sdv_layer_functions.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/


int OneInINT(int min, int max) 
{
   int sdv_ =  Any(); 
   if( min <= sdv_ && sdv_ <= max)
   {
       return sdv_ ;
   }
}

/*int OneInINT(int min, int max) 
{
   int sdv_ =  Any(); 
   Constrain( min <= sdv_ && sdv_ <= max); 
   return sdv_ ;
}*/

int OneOfTwoINT(int x, int y) 
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    default:
      return y;
      break;
    }
}


/*int OneOfTwoINT(int x, int y) 
{
  int sdv_ = Any();
  Constrain( sdv_ == x || sdv_ == y );
  return sdv_ ;
}*/
 
/*int OneOfThreeINT(int x, int y, int z) 
{
  int sdv_ = Any();
  Constrain( sdv_ == x || sdv_ == y  || sdv_ == z);
  return sdv_ ;
}*/


int OneOfThreeINT(int x, int y, int z) 
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    default:
      return z;
      break;
    }
}


/*unsigned int OneOfTwoUINT(unsigned int x, unsigned int y) 
{
  unsigned int sdv_ = Any();
  Constrain( sdv_ == x || sdv_ == y );
  return sdv_ ;
}*/


unsigned int OneOfTwoUINT(unsigned int x, unsigned int y) 
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    default:
      return y;
      break;
    }
}

 
/*unsigned int OneOfThreeUINT(unsigned int x, unsigned int y, unsigned int z) 
{
  unsigned int sdv_ = (unsigned int)Any();
  Constrain( sdv_ == x || sdv_ == y  || sdv_ == z);
  return sdv_ ;
}*/


unsigned int OneOfThreeUINT(unsigned int x, unsigned int y, unsigned int z) 
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    default:
      return z;
      break;
    }
}

BOOLEAN OneOfTwoBOOLEAN()
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return TRUE;
      break;
    default:
      return FALSE;
      break;
    }
}


USHORT Any_USHORT() 
{
  USHORT sdv_ = (USHORT)Any();
  return sdv_;
}

NTSTATUS OneOfTwoNTSTATUS(NTSTATUS x, NTSTATUS y)
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    default:
      return y;
      break;
    }
}

NTSTATUS OneOfThreeNTSTATUS(NTSTATUS x, NTSTATUS y, NTSTATUS z)
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    default:
      return z;
      break;
    }
}

NTSTATUS OneOfFourNTSTATUS(NTSTATUS w,NTSTATUS x, NTSTATUS y, NTSTATUS z)
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return w;
      break;
    case 1:
      return x;
      break;
    case 2:
      return y;
      break;
    default:
      return z;
      break;
    }
}

NTSTATUS OneOfFiveNTSTATUS(NTSTATUS v,NTSTATUS w,NTSTATUS x, NTSTATUS y, NTSTATUS z)
{
  int sdv_ = Any_Local();
  switch(sdv_)
  {
    case 0:
      return v;
      break;
    case 1:
      return w;
      break;
    case 2:
      return x;
      break;
    case 3:
      return y;
      break;
    default:
      return z;
      break;
    }
}


ULONG Any_ULONG()
{
  ULONG sdv_ = (ULONG)Any();
  return sdv_;
}

char * Any_Memory()
{
   int choice = Any_Local();
    switch (choice) 
    {
        case 0: return NULL;break;
        default: return malloc(1);break;
    }
}

/* sdv_layer_functions.c end */

/* ntddk-cm.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/



NTKERNELAPI
NTSTATUS
CmUnRegisterCallback(__in LARGE_INTEGER  Cookie)
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }

}


NTKERNELAPI
NTSTATUS
CmRegisterCallbackEx(__in        PEX_CALLBACK_FUNCTION   Function,
                        __in        PCUNICODE_STRING        Altitude,
                        __in        PVOID                   Driver,
                        __in_opt    PVOID                 	Context,
                        __out       PLARGE_INTEGER    	    Cookie,
                        __reserved  PVOID			        Reserved
                    )
{

int choice = SdvMakeChoice();

if(Driver==NULL)
{
    return STATUS_INVALID_PARAMETER_3;
}

if(Reserved!=NULL)
{
    return STATUS_INVALID_PARAMETER_6;
}
switch (choice) 
{
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_FLT_INSTANCE_ALTITUDE_COLLISION;break;
}

}
 
NTKERNELAPI
NTSTATUS
CmRegisterCallback(__in     PEX_CALLBACK_FUNCTION Function,
                   __in_opt PVOID                 Context,
                   __out    PLARGE_INTEGER    Cookie
                    )
{
int choice = SdvMakeChoice();
switch (choice) 
{
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_FLT_INSTANCE_ALTITUDE_COLLISION;break;
}
}
/* ntddk-cm.c end */

/* ntddk-ex.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/


VOID 
sdv_ExQueueWorkItem(
  PWORK_QUEUE_ITEM WorkItem,
  WORK_QUEUE_TYPE QueueType
)
{
#ifdef SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
      sdv_RunExQueueWorkItems(WorkItem->WorkerRoutine,WorkItem->Parameter);
#endif
}

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex(
    IN PFAST_MUTEX FastMutex
    ) 
{  
    SDV_IRQL_PUSH(APC_LEVEL);
}

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe(
    IN PFAST_MUTEX  FastMutex
    ) 
{
}

VOID ExReleaseResourceForThreadLite(
  __inout  PERESOURCE Resource,
  __in     ERESOURCE_THREAD ResourceThreadId
)
{
}


NTKERNELAPI
BOOLEAN
ExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    ) 
{ 
    if (Wait) { return TRUE; }
    else {
        LONG choice = SdvMakeChoice();
        switch(choice) 
        {
            case 0:
                return FALSE;
                break;
            default:
                return TRUE;
                break;
        }
        
    };
}

NTKERNELAPI
BOOLEAN
ExAcquireResourceSharedLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    ) 
{
    if (Wait) { return TRUE; }
    else {
        LONG choice = SdvMakeChoice();
        switch(choice) 
        {
            case 0:
                return FALSE;
                break;
            default:
                return TRUE;
                break;
        }
    };
}

NTKERNELAPI
BOOLEAN
ExAcquireSharedStarveExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    ) 
{ 
    if (Wait) { return TRUE; }
    else {
        LONG choice = SdvMakeChoice();
        switch(choice) 
        {
            case 0:
                return FALSE;
                break;
            default:
                return TRUE;
                break;
        }
    };
}

NTKERNELAPI
BOOLEAN
ExAcquireSharedWaitForExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    ) 
{ 
    if (Wait) { return TRUE; }
    else {
        LONG choice = SdvMakeChoice();
        switch(choice) 
        {
            case 0:
                return FALSE;
                break;
            default:
                return TRUE;
                break;
        }
    };
}

NTKERNELAPI
PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    ) 
{ 
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: return (PVOID)malloc(NumberOfBytes);break;
      default: return NULL;break;
  }
    
}

NTKERNELAPI
PVOID
sdv_ExAllocatePoolWithQuota(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes
    )
{
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: return (PVOID)malloc(NumberOfBytes);break;
      default: return NULL;break;
  }
}

NTKERNELAPI
PVOID
sdv_ExAllocatePoolWithQuotaTag(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes,
    ULONG Tag
    )
{
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: return (PVOID)malloc(NumberOfBytes);break;
      default: return NULL;break;
  }
}

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    ) 
{
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: return (PVOID)malloc(NumberOfBytes);break;
      default: return NULL;break;
  }
}

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    )
{
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: return (PVOID)malloc(NumberOfBytes);break;

      default: return NULL;break;
  }
}

NTKERNELAPI
NTSTATUS
ExCreateCallback(
    OUT PCALLBACK_OBJECT *CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    ) 
{ 
    int x = SdvMakeChoice();
    switch (x) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}


NTKERNELAPI
VOID sdv_ExFreePool(PVOID P)
{
}


NTKERNELAPI
NTSTATUS
ExDeleteResourceLite(
    IN PERESOURCE Resource
    ) 
{ 
    int x = SdvMakeChoice();
    if (x) 
    {
        return STATUS_SUCCESS;
    } 
    else 
    {
        return STATUS_UNSUCCESSFUL;
    }
}

VOID
sdv_ExInitializeFastMutex(
    IN PFAST_MUTEX FastMutex
    )
{
}

NTKERNELAPI
NTSTATUS
ExInitializeResourceLite(
    IN PERESOURCE Resource
    ) 
{ 
    int x = SdvMakeChoice();
    if (x) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    } 
}

NTKERNELAPI
NTSTATUS
sdv_ExInitializeResource(
    IN PERESOURCE Resource
    ) 
{ 
    int x = SdvMakeChoice();
    if (x) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    }
}

NTKERNELAPI
PLIST_ENTRY
FASTCALL
sdv_ExInterlockedInsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    )
{
  PLIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: 
            return p = (PLIST_ENTRY)malloc(1);
        default:
            return p = NULL;
  }

}

PLIST_ENTRY sdv_ExInterlockedRemoveHeadList(
  __inout  PLIST_ENTRY ListHead,
  __inout  PKSPIN_LOCK Lock
)
{
  PLIST_ENTRY p;
  int x = SdvMakeChoice();
  #ifdef SDV_RULE_NULLCHECK
    sdv_rule_NullCheck = TRUE;
  #else
    sdv_rule_NullCheck = FALSE;
  #endif 
  switch (x) 
  {
        case 0: 
            return p = (PLIST_ENTRY)malloc(1);
        default:
            SdvAssume(!sdv_rule_NullCheck); 
            return p = NULL;
  }

}



NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
sdv_ExInterlockedPushEntryList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    )
{
  PSINGLE_LIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PSINGLE_LIST_ENTRY)malloc(1);break;
        default:return  p=NULL;break;
  }
}

PSINGLE_LIST_ENTRY sdv_ExInterlockedPopEntryList(
  __inout  PSINGLE_LIST_ENTRY ListHead,
  __inout  PKSPIN_LOCK Lock
)
{
  PSINGLE_LIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PSINGLE_LIST_ENTRY)malloc(1);break;
        default:return  p=NULL;break;
  }
}



PLIST_ENTRY
FASTCALL
ExfInterlockedInsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    ) 
{
  PLIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
            case 0: return p=(PLIST_ENTRY)malloc(sizeof(LIST_ENTRY));break;
        default:return  p=NULL;break;
  }
}

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExfInterlockedInsertTailList(
    IN PLIST_ENTRY ListTail,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    ) 
{
  PLIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
    case 0: return p=(PLIST_ENTRY)malloc(sizeof(LIST_ENTRY));break;
  default:return  p=NULL;break;
  }
}


PLIST_ENTRY sdv_ExInterlockedInsertTailList(
  __inout  PLIST_ENTRY ListHead,
  __inout  PLIST_ENTRY ListEntry,
  __inout  PKSPIN_LOCK Lock
)
{
  PLIST_ENTRY p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PLIST_ENTRY)malloc(1);break;
        default:return  p=NULL;break;
  }
}


NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExfInterlockedPushEntryList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    )
{
  PSINGLE_LIST_ENTRY p;
  PSINGLE_LIST_ENTRY ple;
  int x = SdvMakeChoice();
  switch (x) 
  {
  case 0: ple = (PSINGLE_LIST_ENTRY)malloc(sizeof(SINGLE_LIST_ENTRY));return ple; break;
  default:return  p=NULL;break;
  }
}

NTKERNELAPI
DECLSPEC_NORETURN
VOID
ExRaiseAccessViolation (
    VOID
    )
{
    /* As SDV does not support SEH: Stop verification: */
    /* This is unsound but is useful for suppressing false defects */ 
    SdvExit();
}

NTKERNELAPI
DECLSPEC_NORETURN
VOID
ExRaiseDatatypeMisalignment (
    VOID
    )
{
    /* As SDV does not support SEH: Stop verification: */
    /* This is unsound but is useful for suppressing false defects */ 
    SdvExit();
}

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
ExRaiseStatus (
    __in NTSTATUS Status
    )
{
    /* As SDV does not support SEH: Stop verification: */
    /* This is unsound but is useful for suppressing false defects */ 
    SdvExit();
}

NTKERNELAPI
NTSTATUS
ExReinitializeResourceLite(
    IN PERESOURCE Resource
    ) 
{ 
    return STATUS_SUCCESS; 
}

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex(
    IN PFAST_MUTEX FastMutex
    ) 
{  
    SDV_IRQL_POP();
}

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe(
    IN PFAST_MUTEX  FastMutex
    ) 
{
}
    

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceLite(
    IN PERESOURCE Resource    
    ) 
{
}

NTHALAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex(
    IN PFAST_MUTEX FastMutex
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}


#ifdef SDV_Include_NTDDK
NTKERNELAPI
NTSTATUS
ExUuidCreate(
    UUID *Uuid
    ) 
{ 
    int x = SdvMakeChoice();
    switch (x) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_RETRY;break;
    }
}
#endif 

PVOID
sdv_ExAllocateFromNPagedLookasideList(
    __inout PNPAGED_LOOKASIDE_LIST Lookaside
    )
{
  int x = SdvMakeChoice();
  switch (x) 
  {
      case 0: return malloc(1);break;
      default: return NULL;break;
  }
}

BOOLEAN
FASTCALL
ExAcquireRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}

BOOLEAN
FASTCALL
ExAcquireRundownProtectionCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
     )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}

BOOLEAN
FASTCALL
ExAcquireRundownProtectionCacheAwareEx (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware,
     __in ULONG Count
     )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}


BOOLEAN
FASTCALL
ExAcquireRundownProtectionEx
(
     __inout PEX_RUNDOWN_REF RunRef,
     __in ULONG Count
     )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}


VOID
FASTCALL
ExReleaseRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     )
{
    
}


VOID
FASTCALL
ExReleaseRundownProtectionEx (
     __inout PEX_RUNDOWN_REF RunRef,
     __in ULONG Count
     )
{
    
}

VOID ExConvertExclusiveToSharedLite(
  __inout  PERESOURCE Resource
)
{
    
}

NTKERNELAPI
PVOID
ExEnterCriticalRegionAndAcquireResourceExclusive (
    __inout PERESOURCE Resource
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return malloc(1);
            break;
    }
}

PVOID
ExEnterCriticalRegionAndAcquireResourceShared (
    __inout PERESOURCE Resource
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return malloc(1);
            break;
    }

}

PVOID
ExEnterCriticalRegionAndAcquireSharedWaitForExclusive (
    __inout PERESOURCE Resource
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return malloc(1);
            break;
    }

}

ULONG ExGetExclusiveWaiterCount(
  __in  PERESOURCE Resource
)
{
    return (ULONG)SdvKeepChoice();
}

ULONG ExGetSharedWaiterCount(
  __in  PERESOURCE Resource
)
{
   return (ULONG)SdvKeepChoice();
}

BOOLEAN ExIsResourceAcquiredExclusiveLite(
  __in  PERESOURCE Resource
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN sdv_ExIsResourceAcquiredExclusive(
  __in  PERESOURCE Resource
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}


ULONG
ExIsResourceAcquiredSharedLite (
    __in PERESOURCE Resource
    )
{
    return (ULONG)SdvKeepChoice();
}


ULONG
sdv_ExIsResourceAcquiredLite (
    __in PERESOURCE Resource
    )
{
    return (ULONG)SdvKeepChoice();
}


VOID
FASTCALL
ExReleaseResourceAndLeaveCriticalRegion(
    __inout PERESOURCE Resource
    )
{

}


VOID
FASTCALL
ExReleaseRundownProtectionCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
     )
{

}


VOID
FASTCALL
ExReleaseRundownProtectionCacheAwareEx (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRef,
     __in ULONG Count
     )
{

}

VOID
FASTCALL
ExWaitForRundownProtectionRelease (
     __inout PEX_RUNDOWN_REF RunRef
     )
{

}


VOID
FASTCALL
ExWaitForRundownProtectionReleaseCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRef
     )
{

}

VOID 
sdv_ExFreeToPagedLookasideList(
  __inout  PPAGED_LOOKASIDE_LIST Lookaside,
  __in     PVOID Entry
)
{
}

PVOID 
sdv_ExAllocateFromPagedLookasideList(
  __inout  PPAGED_LOOKASIDE_LIST Lookaside
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return malloc(1);
            break;
    }
}

BOOLEAN ExIsProcessorFeaturePresent(
  __in  ULONG ProcessorFeature
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}

VOID ExDeletePagedLookasideList(
  __inout  PPAGED_LOOKASIDE_LIST Lookaside
)
{
}

VOID ExInitializePagedLookasideList(
  __out     PPAGED_LOOKASIDE_LIST Lookaside,
  __in_opt  PALLOCATE_FUNCTION Allocate,
  __in_opt  PFREE_FUNCTION Free,
  __in      ULONG Flags,
  __in      SIZE_T Size,
  __in      ULONG Tag,
  __in      USHORT Depth
)
{
    PPAGED_LOOKASIDE_LIST x = malloc(1);
    *Lookaside = *x;
}

PVOID ExRegisterCallback(
  __inout   PCALLBACK_OBJECT CallbackObject,
  __in      PCALLBACK_FUNCTION CallbackFunction,
  __in_opt  PVOID CallbackContext
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }

}

VOID ExUnregisterCallback(
  __inout  PVOID CbRegistration
)
{

}

ULONG ExSetTimerResolution(
  __in  ULONG DesiredTime,
  __in  BOOLEAN SetResolution
)
{
    return DesiredTime;
}

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64 (
    __inout __drv_interlocked LONGLONG volatile *Destination,
    __in PLONGLONG ExChange,
    __in PLONGLONG Comperand
    )
{
  ULONGLONG r = SdvKeepChoice();
}

LONGLONG ExInterlockedCompareExchange64(
  __inout  PLONGLONG Destination,
  __in     PLONGLONG Exchange,
  __in     PLONGLONG Comparand,
  __in     PKSPIN_LOCK Lock
)
{
  ULONGLONG r = SdvKeepChoice();
}
    




VOID ExSetResourceOwnerPointer(
  __inout  PERESOURCE Resource,
  __in     PVOID OwnerPointer
)
{
}


VOID ExSetResourceOwnerPointerEx(
  __inout  PERESOURCE Resource,
  __in     PVOID OwnerPointer,
  __in     ULONG Flags
)
{
}

PSINGLE_LIST_ENTRY 
FASTCALL
ExfInterlockedPopEntryList(
  __inout  PSINGLE_LIST_ENTRY ListHead,
  __inout  PKSPIN_LOCK Lock
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}


PLIST_ENTRY 
FASTCALL
ExfInterlockedRemoveHeadList(
  __inout  PLIST_ENTRY ListHead,
  __inout  PKSPIN_LOCK Lock
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}

LARGE_INTEGER ExInterlockedAddLargeInteger(
  __inout  PLARGE_INTEGER Addend,
  __in     LARGE_INTEGER Increment,
  __inout  PKSPIN_LOCK Lock
)
{

}



/*BOOLEAN ExIsResourceAcquired(
  IN PERESOURCE  Resource
)
{
  LONG choice = SdvMakeChoice();
  switch(choice) 
  {
      case 0:
          return TRUE;
          break;
      default:
          return FALSE;
          break;
  }
}*/

/*BOOLEAN ExIsResourceAcquiredShared(
  IN PERESOURCE  Resource
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}*/


PSLIST_ENTRY sdv_ExInterlockedPushEntrySList(
  __inout  PSLIST_HEADER ListHead,
  __inout  PSLIST_ENTRY ListEntry,
  __inout  PKSPIN_LOCK Lock
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}

PSLIST_ENTRY sdv_ExInterlockedPopEntrySList(
  __inout  PSLIST_HEADER ListHead,
  __inout  PKSPIN_LOCK Lock
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}

VOID sdv_ExReleaseResourceForThread(
  IN PERESOURCE  Resource,
  IN ERESOURCE_THREAD  ResourceThreadId
)
{
}

BOOLEAN sdv_ExIsResourceAcquiredShared(
  IN PERESOURCE  Resource
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN sdv_ExIsResourceAcquired(
  IN PERESOURCE  Resource
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}

VOID
sdv_ExAcquireSpinLockAtDpcLevel(
    IN PKSPIN_LOCK  SpinLock
    ) 
{
}



VOID
sdv_ExAcquireSpinLock(
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL p_old_irql
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    (*p_old_irql) = sdv_irql_previous;
}

VOID sdv_ExReleaseSpinLockFromDpcLevel(
  __inout  PKSPIN_LOCK SpinLock
)
{
    
}

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    __inout PSLIST_HEADER ListHead,
    __inout __drv_aliasesMem PSLIST_ENTRY ListEntry
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}

#if ((!POOL_NX_OPTIN) && (!POOL_NX_OPTIN_AUTO))

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    __inout PSLIST_HEADER ListHead
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}
#endif


#if !defined(_X86_)
NTKERNELAPI
PSLIST_ENTRY
FASTCALL
sdv_ExInterlockedFlushSList(
     __inout PSLIST_HEADER ListHead
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}
#else
NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList(
     __inout PSLIST_HEADER ListHead
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PVOID)malloc(1);
            break;
    }
}
#endif


BOOLEAN
FASTCALL
ExiTryToAcquireFastMutex(
    __inout __deref __drv_acquiresExclusiveResource(FastMutexType)
    PFAST_MUTEX FastMutex
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

VOID
FASTCALL
ExiReleaseFastMutex(
    __inout __deref __drv_releasesExclusiveResource(FastMutexType)
    __inout PFAST_MUTEX FastMutex
    )
{
}

VOID
FASTCALL
ExiAcquireFastMutex(
    __inout __deref __drv_acquiresExclusiveResource(FastMutexType)
    PFAST_MUTEX FastMutex
    )
{
}


NTKERNELAPI
VOID
ExDeleteLookasideListEx (
    __inout PLOOKASIDE_LIST_EX Lookaside
    )
{
}

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    __inout PNPAGED_LOOKASIDE_LIST Lookaside
    )
{
}

NTKERNELAPI
VOID
ExFreePoolWithTag(
    __in __drv_freesMem(Mem) PVOID P,
    __in ULONG Tag
    )
{
}

NTKERNELAPI
NTSTATUS
ExInitializeLookasideListEx (
    __out PLOOKASIDE_LIST_EX Lookaside,
    __in_opt PALLOCATE_FUNCTION_EX Allocate,
    __in_opt PFREE_FUNCTION_EX Free,
    __in POOL_TYPE PoolType,
    __in ULONG Flags,
    __in SIZE_T Size,
    __in ULONG Tag,
    __in USHORT Depth
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

#if ((!POOL_NX_OPTIN) && (!POOL_NX_OPTIN_AUTO))
NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    PNPAGED_LOOKASIDE_LIST Lookaside,
    PALLOCATE_FUNCTION Allocate,
    PFREE_FUNCTION Free,
    ULONG Flags,
    SIZE_T Size,
    ULONG Tag,
    USHORT Depth
    )
{
}
#endif 

#if !POOL_NX_OPTOUT && (POOL_NX_OPTIN || POOL_NX_OPTIN_AUTO)


FORCEINLINE
VOID
sdv_ExInitializeNPagedLookasideList_NXPoolOptIn (
    PNPAGED_LOOKASIDE_LIST Lookaside,
    PALLOCATE_FUNCTION Allocate,
    PFREE_FUNCTION Free,
    ULONG Flags,
    SIZE_T Size,
    ULONG Tag,
    USHORT Depth
    )
{
    /*ExInitializeNPagedLookasideList(Lookaside,
                                    Allocate,
                                    Free,
#if POOL_NX_OPTIN_AUTO
                                    Flags | POOL_NX_ALLOCATION,
#else
                                    Flags | (ULONG) ExDefaultNonPagedPoolType,
#endif
                                    Size,
                                    Tag,
                                    Depth);*/
}

#endif


/* ntddk-ex.c end */

/* ntddk-fs.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/
#ifdef SDV_Include_NTIFS
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryObject(
     HANDLE Handle,
     OBJECT_INFORMATION_CLASS ObjectInformationClass,
     PVOID ObjectInformation,
     ULONG ObjectInformationLength,
     PULONG ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationToken (
     HANDLE TokenHandle,
     TOKEN_INFORMATION_CLASS TokenInformationClass,
     PVOID TokenInformation,
     ULONG TokenInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


LONG KeInsertQueue(
  __inout  PRKQUEUE Queue,
  __inout  PLIST_ENTRY Entry
)
{
    return (LONG)SdvKeepChoice();
    
}

PLIST_ENTRY KeRemoveQueue(
  __inout   PRKQUEUE Queue,
  __in      KPROCESSOR_MODE WaitMode,
  __in_opt  PLARGE_INTEGER Timeout
)
{
  PVOID p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PVOID)malloc(1);break;
        default:return  p=NULL;break;
  }
}


LONG
KeInsertHeadQueue(
  __inout  PRKQUEUE Queue,
  __inout  PLIST_ENTRY Entry
)
{
    return (LONG)SdvKeepChoice();
}

NTKERNELAPI
NTSTATUS
MmPrefetchPages (
     ULONG NumberOfLists,
     PREAD_LIST *ReadLists
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateToken(
     HANDLE ExistingTokenHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     BOOLEAN EffectiveOnly,
     TOKEN_TYPE TokenType,
     PHANDLE NewTokenHandle
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationToken (
     HANDLE TokenHandle,
     TOKEN_INFORMATION_CLASS TokenInformationClass,
     PVOID TokenInformation,
     ULONG TokenInformationLength,
     PULONG ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryQuotaInformationFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID Buffer,
     ULONG Length,
     BOOLEAN ReturnSingleEntry,
     PVOID SidList,
     ULONG SidListLength,
     PSID StartSid,
     BOOLEAN RestartScan
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTKERNELAPI
NTSTATUS
IoCheckQuotaBufferValidity(
    IN PFILE_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG ErrorOffset
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_DATATYPE_MISALIGNMENT;break;
        default: return STATUS_QUOTA_LIST_INCONSISTENT;break;
    }
}

NTKERNELAPI
NTSTATUS
IoRegisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
IoRegisterFsRegistrationChangeEx(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDRIVER_FS_NOTIFICATION  DriverNotificationRoutine
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}


NTKERNELAPI
BOOLEAN
FsRtlIsTotalDeviceFailure(
    IN NTSTATUS Status
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

PFILE_LOCK FsRtlAllocateFileLock(
  __in_opt  PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine,
  __in_opt  PUNLOCK_ROUTINE UnlockRoutine
)
{
    return (PFILE_LOCK)malloc(1);
}

BOOLEAN FsRtlCheckLockForReadAccess(
  __in  PFILE_LOCK FileLock,
  __in  PIRP Irp
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

BOOLEAN FsRtlCheckLockForWriteAccess(
  __in  PFILE_LOCK FileLock,
  __in  PIRP Irp
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

BOOLEAN FsRtlFastCheckLockForRead(
  __in  PFILE_LOCK FileLock,
  __in  PLARGE_INTEGER StartingByte,
  __in  PLARGE_INTEGER Length,
  __in  ULONG Key,
  __in  PFILE_OBJECT FileObject,
  __in  PVOID ProcessId
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }

}

BOOLEAN FsRtlFastCheckLockForWrite(
  __in  PFILE_LOCK FileLock,
  __in  PLARGE_INTEGER StartingByte,
  __in  PLARGE_INTEGER Length,
  __in  ULONG Key,
  __in  PVOID FileObject,
  __in  PVOID ProcessId
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTSTATUS FsRtlFastUnlockAll(
  __in      PFILE_LOCK FileLock,
  __in      PFILE_OBJECT FileObject,
  __in      PEPROCESS ProcessId,
  __in_opt  PVOID Context
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_SUCCESS;
            break;
        default:
            return STATUS_RANGE_NOT_LOCKED;
            break;
    }
}

NTSTATUS FsRtlFastUnlockAllByKey(
  __in      PFILE_LOCK FileLock,
  __in      PFILE_OBJECT FileObject,
  __in      PEPROCESS ProcessId,
  __in      ULONG Key,
  __in_opt  PVOID Context)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_SUCCESS;
            break;
        default:
            return STATUS_RANGE_NOT_LOCKED;
            break;
    }
}

NTSTATUS FsRtlFastUnlockSingle(
  __in      PFILE_LOCK FileLock,
  __in      PFILE_OBJECT FileObject,
  __in      LARGE_INTEGER UNALIGNED *FileOffset,
  __in      PLARGE_INTEGER Length,
  __in      PEPROCESS ProcessId,
  __in      ULONG Key,
  __in_opt  PVOID Context,
  __in      BOOLEAN AlreadySynchronized
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_SUCCESS;
            break;
        default:
            return STATUS_RANGE_NOT_LOCKED;
            break;
    }
}

VOID FsRtlFreeFileLock(
  __in  PFILE_LOCK FileLock
)
{
}

PFILE_LOCK_INFO FsRtlGetNextFileLock(
  __in  PFILE_LOCK FileLock,
  __in  BOOLEAN Restart
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PFILE_LOCK_INFO)malloc(1);
            break;
    }
}

VOID FsRtlIncrementCcFastReadNoWait(
    VOID 
)
{
}

VOID FsRtlIncrementCcFastReadWait(
    VOID 
)
{
}

VOID FsRtlInitializeFileLock(
  __in      PFILE_LOCK FileLock,
  __in_opt  PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine,
  __in_opt  PUNLOCK_ROUTINE UnlockRoutine
)
{
}

BOOLEAN
FsRtlPrivateLock (
    __in PFILE_LOCK FileLock,
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in PLARGE_INTEGER Length,
    __in PEPROCESS ProcessId,
    __in ULONG Key,
    __in BOOLEAN FailImmediately,
    __in BOOLEAN ExclusiveLock,
    __out PIO_STATUS_BLOCK Iosb,
    __in_opt PIRP Irp,
    __in_opt __drv_aliasesMem PVOID Context,
    __in BOOLEAN AlreadySynchronized
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTSTATUS FsRtlProcessFileLock(
  __in      PFILE_LOCK FileLock,
  __in      PIRP Irp,
  __in_opt  PVOID Context
)
{


    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_INSUFFICIENT_RESOURCES;
            break;
        case 1:
            return STATUS_INVALID_DEVICE_REQUEST;
            break;
        case 2:
            return STATUS_RANGE_NOT_LOCKED;
            break;
        default:
            return STATUS_SUCCESS;
            break;
    }
}

VOID FsRtlUninitializeFileLock(
  __in  PFILE_LOCK FileLock
)
{
}


NTSYSAPI
NTSTATUS
NTAPI
NtLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_UNSUCCESSFUL;
            break;
        default:
            return STATUS_SUCCESS;
            break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
NtUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_UNSUCCESSFUL;
            break;
        default:
            return STATUS_SUCCESS;
            break;
    }
}

NTSTATUS CcWaitForCurrentLazyWriterActivity()
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_INSUFFICIENT_RESOURCES;
            break;
        default:
            return STATUS_SUCCESS;
            break;
    }
}

/*BOOLEAN 
FsRtlFastLock(
  __in   PFILE_LOCK FileLock,
  __in   PFILE_OBJECT FileObject,
  __in   PLARGE_INTEGER FileOffset,
  __in   PLARGE_INTEGER Length,
  __in   PEPROCESS ProcessId,
  __in   ULONG Key,
  __in   BOOLEAN FailImmediately,
  __in   BOOLEAN ExclusiveLock,
  __out  PIO_STATUS_BLOCK Iosb,
  __in   PVOID Context,
  __in   BOOLEAN AlreadySynchronized
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }

}*/

NTSTATUS FsRtlCancellableWaitForSingleObject(
  __in      PVOID Object,
  __in_opt  PLARGE_INTEGER Timeout,
  __in_opt  PIRP Irp
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_TIMEOUT;
            break;
        case 1:
            return STATUS_WAIT_0;
            break;
        case 2:
            return STATUS_ABANDONED_WAIT_0;
            break;
        case 3:
            return STATUS_CANCELLED;
            break;
        case 4:
            return STATUS_THREAD_IS_TERMINATING;
            break;
    
        default:
            return STATUS_SUCCESS;
            break;
    }
}

NTSTATUS  FsRtlCancellableWaitForMultipleObjects(
  __in      ULONG Count,
  __in      PVOID ObjectArray,
  __in      WAIT_TYPE WaitType,
  __in_opt  PLARGE_INTEGER Timeout,
  __in_opt  PKWAIT_BLOCK WaitBlockArray,
  __in_opt  PIRP Irp
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return STATUS_TIMEOUT;
            break;
        case 1:
            return STATUS_WAIT_0;
            break;
        case 2:
            return STATUS_ABANDONED_WAIT_0;
            break;
        case 3:
            return STATUS_CANCELLED;
            break;
        case 4:
            return STATUS_THREAD_IS_TERMINATING;
            break;
    
        default:
            return STATUS_SUCCESS;
            break;
    }
}



BOOLEAN
FsRtlAreNamesEqual (
     PCUNICODE_STRING ConstantNameA,
     PCUNICODE_STRING ConstantNameB,
     BOOLEAN IgnoreCase,
     PCWCH UpcaseTable
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
NTSTATUS
FsRtlBalanceReads (
     PDEVICE_OBJECT TargetDevice
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

/*
NTSYSAPI
NTSTATUS
NTAPI
FsRtlChangeBackingFileObject (
     PFILE_OBJECT CurrentFileObject,
     PFILE_OBJECT NewFileObject,
     FSRTL_CHANGE_BACKING_TYPE ChangeBackingType,
     ULONG Flags                
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}*/

BOOLEAN
FsRtlCopyWrite (
     PFILE_OBJECT FileObject,
     PLARGE_INTEGER FileOffset,
     ULONG Length,
     BOOLEAN Wait,
     ULONG LockKey,
     PVOID Buffer,
     PIO_STATUS_BLOCK IoStatus,
     PDEVICE_OBJECT DeviceObject
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

VOID
FsRtlDeregisterUncProvider(
     HANDLE Handle
    ) 
{
}

VOID
FsRtlDissectName (
     UNICODE_STRING Path,
     PUNICODE_STRING FirstName,
     PUNICODE_STRING RemainingName
    ) 
{
}

BOOLEAN
FsRtlDoesNameContainWildCards (
     PUNICODE_STRING Name
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
FsRtlGetFileSize(
     PFILE_OBJECT FileObject,
     PLARGE_INTEGER FileSize
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

BOOLEAN
FsRtlIsNameInExpression (
     PUNICODE_STRING Expression,
     PUNICODE_STRING Name,
     BOOLEAN IgnoreCase,
     PWCH UpcaseTable
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

BOOLEAN
FsRtlMdlReadDev (
     PFILE_OBJECT FileObject,
     PLARGE_INTEGER FileOffset,
     ULONG Length,
     ULONG LockKey,
     PMDL *MdlChain,
     PIO_STATUS_BLOCK IoStatus,
     PDEVICE_OBJECT DeviceObject
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

BOOLEAN
FsRtlMdlReadCompleteDev (
     PFILE_OBJECT FileObject,
     PMDL MdlChain,
     PDEVICE_OBJECT DeviceObject
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

BOOLEAN
FsRtlMdlWriteCompleteDev (
     PFILE_OBJECT FileObject,
     PLARGE_INTEGER FileOffset,
     PMDL MdlChain,
     PDEVICE_OBJECT DeviceObject
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}

VOID
FsRtlNotifyFilterChangeDirectory (
     PNOTIFY_SYNC NotifySync,
     PLIST_ENTRY NotifyList,
     PVOID FsContext,
     PSTRING FullDirectoryName,
     BOOLEAN WatchTree,
     BOOLEAN IgnoreBuffer,
     ULONG CompletionFilter,
     PIRP NotifyIrp,
     PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback,
     PSECURITY_SUBJECT_CONTEXT SubjectContext,
     PFILTER_REPORT_CHANGE FilterCallback
    ) 
{
}

VOID
FsRtlNotifyFilterReportChange (
     PNOTIFY_SYNC NotifySync,
     PLIST_ENTRY NotifyList,
     PSTRING FullTargetName,
     USHORT TargetNameOffset,
     PSTRING StreamName,
     PSTRING NormalizedParentName,
     ULONG FilterMatch,
     ULONG Action,
     PVOID TargetContext,
     PVOID FilterContext
    ) 
{
}

VOID
FsRtlNotifyFullChangeDirectory (
     PNOTIFY_SYNC NotifySync,
     PLIST_ENTRY NotifyList,
     PVOID FsContext,
     PSTRING FullDirectoryName,
     BOOLEAN WatchTree,
     BOOLEAN IgnoreBuffer,
     ULONG CompletionFilter,
     PIRP NotifyIrp,
     PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback,
     PSECURITY_SUBJECT_CONTEXT SubjectContext
    ) 
{
}

VOID
FsRtlNotifyFullReportChange (
     PNOTIFY_SYNC NotifySync,
     PLIST_ENTRY NotifyList,
     PSTRING FullTargetName,
     USHORT TargetNameOffset,
     PSTRING StreamName,
     PSTRING NormalizedParentName,
     ULONG FilterMatch,
     ULONG Action,
     PVOID TargetContext
    ) 
{
}

_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
NTSTATUS
FsRtlRegisterUncProviderEx(
     PHANDLE MupHandle,
     PUNICODE_STRING RedirDevName,
     PDEVICE_OBJECT DeviceObject,
     ULONG Flags
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
NTSTATUS
FsRtlRegisterUncProvider(
    __out PHANDLE MupHandle,
    __in PUNICODE_STRING RedirectorDeviceName,
    __in BOOLEAN MailslotsSupported
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
NTSTATUS
FsRtlRemoveDotsFromPath(
     PWSTR OriginalString,
     USHORT PathLength,
     USHORT *NewLength
) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
NTSTATUS
FsRtlValidateReparsePointBuffer (
     ULONG BufferLength,
     PREPARSE_DATA_BUFFER ReparseBuffer
) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif
/* ntddk-fs.c end */

/* ntddk-io.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/



typedef struct _IO_WORKITEM *PIO_WORKITEM;

typedef
VOID
(*PIO_WORKITEM_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

typedef
VOID
(*PIO_WORKITEM_ROUTINE_EX) (
    IN PVOID IoObject,
    IN PVOID Context,
    IN PIO_WORKITEM IoWorkItem
    );

typedef struct _IO_WORKITEM {
    WORK_QUEUE_ITEM WorkItem;
    PIO_WORKITEM_ROUTINE_EX Routine;
    PVOID IoObject;
    PVOID Context;
    ULONG Type;
#if DBG
    ULONG Size;
#endif
} IO_WORKITEM;



DEVICE_OBJECT sdv_devobj_top;
PDEVICE_OBJECT sdv_p_devobj_top = &sdv_devobj_top;

#ifdef SDV_Include_NTDDK
CONTROLLER_OBJECT sdv_IoCreateController_CONTROLLER_OBJECT;
#endif

KEVENT sdv_IoCreateNotificationEvent_KEVENT;
KEVENT sdv_IoCreateSynchronizationEvent_KEVENT;
DEVICE_OBJECT sdv_IoGetDeviceObjectPointer_DEVICE_OBJECT;
DEVICE_OBJECT sdv_IoGetDeviceToVerify_DEVICE_OBJECT;
DMA_ADAPTER sdv_IoGetDmaAdapter_DMA_ADAPTER;
GENERIC_MAPPING sdv_IoGetFileObjectGenericMapping_GENERIC_MAPPING;
DEVICE_OBJECT sdv_IoGetRelatedDeviceObject_DEVICE_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION_EX sdv_IoReadPartitionTableEx_DRIVE_LAYOUT_INFORMATION_EX;




NTKERNELAPI
VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL p_old_irql
    )
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    *p_old_irql = sdv_irql_previous;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTKERNELAPI
NTSTATUS
NTAPI
IoAcquireRemoveLockEx(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag,
    IN PCSTR File,
    IN ULONG Line,
    IN ULONG RemlockSize
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_UNSUCCESSFUL;break;
        default: 
            if (sdv_Io_Removelock_release_wait_returned) 
            {
                return STATUS_DELETE_PENDING;
            }
            else 
            {
	              return STATUS_SUCCESS;
            }
			break;
    }
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTKERNELAPI
NTSTATUS
NTAPI
sdv_IoAcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_UNSUCCESSFUL;break;
        default: 
            if (sdv_Io_Removelock_release_wait_returned) 
            {
                return STATUS_DELETE_PENDING;
            }
            else 
            {
	              return STATUS_SUCCESS;
            }
			break;
    }
}


NTKERNELAPI
VOID
sdv_IoAdjustPagingPathCount(
    IN PLONG Count,
    IN BOOLEAN Increment
    )
{
}

NTKERNELAPI
NTSTATUS
sdv_IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}
#ifdef SDV_Include_NTDDK
NTKERNELAPI
VOID
IoAllocateController(
    IN PCONTROLLER_OBJECT ControllerObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )
{
#ifdef SDV_HARNESS_DRIVER_CONTROL_ROUTINE
	sdv_stub_driver_control_begin();
#ifdef fun_DRIVER_CONTROL
    fun_DRIVER_CONTROL(DeviceObject,sdv_ControllerPirp,sdv_MapRegisterBase,Context);
#else
    sdv_DoNothing();
#endif
    sdv_stub_driver_control_end();
#endif
}
#endif


NTKERNELAPI
NTSTATUS
IoAllocateDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress,
    IN ULONG DriverObjectExtensionSize,
    OUT PVOID *DriverObjectExtension
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_OBJECT_NAME_COLLISION;break;
    }
}

NTKERNELAPI
PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return malloc(1);break;
        default: return NULL;break;
    }
}

NTKERNELAPI
PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    )
{
    PIO_STACK_LOCATION irpSp;
    int choice = SdvMakeChoice();
    switch (choice)
	  {
        case 0: 
            sdv_other_irp->PendingReturned=0;
            irpSp = IoGetNextIrpStackLocation(sdv_other_irp);
            irpSp->CompletionRoutine = NULL;
            sdv_compFset = 0;
            return sdv_other_irp;
            break;
        default: return NULL;break;
    }

}

NTKERNELAPI
PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return (PMDL) malloc(1);break;
        default: return NULL;break;
    }
}

PIO_WORKITEM
IoAllocateWorkItem(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    int choice = SdvMakeChoice();
    PIO_WORKITEM ioWorkItem = (PIO_WORKITEM)malloc(1);
    switch (choice) 
    {
      case 0:
	  SdvAssume(ioWorkItem != NULL);
	  ioWorkItem->WorkItem.List.Flink = NULL;
		
          return ioWorkItem;
      break;
      default:

          return NULL;
      break;

    }
}

VOID
sdv_IoAssignArcName(
    IN PUNICODE_STRING ArcName,
    IN PUNICODE_STRING DeviceName
    )
{
}

NTKERNELAPI
NTSTATUS
IoAssignResources(
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIO_RESOURCE_REQUIREMENTS_LIST RequestedResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INVALID_PARAMETER;break;
        case 2: return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 3: return STATUS_OBJECT_NAME_INVALID;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTKERNELAPI
NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )
{
    if (TargetDevice == sdv_p_devobj_pdo) {
        return TargetDevice;
    } else {
        return NULL;
    }
}

NTKERNELAPI
PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    )
{

    int choice = SdvMakeChoice();
    if(IoStatusBlock!=NULL)
    {
        IoStatusBlock = sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock;
    }
    switch (choice) 
    {
        case 0:
            sdv_IoBuildAsynchronousFsdRequest_irp->PendingReturned=1;
            sdv_IoBuildAsynchronousFsdRequest_irp->Tail.Overlay.CurrentStackLocation->MajorFunction=(UCHAR) MajorFunction;
	    sdv_IoBuildAsynchronousFsdRequest_harnessStackLocation_next.MajorFunction = (UCHAR) MajorFunction;
	    if(IoStatusBlock!=NULL)
	    {
	        IoStatusBlock->Status = STATUS_SUCCESS;
                sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status = STATUS_SUCCESS;
                sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock=IoStatusBlock;
	    }
	    return sdv_IoBuildAsynchronousFsdRequest_irp;
	    break;
	    default:
	    if(IoStatusBlock!=NULL)
	    {
                 IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
                 sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
                 sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock=IoStatusBlock;
	    }
	    return NULL;
	    break;
    }
}

NTKERNELAPI
PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
{
    int choice = SdvMakeChoice();
    
    
    switch (choice) 
    {

    case 0:
        if(InternalDeviceIoControl)
        {
            sdv_IoBuildDeviceIoControlRequest_irp->Tail.Overlay.CurrentStackLocation->MajorFunction=IRP_MJ_INTERNAL_DEVICE_CONTROL;
            sdv_IoBuildDeviceIoControlRequest_harnessStackLocation_next.MajorFunction=IRP_MJ_INTERNAL_DEVICE_CONTROL;
        }
        else
        {
            sdv_IoBuildDeviceIoControlRequest_irp->Tail.Overlay.CurrentStackLocation->MajorFunction=IRP_MJ_DEVICE_CONTROL;
            sdv_IoBuildDeviceIoControlRequest_harnessStackLocation_next.MajorFunction=IRP_MJ_DEVICE_CONTROL;
        }
        sdv_IoBuildDeviceIoControlRequest_irp->PendingReturned=1;
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_SUCCESS;
        IoStatusBlock->Status=STATUS_SUCCESS;
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock=IoStatusBlock;
        return sdv_IoBuildDeviceIoControlRequest_irp;
        break;
    default:
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
        IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock=IoStatusBlock;
        return NULL;
        break;

    }
}

NTKERNELAPI
VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    )
{
}

NTKERNELAPI
PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
{
    
    int choice = SdvMakeChoice();
    
    switch (choice) 
    {

    case 0:
        sdv_IoBuildSynchronousFsdRequest_irp->Tail.Overlay.CurrentStackLocation->MajorFunction=(UCHAR) MajorFunction;
        sdv_IoBuildSynchronousFsdRequest_harnessStackLocation_next.MajorFunction = (UCHAR) MajorFunction;
        IoStatusBlock->Status = STATUS_SUCCESS;
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock=STATUS_SUCCESS;
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock=IoStatusBlock;
        sdv_IoBuildSynchronousFsdRequest_irp->PendingReturned=1;
        return sdv_IoBuildSynchronousFsdRequest_irp;
        break;
    default:
        sdv_IoBuildSynchronousFsdRequest_irp->PendingReturned=0;
        IoStatusBlock->Status =STATUS_UNSUCCESSFUL;
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock=STATUS_UNSUCCESSFUL;
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock=IoStatusBlock;
        return NULL;
        break;

    }
}

NTKERNELAPI
NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    int choice;
    BOOLEAN completion_routine_called=FALSE;
    NTSTATUS status;
    choice= SdvMakeChoice();
    status=STATUS_PENDING;
    switch (choice) 
    {
      case 0:
      Irp->IoStatus.Status = STATUS_SUCCESS;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
	  if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_success&&sdv_compFset)
      { 
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      case 1:
      Irp->IoStatus.Status = STATUS_CANCELLED;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
	  if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_cancel&&sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      case 3:
      Irp->IoStatus.Status = STATUS_PENDING;
      Irp->PendingReturned = 1;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      default:
      Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_error&&sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
   }
   return status;
}


NTKERNELAPI
NTSTATUS
sdv_IoCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    return IofCallDriver(DeviceObject,Irp);
}



NTKERNELAPI
BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_SHARING_VIOLATION;break;
    }
}

VOID
FASTCALL
IofCompleteRequest(
    IN PIRP pirp,
    IN CCHAR PriorityBoost
    )
{
}

VOID
sdv_IoCompleteRequest(
    IN PIRP pirp,
    IN CCHAR PriorityBoost
    )
{
}



VOID
sdv_IofCompleteRequest(
    IN PIRP pirp,
    IN CCHAR PriorityBoost
    )
{
}


NTKERNELAPI
NTSTATUS
IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave
    )
{
    int choice = SdvMakeChoice();
    sdv_isr_routine = ServiceRoutine;
    sdv_pDpcContext = ServiceContext;
    switch (choice) 
    {
        case 0: 
#ifdef SDV_RUN_KE_ISR_ROUTINES
        sdv_RunISRRoutines(InterruptObject,ServiceContext);
#endif
                 return STATUS_SUCCESS;
                 break;
        case 1: return STATUS_INVALID_PARAMETER;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

VOID   
IoDisconnectInterrupt(
    IN PKINTERRUPT  InterruptObject
    )
{
    sdv_isr_routine = sdv_isr_dummy;
    sdv_pDpcContext = &sdv_DpcContext;
    
}

#if WINVER >= 0x0600
NTKERNELAPI
NTSTATUS
IoConnectInterruptEx(
    IN OUT PIO_CONNECT_INTERRUPT_PARAMETERS Parameters
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INVALID_DEVICE_REQUEST;break;
        case 3: return STATUS_INVALID_PARAMETER;break;
        case 4: return STATUS_INVALID_PARAMETER_1;break;
        case 5: return STATUS_INVALID_PARAMETER_10;break;
        default: return STATUS_NOT_FOUND;break;
    }
}
#endif

FORCEINLINE
VOID
sdv_IoCopyCurrentIrpStackLocationToNext(
    IN PIRP pirp
    )
{
   SDV_MACRO_COPYCURRENTIRPSTACKLOCATIONTONEXT(pirp)
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
PCONTROLLER_OBJECT
IoCreateController(
    IN ULONG Size
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return &sdv_IoCreateController_CONTROLLER_OBJECT;break;
        default: return NULL;break;
    }
}
#endif

NTKERNELAPI
NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    )
{

    int choice = SdvMakeChoice();
    sdv_io_create_device_called++;

    switch (choice) 
    {
        case 0: 
            if(sdv_inside_init_entrypoint)
            {
                sdv_p_devobj_fdo->Flags = DO_DEVICE_INITIALIZING;
                (*DeviceObject) = sdv_p_devobj_fdo; 
            }
            else
            {
                sdv_p_devobj_child_pdo->Flags = DO_DEVICE_INITIALIZING;
                (*DeviceObject) = sdv_p_devobj_child_pdo; 
            }
            return STATUS_SUCCESS;break;
        case 1: (*DeviceObject) = NULL; return STATUS_UNSUCCESSFUL;break;
        case 2: (*DeviceObject) = NULL; return STATUS_INSUFFICIENT_RESOURCES;break;
        default: (*DeviceObject) = NULL; return STATUS_OBJECT_NAME_COLLISION;break;
    }
}

NTSTATUS
WdmlibIoCreateDeviceSecure(
    __in     PDRIVER_OBJECT      DriverObject,
    __in     ULONG               DeviceExtensionSize,
    __in_opt PUNICODE_STRING     DeviceName,
    __in     DEVICE_TYPE         DeviceType,
    __in     ULONG               DeviceCharacteristics,
    __in     BOOLEAN             Exclusive,
    __in     PCUNICODE_STRING    DefaultSDDLString,
    __in_opt LPCGUID             DeviceClassGuid,
    __out    PDEVICE_OBJECT     *DeviceObject
    )
{

    int choice = SdvMakeChoice();
    sdv_io_create_device_called++;

    switch (choice) {
        case 0: (*DeviceObject) = sdv_p_devobj_fdo; return STATUS_SUCCESS;break;
        case 1: (*DeviceObject) = NULL; return STATUS_UNSUCCESSFUL;break;
        case 2: (*DeviceObject) = NULL; return STATUS_INSUFFICIENT_RESOURCES;break;
        default: (*DeviceObject) = NULL; return STATUS_OBJECT_NAME_COLLISION;break;
    }
}


NTSTATUS
sdv_IoCreateDeviceSecure(
  __in      PDRIVER_OBJECT DriverObject,
  __in      ULONG DeviceExtensionSize,
  __in_opt  PUNICODE_STRING DeviceName,
  __in      DEVICE_TYPE DeviceType,
  __in      ULONG DeviceCharacteristics,
  __in      BOOLEAN Exclusive,
  __in      PCUNICODE_STRING DefaultSDDLString,
  __in_opt  LPCGUID DeviceClassGuid,
  __out     PDEVICE_OBJECT *DeviceObject
)
{

    int choice = SdvMakeChoice();

    sdv_io_create_device_called++;

    switch (choice) 
    {
        case 0: 
            if(sdv_inside_init_entrypoint)
            {
                sdv_p_devobj_fdo->Flags = DO_DEVICE_INITIALIZING;
                (*DeviceObject) = sdv_p_devobj_fdo; 
            }
            else
            {
                sdv_p_devobj_child_pdo->Flags = DO_DEVICE_INITIALIZING;
                (*DeviceObject) = sdv_p_devobj_child_pdo; 
            }
            return STATUS_SUCCESS;
        break;
        case 1: (*DeviceObject) = NULL; return STATUS_UNSUCCESSFUL;break;
        case 2: (*DeviceObject) = NULL; return STATUS_INSUFFICIENT_RESOURCES;break;
        default: (*DeviceObject) = NULL; return STATUS_OBJECT_NAME_COLLISION;break;
    }
}



NTKERNELAPI
NTSTATUS
IoCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
PKEVENT
IoCreateNotificationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return &sdv_IoCreateNotificationEvent_KEVENT;break;
        default: return NULL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return &sdv_IoCreateSynchronizationEvent_KEVENT;break;
        default: return NULL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSTATUS
sdv_IoCsqInitialize(
    IN PIO_CSQ Csq,
    IN PIO_CSQ_INSERT_IRP CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP CsqCompleteCanceledIrp
    )
{
    Csq->CsqInsertIrp = CsqInsertIrp;
    Csq->CsqRemoveIrp = CsqRemoveIrp;
    Csq->CsqPeekNextIrp = CsqPeekNextIrp;
    Csq->CsqAcquireLock = CsqAcquireLock;
    Csq->CsqReleaseLock = CsqReleaseLock;
    Csq->CsqCompleteCanceledIrp = CsqCompleteCanceledIrp;
    Csq->ReservePointer = NULL;

    Csq->Type = IO_TYPE_CSQ;

    return STATUS_SUCCESS;
}

NTKERNELAPI
NTSTATUS
sdv_IoCsqInitializeEx(
    IN PIO_CSQ Csq,
    IN PIO_CSQ_INSERT_IRP_EX CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP CsqCompleteCanceledIrp 
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

VOID
sdv_IoCsqInsertIrp(
    IN PIO_CSQ Csq,
    IN PIRP pirp,
    IN PIO_CSQ_IRP_CONTEXT Context
    )
{
    Csq->CsqInsertIrp(Csq, pirp);
    IoMarkIrpPending(pirp);
}

NTSTATUS
sdv_IoCsqInsertIrpEx(
    IN PIO_CSQ Csq,
    IN PIRP pirp,
    IN PIO_CSQ_IRP_CONTEXT Context,
    IN PVOID InsertContext
    )
{
    NTSTATUS status;

    PIO_CSQ_INSERT_IRP_EX func;

    func = (PIO_CSQ_INSERT_IRP_EX)Csq->CsqInsertIrp;

    status = func(Csq, pirp, InsertContext);

    if (!NT_SUCCESS(status)) {
        return status;
    }
    IoMarkIrpPending(pirp);

    return status;
}

NTKERNELAPI
PIRP
sdv_IoCsqRemoveIrp(
    IN PIO_CSQ Csq,
    IN PIO_CSQ_IRP_CONTEXT Context
    )
{

    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return (PIRP) malloc(1);break;
        default: return NULL;break;
    }
}



PIRP sicrni;


PIRP
sdv_IoCsqRemoveNextIrp(
    IN PIO_CSQ Csq,
    IN PVOID PeekContext
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: 
		return sicrni; break;
        default: return NULL;break;
    }
}

NTKERNELAPI
VOID
sdv_IoDeassignArcName(
    IN PUNICODE_STRING ArcName
    )
{
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
VOID
IoDeleteController(
    IN PCONTROLLER_OBJECT ControllerObject
    )
{
}
#endif

NTKERNELAPI
VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
{
}

NTKERNELAPI
NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
VOID
IoDetachDevice(
        IN OUT PDEVICE_OBJECT TargetDevice
    )
{
}

#if WINVER >= 0x0600
NTKERNELAPI
VOID
IoDisconnectInterruptEx(
    IN OUT PIO_DISCONNECT_INTERRUPT_PARAMETERS Parameters
    )
{
}
#endif

NTKERNELAPI
BOOLEAN
sdv_IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
BOOLEAN
IoForwardIrpSynchronously(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {

    case 0:
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->PendingReturned = 0;
        return TRUE;break;
    default:
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        Irp->PendingReturned = 0;
        return FALSE;break;
    }
}

NTKERNELAPI
VOID
sdv_IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    )
{
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
VOID
IoFreeController(
    IN PCONTROLLER_OBJECT ControllerObject
    )
{
}
#endif

NTKERNELAPI
VOID
IoFreeErrorLogEntry(
    PVOID ElEntry
    )
{
}

NTKERNELAPI
VOID
IoFreeIrp(
    IN PIRP pirp
    )
{
}

VOID 
sdv_IoFreeMapRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN PVOID MapRegisterBase,
    IN ULONG NumberOfMapRegisters
    )
{
}

NTKERNELAPI
VOID
IoFreeMdl(
    IN PMDL Mdl
    )
{
}

VOID
IoFreeWorkItem(
    IN PIO_WORKITEM IoWorkItem
    )
{
}

NTKERNELAPI
PDEVICE_OBJECT
IoGetAttachedDeviceReference(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return sdv_p_devobj_top;break;
        default: return DeviceObject;break;
    }
    return &sdv_devobj_top;
}

NTKERNELAPI
NTSTATUS
IoGetBootDiskInformation(
    IN OUT PBOOTDISK_INFORMATION BootDiskInformation,
    IN ULONG Size
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INVALID_PARAMETER;break;
        default: return STATUS_TOO_LATE;break;
    }
}

#if defined(SDV_Include_NTIFS) || defined(SDV_Include_NTDDK)

NTKERNELAPI
PCONFIGURATION_INFORMATION
IoGetConfigurationInformation(
    VOID
    )
{
    return (PCONFIGURATION_INFORMATION) malloc(1);
}
#endif

PIO_STACK_LOCATION
sdv_IoGetCurrentIrpStackLocation(
    IN PIRP pirp
    )
{
    return (pirp->Tail.Overlay.CurrentStackLocation);
}

NTKERNELAPI
PEPROCESS
IoGetCurrentProcess(
    VOID
    )
{
    PEPROCESS p = (PEPROCESS) malloc(1);
    return p;
}

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaceAlias(
    IN PUNICODE_STRING SymbolicLinkName,
    IN CONST GUID *AliasInterfaceClassGuid,
    OUT PUNICODE_STRING AliasSymbolicLinkName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_OBJECT_NAME_NOT_FOUND;break;
        case 3: return STATUS_OBJECT_PATH_NOT_FOUND;break;
        default: return STATUS_INVALID_HANDLE;break;
    }
}

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaces(
    IN CONST GUID *InterfaceClassGuid,
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN ULONG Flags,
    OUT PWSTR *SymbolicLinkList
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: SdvAssume(*SymbolicLinkList != NULL);  
                return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_DEVICE_REQUEST;break;
    }
}

NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: *DeviceObject = &sdv_IoGetDeviceObjectPointer_DEVICE_OBJECT;
                return STATUS_SUCCESS;break;
        case 1: return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 2: return STATUS_INVALID_PARAMETER;break;
        case 3: return STATUS_PRIVILEGE_NOT_HELD;break;
        case 4: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_OBJECT_NAME_INVALID;break;
    }
}

NTKERNELAPI
NTSTATUS
IoGetDeviceProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_REGISTRY_PROPERTY DeviceProperty,
    IN ULONG BufferLength,
    OUT PVOID PropertyBuffer,
    OUT PULONG ResultLength
    )
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_UNSUCCESSFUL;break;
            case -1: return STATUS_INVALID_PARAMETER_2;break;
            default: return STATUS_INVALID_DEVICE_REQUEST;break;
        }
    } else if ( L <= BufferLength ) {
        *ResultLength = L;
        return STATUS_SUCCESS;
    } else {
        *ResultLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }
}

NTKERNELAPI
PDEVICE_OBJECT
IoGetDeviceToVerify(
    PETHREAD Thread
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return NULL;break;
        default: return &sdv_IoGetDeviceToVerify_DEVICE_OBJECT;break;
    }
}

NTKERNELAPI
VOID
IoSetDeviceToVerify(
    PETHREAD Thread,
    PDEVICE_OBJECT DeviceObject
    )
{
}


NTKERNELAPI
PDMA_ADAPTER
IoGetDmaAdapter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return NULL;break;
        default: return &sdv_IoGetDmaAdapter_DMA_ADAPTER;break;
    }
}

PVOID igdoe;

NTKERNELAPI
PVOID
IoGetDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
	case 0: 
		return igdoe; break;
        default: return NULL;break;
    }
}

NTKERNELAPI
PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    )
{
    return &sdv_IoGetFileObjectGenericMapping_GENERIC_MAPPING;
}

NTKERNELAPI
ULONG
sdv_IoGetFunctionCodeFromCtlCode(
    IN ULONG ControlCode
    )
{
    ULONG res;
    res = SdvKeepChoice();
    return res;
}

NTKERNELAPI
PVOID
IoGetInitialStack(
    VOID
    )
{
    return malloc(1);
}

PIO_STACK_LOCATION
sdv_IoGetNextIrpStackLocation(
    IN PIRP pirp
    )
{
    if (pirp == &sdv_harnessIrp) {
        return &sdv_harnessStackLocation_next;
    } else if (pirp == &sdv_other_harnessIrp) {
        return &sdv_other_harnessStackLocation_next;
    } else {
        return &sdv_harnessStackLocation;
    }
}

NTKERNELAPI
PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    )
{
    return &sdv_IoGetRelatedDeviceObject_DEVICE_OBJECT;
}

NTKERNELAPI
ULONG_PTR
sdv_IoGetRemainingStackSize(
    VOID
    )
{
    ULONG_PTR l = SdvKeepChoice();
    return l;
}

NTKERNELAPI
VOID
IoGetStackLimits(
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    )
{
}

VOID
sdv_IoInitializeDpcRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_DPC_ROUTINE DpcRoutine
    )
{
    sdv_io_dpc = DpcRoutine;
}

NTKERNELAPI
VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    )
{
	*Irp = *sdv_IoInitializeIrp_irp;
}

NTKERNELAPI
VOID
sdv_IoInitializeRemoveLock(
    IN PIO_REMOVE_LOCK Lock,
    IN ULONG AllocateTag,
    IN ULONG MaxLockedMinutes,
    IN ULONG HighWatermark
    )
{
}


NTKERNELAPI
NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

VOID
IoInitializeWorkItem(
    IN PVOID IoObject,
    IN PIO_WORKITEM IoWorkItem
    )
{
}

NTKERNELAPI
VOID
IoInvalidateDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_RELATION_TYPE Type
    )
{
}

NTKERNELAPI
VOID
IoInvalidateDeviceState(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
}

NTKERNELAPI
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
BOOLEAN
sdv_IoIsErrorUserInduced(
    IN NTSTATUS Status
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
BOOLEAN
IoIsWdmVersionAvailable(
    IN UCHAR MajorVersion,
    IN UCHAR MinorVersion
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
PIRP
IoMakeAssociatedIrp(
    IN PIRP pirp,
    IN CCHAR StackSize
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
	{
		case 0:
			sdv_IoMakeAssociatedIrp_irp->Tail.Overlay.CurrentStackLocation->MajorFunction=pirp->Tail.Overlay.CurrentStackLocation->MajorFunction;
			sdv_IoMakeAssociatedIrp_irp->IoStatus=pirp->IoStatus;
			return sdv_IoMakeAssociatedIrp_irp;
			break;
        default: 
			return NULL;
			break;
    }
}

NTKERNELAPI
PHYSICAL_ADDRESS
sdv_IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    )
{
    PHYSICAL_ADDRESS l;
    l.QuadPart = (LONGLONG) SdvKeepChoice();
    return l;
}

VOID
sdv_IoMarkIrpPending(
    IN OUT PIRP pirp
    )
{
}

NTKERNELAPI
NTSTATUS
IoOpenDeviceInterfaceRegistryKey(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceKey
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_OBJECT_NAME_NOT_FOUND;break;
        case 3: return STATUS_OBJECT_PATH_NOT_FOUND;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTKERNELAPI
NTSTATUS
IoOpenDeviceRegistryKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DevInstRegKey
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INVALID_PARAMETER;break;
        default: return STATUS_INVALID_DEVICE_REQUEST;break;
    }
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
NTSTATUS
IoQueryDeviceDescription(
    IN PINTERFACE_TYPE BusType OPTIONAL,
    IN PULONG BusNumber OPTIONAL,
    IN PCONFIGURATION_TYPE ControllerType OPTIONAL,
    IN PULONG ControllerNumber OPTIONAL,
    IN PCONFIGURATION_TYPE PeripheralType OPTIONAL,
    IN PULONG PeripheralNumber OPTIONAL,
    IN PIO_QUERY_DEVICE_ROUTINE CalloutRoutine,
    IN PVOID Context
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}
#endif

NTKERNELAPI
VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    )
{
  #ifdef SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
      sdv_RunIoQueueWorkItems(IoWorkItem,WorkerRoutine,QueueType,Context);
  #endif
}

NTKERNELAPI
VOID
IoQueueWorkItemEx(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE_EX WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    )
{
  #ifdef SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
      sdv_RunIoQueueWorkItemsEx(IoWorkItem,WorkerRoutine,QueueType,Context);
  #endif

}

NTKERNELAPI
VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    )
{
}

NTKERNELAPI
BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
VOID
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    )
{
}
#endif


NTKERNELAPI
NTSTATUS
NTAPI
IoRegisterDeviceInterface(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString, OPTIONAL
    OUT PUNICODE_STRING SymbolicLinkName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: SdvAssume(SymbolicLinkName->Buffer != NULL);
                return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_DEVICE_REQUEST;break;
    }
}

#ifdef SDV_Include_NTDDK
NTKERNELAPI
VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    )
{
}
#endif
NTKERNELAPI
NTSTATUS
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoRegisterPlugPlayNotification(
    IN IO_NOTIFICATION_EVENT_CATEGORY EventCategory,
    IN ULONG EventCategoryFlags,
    IN PVOID EventCategoryData OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Context,
    OUT PVOID *NotificationEntry
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
VOID
IoReleaseCancelSpinLock(
    IN KIRQL new_irql
    )
{
    SDV_IRQL_POPTO(new_irql);
}


NTKERNELAPI
VOID
NTAPI
IoReleaseRemoveLockEx(
     PIO_REMOVE_LOCK RemoveLock,
     PVOID       Tag, 
     ULONG           RemlockSize 
    )
{
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTKERNELAPI
VOID
NTAPI
IoReleaseRemoveLockAndWaitEx(
     PIO_REMOVE_LOCK RemoveLock,
     PVOID       Tag,
     ULONG           RemlockSize 
    )
{
}

VOID
sdv_IoReleaseRemoveLockEx(
     PIO_REMOVE_LOCK RemoveLock,
     PVOID       Tag, 
     ULONG           RemlockSize 
    )
{
}

VOID
sdv_IoReleaseRemoveLockAndWaitEx(
     PIO_REMOVE_LOCK RemoveLock,
     PVOID       Tag,
     ULONG           RemlockSize 
    )
{
    sdv_Io_Removelock_release_wait_returned = 1;
}

VOID
sdv_IoReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    )
{
    sdv_Io_Removelock_release_wait_returned = 1;
}

NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    )
{
}

NTKERNELAPI
NTSTATUS
IoReportDetectedDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements OPTIONAL,
    IN BOOLEAN ResourceAssigned,
    IN OUT PDEVICE_OBJECT *DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoReportResourceForDetection(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    OUT PBOOLEAN ConflictDetected
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_CONFLICTING_ADDRESSES;break;
    }
}

NTKERNELAPI
NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChange(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_DEVICE_REQUEST;break;
    }
}

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChangeAsynchronous(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure,
    IN PDEVICE_CHANGE_COMPLETE_CALLBACK Callback OPTIONAL,
    IN PVOID Context OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_DEVICE_REQUEST;break;
    }
}

NTKERNELAPI
VOID
IoRequestDeviceEject(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
}

NTKERNELAPI
VOID
sdv_IoRequestDpc(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    sdv_dpc_io_registered = TRUE;
#ifdef SDV_RUN_KE_DPC_ROUTINES           
    sdv_RunIoDpcRoutines(Context,DeviceObject,Irp,Context);
#endif            

}


NTKERNELAPI
VOID
IoReuseIrp(
    IN OUT PIRP Irp,
    IN NTSTATUS Status
    )
{
    Irp->IoStatus.Status=Status;
}

PDRIVER_CANCEL
sdv_IoSetCancelRoutine(
    IN PIRP pirp,
    IN PDRIVER_CANCEL CancelRoutine
    )
{
    
    PDRIVER_CANCEL r = (PDRIVER_CANCEL) pirp->CancelRoutine;
    pirp->CancelRoutine = CancelRoutine;
    return r;
}

NTKERNELAPI
VOID
sdv_IoSetCompletionRoutine(
    IN PIRP pirp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    )
{
    PIO_STACK_LOCATION irpSp;
    irpSp = IoGetNextIrpStackLocation(pirp);
    irpSp->CompletionRoutine = CompletionRoutine;
    sdv_compFset = 1;
    sdv_context = Context;
    sdv_invoke_on_success = InvokeOnSuccess;
    sdv_invoke_on_error = InvokeOnError;
    sdv_invoke_on_cancel = InvokeOnCancel;

}

NTKERNELAPI
NTSTATUS
IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    )
{
    PIO_STACK_LOCATION irpSp;
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: 
           irpSp = IoGetNextIrpStackLocation(Irp);
           irpSp->CompletionRoutine = CompletionRoutine;
           sdv_compFset = 1;
           sdv_context = Context;
           sdv_invoke_on_success = InvokeOnSuccess;
           sdv_invoke_on_error = InvokeOnError;
           sdv_invoke_on_cancel = InvokeOnCancel;
 	   return STATUS_SUCCESS;
	   break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTKERNELAPI
NTSTATUS
IoSetDeviceInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INVALID_DEVICE_REQUEST;break;
        case 2: return STATUS_INSUFFICIENT_RESOURCES;break;
        case 3: return STATUS_BUFFER_TOO_SMALL;break;
        case 4: return STATUS_OBJECT_NAME_NOT_FOUND;break;
        default: return STATUS_OBJECT_NAME_EXISTS;break;
    }
}

NTKERNELAPI
VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    )
{
}

NTKERNELAPI
VOID
sdv_IoSetNextIrpStackLocation(
    IN OUT PIRP Irp
    )
{
}

NTKERNELAPI
VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    )
{
}

VOID
IoSetStartIoAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeferredStartIo,
    IN BOOLEAN NonCancelable
    )
{
}

NTKERNELAPI
NTSTATUS
IoSetSystemPartition(
    PUNICODE_STRING VolumeNameString
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
BOOLEAN
IoSetThreadHardErrorMode(
    IN BOOLEAN EnableHardErrors
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return FALSE;break;
        default: return TRUE;break;
    }
}

NTKERNELAPI
USHORT
sdv_IoSizeOfIrp(
    IN CCHAR StackSize
    )
{
    USHORT l = (USHORT) SdvKeepChoice();
    return l;
}

NTKERNELAPI
ULONG
sdv_IoSizeofWorkItem(
    VOID
    )
{
    ULONG l = SdvKeepChoice();
    return l;
}

FORCEINLINE
VOID
sdv_IoSkipCurrentIrpStackLocation(
    IN PIRP pirp
    )
{
    SDV_MACRO_COPYCURRENTIRPSTACKLOCATIONTONEXT(pirp)
}

NTKERNELAPI
VOID
IoStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable
    )
{
#ifdef SDV_HARNESS_DRIVERSTARTIO_ROUTINE
	sdv_stub_startio_begin();
#ifdef fun_DriverStartIo
    sdv_StartIopirp->CancelRoutine = NULL;
    sdv_StartIopirp->Cancel = FALSE;
    fun_DriverStartIo(DeviceObject, sdv_StartIopirp);
#else
    sdv_DoNothing();
#endif
    sdv_stub_startio_end();
#endif
}

VOID 
IoStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable,
    IN ULONG Key
    )
{

}

NTKERNELAPI
VOID
IoStartPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PULONG Key OPTIONAL,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL
    )
{
#ifdef SDV_HARNESS_DRIVERSTARTIO_ROUTINE
	sdv_stub_startio_begin();
#ifdef fun_DriverStartIo
    sdv_StartIopirp->CancelRoutine = CancelFunction;
    sdv_StartIopirp->Cancel = FALSE;
    fun_DriverStartIo(DeviceObject, Irp);
#else
    sdv_DoNothing();
#endif
    sdv_stub_startio_end();
#endif
}

NTKERNELAPI
VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    )
{
}

NTKERNELAPI
VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    )
{
}

VOID
IoUninitializeWorkItem(
    IN PIO_WORKITEM IoWorkItem
    )
{
}

NTKERNELAPI
NTSTATUS
IoUnregisterPlugPlayNotification(
    IN PVOID NotificationEntry
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )
{
}

NTKERNELAPI
VOID
IoUpdateShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    )
{
}

NTSTATUS
IoValidateDeviceIoControlAccess(
    IN PIRP pirp,
    IN ULONG RequiredAccess
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTKERNELAPI
NTSTATUS
IoVerifyPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FixErrors
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_DISK_CORRUPT_ERROR;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoVolumeDeviceToDosName(
    IN PVOID VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIAllocateInstanceIds(
    IN GUID *Guid,
    IN ULONG InstanceCount,
    OUT ULONG *FirstInstanceId
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIDeviceObjectToInstanceName(
    IN PVOID DataBlockObject,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING InstanceName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
    }
}

NTKERNELAPI
ULONG
sdv_IoWMIDeviceObjectToProviderId(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    ULONG l = SdvKeepChoice();
    return l;
}

NTKERNELAPI
NTSTATUS
IoWMIExecuteMethod(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN OUT PULONG OutBufferSize,
    IN OUT PUCHAR InOutBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_WMI_GUID_NOT_FOUND;break;
        case 3: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
        case 4: return STATUS_WMI_ITEMID_NOT_FOUND;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIHandleToInstanceName(
    IN PVOID DataBlockObject,
    IN HANDLE FileHandle,
    OUT PUNICODE_STRING InstanceName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIOpenBlock(
    IN GUID *DataBlockGuid,
    IN ULONG DesiredAccess,
    OUT PVOID *DataBlockObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIQueryAllData(
    IN PVOID DataBlockObject,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_WMI_GUID_NOT_FOUND;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIQueryAllDataMultiple(
    IN PVOID *DataBlockObjectList,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_WMI_GUID_NOT_FOUND;break;
        case 3: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstanceMultiple(
    IN PVOID *DataBlockObjectList,
    IN PUNICODE_STRING InstanceNames,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

#if WINVER > 0x0500
NTKERNELAPI
NTSTATUS
IoWMISetNotificationCallback(
    IN PVOID Object,
    IN WMI_NOTIFICATION_CALLBACK Callback,
    IN PVOID Context
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
NTSTATUS
IoWMISetSingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_WMI_GUID_NOT_FOUND;break;
        case 3: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
        case 4: return STATUS_WMI_READ_ONLY;break;
        default: return STATUS_WMI_SET_FAILURE;break;
    }
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
NTSTATUS
IoWMISetSingleItem(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_WMI_GUID_NOT_FOUND;break;
        case 3: return STATUS_WMI_INSTANCE_NOT_FOUND;break;
        case 4: return STATUS_WMI_ITEMID_NOT_FOUND;break;
        case 5: return STATUS_WMI_READ_ONLY;break;
        default: return STATUS_WMI_SET_FAILURE;break;
    }
}
#endif

NTKERNELAPI
NTSTATUS
IoWMISuggestInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN PUNICODE_STRING SymbolicLinkName OPTIONAL,
    IN BOOLEAN CombineNames,
    OUT PUNICODE_STRING SuggestedInstanceName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_NO_MEMORY;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWMIWriteEvent(
    IN PVOID WnodeEventItem
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_BUFFER_OVERFLOW;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTKERNELAPI
VOID
IoWriteErrorLogEntry(
    IN PVOID ElEntry
    )
{
}




NTKERNELAPI
NTSTATUS
IoAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject 
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_NO_SUCH_DEVICE;break;
    }
}



NTKERNELAPI
NTSTATUS
IoCheckEaBufferValidity(
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    OUT PULONG ErrorOffset
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_EA_LIST_INCONSISTENT;break;
    }
}






NTKERNELAPI
NTSTATUS
IoCreateFileSpecifyDeviceObjectHint(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PVOID DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INVALID_DEVICE_OBJECT_PARAMETER;break;
        case 3: return STATUS_MOUNT_POINT_NOT_RESOLVED;break;
        default: return STATUS_OBJECT_PATH_SYNTAX_BAD;break;
    }
}




NTKERNELAPI
NTSTATUS
IoEnumerateDeviceObjectList(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObjectList,
    IN ULONG DeviceObjectListSize,
    OUT PULONG ActualNumberDeviceObjects
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoEnumerateRegisteredFiltersList(
    IN PDRIVER_OBJECT *DriverObjectList,
    IN ULONG DriverObjectListSize,
    OUT PULONG ActualNumberDriverObjects
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}





NTKERNELAPI
NTSTATUS
IoGetDiskDeviceObject(
    IN PDEVICE_OBJECT FileSystemDeviceObject,
    OUT PDEVICE_OBJECT *DeviceObject
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_INVALID_PARAMETER;break;
        case 1: return STATUS_VOLUME_DISMOUNTED;break;
        default: return STATUS_SUCCESS;break;
    }
}




NTKERNELAPI
NTSTATUS
IoGetRequestorSessionId(
    IN PIRP Irp,
    OUT PULONG pSessionId
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}









NTKERNELAPI
NTSTATUS
IoQueryFileDosDeviceName(
    IN PFILE_OBJECT FileObject,
    OUT POBJECT_NAME_INFORMATION *ObjectNameInformation
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}










NTKERNELAPI
NTSTATUS
IoSetFileOrigin(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Remote
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INVALID_PARAMETER_MIX;break;
    }
}







NTKERNELAPI
NTSTATUS
IoVerifyVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INSUFFICIENT_RESOURCES;break;
        case 2: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_WRONG_VOLUME;break;
    }
}






#if WINVER > 0x0500
NTKERNELAPI
NTSTATUS
IoCreateDisk(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK Disk
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}
#endif

#if WINVER > 0x0500
#ifdef SDV_Include_NTDDK
NTKERNELAPI
NTSTATUS
IoReadDiskSignature(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG BytesPerSector,
    OUT PDISK_SIGNATURE Signature
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_DISK_CORRUPT_ERROR;break;
    }
}
#endif
#endif

#if WINVER > 0x0500
DECLSPEC_DEPRECATED_DDK
#endif
NTKERNELAPI
NTSTATUS
#if WINVER > 0x0500
FASTCALL
#endif
IoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoReadPartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _DRIVE_LAYOUT_INFORMATION_EX **PartitionBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: *PartitionBuffer=&sdv_IoReadPartitionTableEx_DRIVE_LAYOUT_INFORMATION_EX;
                return STATUS_SUCCESS;
				break;
        default: *PartitionBuffer=NULL;
                 return STATUS_UNSUCCESSFUL;
				 break;
    }
}

#if WINVER > 0x0500
DECLSPEC_DEPRECATED_DDK
#endif
NTKERNELAPI
NTSTATUS
#if WINVER > 0x0500
FASTCALL
#endif
IoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTKERNELAPI
NTSTATUS
IoSetPartitionInformationEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG PartitionNumber,
    IN struct _SET_PARTITION_INFORMATION_EX *PartitionInfo
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

#if WINVER > 0x0500
DECLSPEC_DEPRECATED_DDK
#endif
NTKERNELAPI
NTSTATUS
#if WINVER > 0x0500
FASTCALL
#endif
IoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_DEVICE_NOT_READY;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTKERNELAPI
NTSTATUS
IoWritePartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _DRIVE_LAYOUT_INFORMATION_EX *PartitionBuffer
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_DEVICE_NOT_READY;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

VOID IoAcquireVpbSpinLock(
  __out  PKIRQL Irql
)
{
   *Irql=sdv_irql_current;
    
}

VOID IoReleaseVpbSpinLock(
  __in  KIRQL Irql
)
{
   SDV_IRQL_POPTO(Irql);
}

NTKERNELAPI
VOID
NTAPI
IoInitializeRemoveLockEx(
     PIO_REMOVE_LOCK Lock,
     ULONG  AllocateTag, 
     ULONG  MaxLockedMinutes, 
     ULONG  HighWatermark, 
     ULONG  RemlockSize 
    )
{
}


NTSTATUS
IoGetDeviceNumaNode (
     PDEVICE_OBJECT Pdo,
     PUSHORT NodeNumber
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


#ifdef DEVPROPKEY_DEFINED
NTSTATUS
IoGetDevicePropertyData (
     PDEVICE_OBJECT     Pdo,
     const DEVPROPKEY   *PropertyKey,
     LCID               Lcid,
     ULONG        Flags,
     ULONG              Size,
     PVOID             Data,
     PULONG            RequiredSize,
     PDEVPROPTYPE      Type
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
NTKERNELAPI
NTSTATUS
NTAPI
IoReplacePartitionUnit (
     PDEVICE_OBJECT TargetPdo,
     PDEVICE_OBJECT SparePdo,
     ULONG Flags
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

#ifdef DEVPROPKEY_DEFINED
NTSTATUS
IoSetDevicePropertyData (
     PDEVICE_OBJECT     Pdo,
     const DEVPROPKEY   *PropertyKey,
     LCID               Lcid,
     ULONG              Flags,
     DEVPROPTYPE        Type,
     ULONG              Size,
     PVOID          Data
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif
NTSTATUS
IoUnregisterPlugPlayNotificationEx(
     PVOID NotificationEntry
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


VOID
IoSetShareAccessEx(
      ACCESS_MASK DesiredAccess,
      ULONG DesiredShareAccess,
     PFILE_OBJECT FileObject,
     PSHARE_ACCESS ShareAccess,
     PBOOLEAN WritePermission
    )
{
}


/* ntddk-io.c end */

/* ntddk-ke.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

#if WINVER > 0x0500
_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockForDpc(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}
#endif

KIRQL
KeAcquireInterruptSpinLock(
    IN PKINTERRUPT Interrupt
    )
{
    SDV_IRQL_PUSH(SDV_DIRQL);
    return sdv_irql_previous;
}

VOID
sdv_KeAcquireSpinLock(
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL p_old_irql
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    (*p_old_irql) = sdv_irql_previous;
}


VOID
sdv_KeAcquireSpinLockAtDpcLevel(
    IN PKSPIN_LOCK  SpinLock
    ) 
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}


NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc(
    IN PKSPIN_LOCK SpinLock
    ) 
{ 
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    return sdv_irql_previous;
}

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheck (
    __in ULONG BugCheckCode
    )
{
    /* Simulate bug check by stopping verification: */
    SdvExit();
}

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheckEx(
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3,
    __in ULONG_PTR BugCheckParameter4
    )
{
    /* Simulate bug check by stopping verification: */
    SdvExit();
}

NTKERNELAPI
VOID
KeClearEvent(
    __inout PRKEVENT Event
    ) 
{
    Event->Header.SignalState = 0;
    return;
}

NTKERNELAPI                                         
NTSTATUS                                            
KeDelayExecutionThread(                            
    IN KPROCESSOR_MODE WaitMode,                    
    IN BOOLEAN Alertable,                           
    IN PLARGE_INTEGER Interval                      
    )
{
    int x = SdvMakeChoice();
    switch (x) {
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_ALERTED;break;
		case 2: return STATUS_USER_APC;break;
#endif
        default: return STATUS_UNSUCCESSFUL;break;

  }
}

NTKERNELAPI
VOID
KeEnterCriticalRegion(
    VOID
    )
{
}

NTKERNELAPI
VOID
sdv_KeFlushIoBuffers(
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    )
{
}


NTHALAPI
KIRQL
sdv_KeGetCurrentIrql(void) 
{
    return sdv_irql_current;
}


NTKERNELAPI
VOID
KeInitializeDpc(
    PRKDPC Dpc,
    PKDEFERRED_ROUTINE DeferredRoutine,
    PVOID DeferredContext
    ) 
{
   Dpc->DeferredRoutine = DeferredRoutine;
}

NTKERNELAPI
VOID
KeInitializeThreadedDpc(
    PRKDPC Dpc,
    PKDEFERRED_ROUTINE DeferredRoutine,
    PVOID DeferredContext
    ) 
{
   Dpc->DeferredRoutine = DeferredRoutine;
}


NTKERNELAPI
VOID
KeInitializeEvent(
    OUT PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    )
{ 
     Event->Header.Type = (UCHAR)Type;
     Event->Header.Signalling = FALSE;
     Event->Header.Size = sizeof(KEVENT) / sizeof(LONG);
     Event->Header.SignalState = State;
}

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue(
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue(
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTKERNELAPI
BOOLEAN
KeInsertQueueDpc(
    OUT PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            sdv_kdpc3=Dpc;
            sdv_dpc_ke_registered=TRUE;
#ifdef SDV_RUN_KE_DPC_ROUTINES           
            sdv_RunKeDpcRoutines(sdv_kdpc3,sdv_pDpcContext,SystemArgument1,SystemArgument2);
#endif            
            return TRUE;
            break;
    }
}
NTKERNELAPI
VOID
KeLeaveCriticalRegion(
    VOID
    )
{
}

_IRQL_requires_max_(HIGH_LEVEL)
#if defined(_ARM_)
NTHALAPI
VOID
#else
_DECL_HAL_KE_IMPORT
VOID
FASTCALL
#endif
KfLowerIrql (
    _In_ _IRQL_restores_ _Notliteral_ KIRQL NewIrql
    ) 
{ 
    SDV_IRQL_POPTO(NewIrql);
}

VOID
sdv_KeLowerIrql (
         KIRQL NewIrql
   )
{
    SDV_IRQL_POPTO(NewIrql);
}



NTKERNELAPI
LONG
KePulseEvent(
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    )
{
    LONG l = SdvKeepChoice();
    Event->Header.SignalState = 0;
    return l;
}

VOID
sdv_KeRaiseIrql(
    IN KIRQL new_irql,
    OUT PKIRQL p_old_irql
    ) 
{  
    SDV_IRQL_PUSH(new_irql);
    *p_old_irql = sdv_irql_previous;
}


KIRQL
sdv_KeRaiseIrqlToDpcLevel(
    VOID
    ) 
{ 
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    return sdv_irql_previous;
}

KIRQL
sdv_KeRaiseIrqlToSynchLevel(
    VOID
    ) 
{ 
    KIRQL r = (KIRQL) SdvMakeChoice(); 
    return r; 
}

NTKERNELAPI
LONG
KeReadStateEvent(
    IN PRKEVENT Event
    )
{
    LONG l = SdvKeepChoice();
    return l;
}

#if WINVER > 0x0500
_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock(
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_POP();
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockForDpc(
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_POP();
}
#endif

#if WINVER > 0x0500
NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel(
    IN PKLOCK_QUEUE_HANDLE LockHandle
    ) 
{
    SDV_IRQL_POP();
}
#endif

VOID
KeReleaseInterruptSpinLock(
    IN PKINTERRUPT Interrupt,
    IN KIRQL old_irql
    )
{
    SDV_IRQL_POPTO(old_irql);
}

NTKERNELAPI
LONG
KeReleaseSemaphore(
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    ) 
{
    LONG r = SdvKeepChoice(); 
    return r;
}


VOID
sdv_KeReleaseSpinLock(
    IN PKSPIN_LOCK  SpinLock,
    IN KIRQL  new_irql
    ) 
{
    SDV_IRQL_POPTO(new_irql);
}

sdv_ExReleaseSpinLock(
    IN PKSPIN_LOCK  SpinLock,
    IN KIRQL  new_irql
    ) 
{
    SDV_IRQL_POPTO(new_irql);
}

VOID
sdv_KeReleaseSpinLockFromDpcLevel(
    IN PKSPIN_LOCK  SpinLock
    ) 
{
    SDV_IRQL_POP();
}


NTKERNELAPI
LONG
KeResetEvent(
    IN PRKEVENT Event
    ) 
{
    LONG OldState;
    OldState = Event->Header.SignalState;
    Event->Header.SignalState = 0;
    return OldState;
}

NTKERNELAPI
LONG
KeSetEvent(
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    ) 
{
    LONG OldState;
    OldState = Event->Header.SignalState;
    Event->Header.SignalState = 1;
    return OldState;
}

BOOLEAN
KeSynchronizeExecution(
    PKINTERRUPT Interrupt,
    PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID SynchronizeContext
    )
{
    BOOLEAN b;

    /* SynchronizeRoutine must be non-null.  Check anyhow. */
    /*if ( SynchronizeRoutine == NULL )
    {
        return FALSE;
    }*/
#ifdef SDV_HARNESS_RUN_KSYNC_ROUTINES
    SDV_IRQL_PUSH(SDV_DIRQL);
    b=sdv_RunKSynchronizeRoutines(SynchronizeRoutine,SynchronizeContext);
    SDV_IRQL_POP();
#else
    b=OneOfTwoBOOLEAN();
#endif
    return b;
}

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects(
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    )  
{
    int x = SdvMakeChoice();
      
    switch (x) { 
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_ALERTED;break;
        case 2: return STATUS_USER_APC;break;
        
#else
        default: return STATUS_TIMEOUT;break;
#endif
    }
}

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject(
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{

    int x = SdvMakeChoice();
    if(Timeout==NULL)
    {
        return STATUS_SUCCESS;
    }
    switch (x) 
    {
        case 0: return STATUS_TIMEOUT;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_ALERTED;break;
        case 2: return STATUS_USER_APC;break;
#else
        default: return STATUS_SUCCESS;break;
#endif
    }
}

NTSTATUS sdv_KeWaitForMutexObject(
  __in      PVOID Mutex,
  __in      KWAIT_REASON WaitReason,
  __in      KPROCESSOR_MODE WaitMode,
  __in      BOOLEAN Alertable,
  __in_opt  PLARGE_INTEGER Timeout
)
{

    int x = SdvMakeChoice();
    if(Timeout==NULL)
    {
        return STATUS_SUCCESS;
    }
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_ALERTED;break;
        case 2: return STATUS_USER_APC;break;
        default: return STATUS_TIMEOUT;break;
#else
        default: return STATUS_TIMEOUT;break;
#endif
    }
}





BOOLEAN  
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel(
    IN PKSPIN_LOCK  SpinLock)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            SDV_IRQL_PUSH(DISPATCH_LEVEL);
            return TRUE;
            break;
    }
}


VOID 
FASTCALL
KeReleaseSpinLockForDpc(
  __inout  PKSPIN_LOCK SpinLock,
  __in     KIRQL OldIrql
)
{
    SDV_IRQL_POPTO(OldIrql);
}

KIRQL 
FASTCALL
KeAcquireSpinLockForDpc(
  __inout  PKSPIN_LOCK SpinLock
)
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    return sdv_irql_previous;
}

LONG KeReleaseMutex(
  __inout  PRKMUTEX Mutex,
  __in     BOOLEAN Wait
)
{
    return 0;
}

VOID 
FASTCALL
KeAcquireGuardedMutex(
  __inout  PKGUARDED_MUTEX Mutex
)
{

}
VOID 
FASTCALL
KeAcquireGuardedMutexUnsafe(
  __inout  PKGUARDED_MUTEX FastMutex
)
{

}
VOID KeEnterGuardedRegion(void)
{

}
VOID 
FASTCALL
KeInitializeGuardedMutex(
  __out  PKGUARDED_MUTEX Mutex
)
{

}

VOID KeInitializeMutex(
  __out  PRKMUTEX Mutex,
  __in   ULONG Level
)
{

}

VOID KeInitializeSemaphore(
  __out  PRKSEMAPHORE Semaphore,
  __in   LONG Count,
  __in   LONG Limit
)
{

}

NTKERNELAPI
VOID
NTAPI
sdv_KeInitializeSpinLock(
  __out  PKSPIN_LOCK SpinLock
)
{
    *SpinLock = 0;
    return;
}

VOID KeLeaveGuardedRegion(void)
{

}
LONG KeReadStateMutex(
  __in  PRKMUTEX Mutex
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return 1;
            break;
        default:
            return 0;
            break;
    }
}
LONG KeReadStateSemaphore(
  __in  PRKSEMAPHORE Semaphore
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return 1;
            break;
        default:
            return 0;
            break;
    }

}
VOID 
FASTCALL
KeReleaseGuardedMutex(
  __inout  PKGUARDED_MUTEX Mutex
)
{

}
VOID 
FASTCALL
KeReleaseGuardedMutexUnsafe(
  __inout  PKGUARDED_MUTEX FastMutex
)
{

}

BOOLEAN 
FASTCALL
KeTestSpinLock(
  __in  PKSPIN_LOCK SpinLock
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}
BOOLEAN 
FASTCALL
KeTryToAcquireGuardedMutex(
  __inout  PKGUARDED_MUTEX Mutex
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN KeRemoveEntryDeviceQueue(
  __inout  PKDEVICE_QUEUE DeviceQueue,
  __inout  PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

#if defined (_X86_)
NTSTATUS KeSaveFloatingPointState(
  __out  PKFLOATING_SAVE FloatSave
)
{
    int x = SdvMakeChoice();

    switch (x) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_ILLEGAL_FLOAT_CONTEXT;break;
        case 2: return STATUS_INSUFFICIENT_RESOURCES;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}
#endif

BOOLEAN KeSetTimer(
  __inout   PKTIMER Timer,
  __in      LARGE_INTEGER DueTime,
  __in_opt  PKDPC Dpc
)
{
   LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN KeSetTimerEx(
  __inout   PKTIMER Timer,
  __in      LARGE_INTEGER DueTime,
  __in      LONG Period,
  __in_opt  PKDPC Dpc
)
{
   LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN KeAreAllApcsDisabled(void)
{
   LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

BOOLEAN KeAreApcsDisabled(void)
{
   LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }

}
NTSTATUS KeDeregisterNmiCallback(
  __in  PVOID Handle
)
{
    int x = SdvMakeChoice();

    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INVALID_HANDLE;break;
    }
}

PVOID KeRegisterNmiCallback(
  __in      PNMI_CALLBACK CallbackRoutine,
  __in_opt  PVOID Context
)
{
    int choice = SdvMakeChoice();
    PVOID p;
    switch (choice) 
    {
        case 0: p= (PVOID) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }

}

KAFFINITY KeQueryActiveProcessors(void)
{
     return (KAFFINITY)SdvKeepChoice();

}

PKDEVICE_QUEUE_ENTRY KeRemoveByKeyDeviceQueue(
  __inout  PKDEVICE_QUEUE DeviceQueue,
  __in     ULONG SortKey
)
{
    int choice = SdvMakeChoice();
    PKDEVICE_QUEUE_ENTRY p;
    switch (choice) 
    {
        case 0: p= (PKDEVICE_QUEUE_ENTRY) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}


PKDEVICE_QUEUE_ENTRY KeRemoveDeviceQueue(
  __inout  PKDEVICE_QUEUE DeviceQueue
)
{
    return NULL;
}

BOOLEAN KeCancelTimer(
  __inout  PKTIMER Timer
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

VOID KeInitializeDeviceQueue(
  __out  PKDEVICE_QUEUE DeviceQueue
)
{
    PKDEVICE_QUEUE x = malloc(1);
    *DeviceQueue = *x;
}


VOID KeInitializeTimer(
  __out  PKTIMER Timer
)
{

}

VOID KeInitializeTimerEx(
  __out  PKTIMER Timer,
  __in   TIMER_TYPE Type
)
{
}


BOOLEAN KeReadStateTimer(
  __in  PKTIMER Timer
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    __inout __deref __drv_releasesExclusiveResource(KeSpinLockType)
    PKSPIN_LOCK SpinLock
    )
{
    SDV_IRQL_POP();
}


NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    __inout __deref __drv_acquiresExclusiveResource(KeSpinLockType)
    PKSPIN_LOCK SpinLock
    )
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
}


VOID
FASTCALL
KfReleaseSpinLock (
    __inout __deref __drv_releasesExclusiveResource(KeSpinLockType)
    PKSPIN_LOCK SpinLock,
    __in __drv_restoresIRQL KIRQL NewIrql
    )
{
    SDV_IRQL_POPTO(NewIrql); 
}


KIRQL
FASTCALL
KfAcquireSpinLock (
    __inout __deref __drv_acquiresExclusiveResource(KeSpinLockType)
    PKSPIN_LOCK SpinLock
    )
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    return  sdv_irql_previous;
}




BOOLEAN KeRemoveQueueDpc(
  __inout  PRKDPC Dpc
)
{
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return TRUE;break;
        default:return  FALSE;break;
  }
}


VOID KeFlushQueuedDpcs(void)
{
}

#if defined (_X86_)
KIRQL
FASTCALL
KfRaiseIrql (
    __in KIRQL NewIrql
    )
{
    SDV_IRQL_PUSH(NewIrql);
    return sdv_irql_previous;
}
#endif

ULONG 
  sdv_KeGetCurrentProcessorNumber(VOID)
{
  return (ULONG) SdvKeepChoice();
}


LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock (
    KSPIN_LOCK_QUEUE_NUMBER Number,
    PKIRQL OldIrql
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            SDV_IRQL_PUSH(DISPATCH_LEVEL);
            (*OldIrql) = sdv_irql_previous;
            return TRUE;
            break;
    }

}

VOID
FASTCALL
KeReleaseQueuedSpinLock (
     KSPIN_LOCK_QUEUE_NUMBER Number,
     KIRQL OldIrql
    )
{
    SDV_IRQL_POPTO(OldIrql);
}

LONG
KeReleaseMutant (
     PRKMUTANT Mutant,
     KPRIORITY Increment,
     BOOLEAN Abandoned,
     BOOLEAN Wait
    )
{
    return (LONG)SdvKeepChoice();
}

KIRQL
FASTCALL
KeAcquireQueuedSpinLock (
      
     KSPIN_LOCK_QUEUE_NUMBER Number
    )
{
    SDV_IRQL_PUSH(DISPATCH_LEVEL);
    return sdv_irql_previous;
}

KPRIORITY
KeQueryPriorityThread (
     PKTHREAD Thread
    ) 
{
    return (LONG)SdvKeepChoice();
}

ULONG
KeQueryRuntimeThread (
     PKTHREAD Thread,
     PULONG UserTime
    ) 
{
    return (ULONG)SdvKeepChoice();
}

VOID
KeRevertToUserAffinityThreadEx (
     KAFFINITY Affinity
    ) 
{
}

VOID
KeSetSystemAffinityThread (
     KAFFINITY Affinity
    ) 
{
}

VOID
KeSetSystemGroupAffinityThread (
     PGROUP_AFFINITY Affinity,
     PGROUP_AFFINITY PreviousAffinity
    ) 
{
}

NTKERNELAPI
VOID
KeInitializeMutant (
    __out PRKMUTANT Mutant,
    __in BOOLEAN InitialOwner
    )
{
}

#if defined(_M_AMD64)

__forceinline
PKTHREAD
sdv_KeGetCurrentThread (
    VOID
    )

{
    return (PKTHREAD)malloc(4);
}

#endif

#if defined(_M_IX86) || defined(_M_ARM)

NTSYSAPI
PKTHREAD
NTAPI
sdv_KeGetCurrentThread(
    VOID
    )
{
    return (PKTHREAD)malloc(4);
}

#endif

/* ntddk-mm.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTKERNELAPI
PVOID
MmAllocateContiguousMemory(
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    ) 
{
    PVOID p;
    if(NumberOfBytes>0)
    {
       p=(PVOID)malloc(NumberOfBytes);
    }
    else
    {
        p=NULL;
    }
}

ULONG
sdv_MmGetMdlByteCount(
    IN PMDL Mdl
     )
{
    /* Suppress C4311: 'type cast' : pointer truncation .
       This is acceptable as the OS Model will not be executed:*/
    #pragma warning(disable:4305)
    ULONG x = ((ULONG)malloc(1));
    #pragma warning(default:4305)
    return  x;
}

ULONG
sdv_MmGetMdlByteOffset (
    IN PMDL Mdl
    )
{
    /* Suppress C4311: 'type cast' : pointer truncation .
       This is acceptable as the OS Model will not be executed:*/
    #pragma warning(disable:4305)
    ULONG x = ((ULONG)malloc(1));
    #pragma warning(default:4305)
    return x;
}


PPFN_NUMBER
sdv_MmGetMdlPfnArray(
    IN PMDL Mdl
    )
{
	PPFN_NUMBER x=(PPFN_NUMBER) malloc(1);
	return x;
}


PVOID
sdv_MmGetMdlVirtualAddress(
    IN PMDL Mdl
     )
{
	PVOID x= (PVOID) malloc(1); 
	return x;
}

PVOID
sdv_MmGetSystemAddressForMdlSafe(
     IN PMDL MDL,
     IN MM_PAGE_PRIORITY PRIORITY
     ) 
{
    int choice = SdvMakeChoice();
    PVOID p;
    switch (choice) 
    {
        case 0: p= (PVOID) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}
PVOID
sdv_MmLockPagableCodeSection(
    IN PVOID  AddressWithinSection
    ) 
{
    return(AddressWithinSection);
}

NTKERNELAPI
VOID
MmLockPagableSectionByHandle(
    IN PVOID ImageSectionHandle
    )
{
}

int sdv_MmMapIoSpace_int=0;  

NTKERNELAPI
PVOID
MmMapIoSpace(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    )
{
    return (PVOID) &sdv_MmMapIoSpace_int;
}

NTKERNELAPI
PVOID
MmMapLockedPages(
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    ) 
{
	PVOID r= (PVOID) malloc(1);
	return r;
}

NTKERNELAPI
PVOID
sdv_MmMapLockedPagesSpecifyCache(
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseAddress,
     IN ULONG BugCheckOnFailure,
     IN ULONG Priority
     ) 
{
	PVOID r= (PVOID) malloc(1);
	return r;
}


NTKERNELAPI
PVOID
MmPageEntireDriver(
    IN PVOID AddressWithinSection
    )
{
	PVOID p= (PVOID) malloc(1);
	return p;
}

VOID
sdv_MmPrepareMdlForReuse(
    IN PMDL Mdl
    )
{
}

NTKERNELAPI
VOID
MmResetDriverPaging(
    IN PVOID AddressWithinSection
    )
{
}

NTKERNELAPI
VOID
MmUnlockPagableImageSection(
    IN PVOID ImageSectionHandle
    )
{
}

ULONG MmDoesFileHaveUserWritableReferences(
  __in  PSECTION_OBJECT_POINTERS SectionPointer
)
{
    return (ULONG)SdvKeepChoice();
}

PVOID MmLockPagableDataSection(
  __in  PVOID AddressWithinSection
)
{
  PVOID r= (PVOID) malloc(1);
	return r;
}

PVOID MmMapLockedPagesWithReservedMapping(
  __in  PVOID MappingAddress,
  __in  ULONG PoolTag,
  __in  PMDLX MemoryDescriptorList,
  __in  MEMORY_CACHING_TYPE CacheType
)
{
    LONG choice = SdvMakeChoice();
    PVOID p;
    switch (choice) 
    {
        case 0: p= (PVOID) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}

VOID MmProbeAndLockPages(
  __inout  PMDLX MemoryDescriptorList,
  __in     KPROCESSOR_MODE AccessMode,
  __in     LOCK_OPERATION Operation
)
{
}

NTKERNELAPI
VOID
MmProbeAndLockProcessPages (
    __inout PMDL MemoryDescriptorList,
    __in PEPROCESS Process,
    __in KPROCESSOR_MODE AccessMode,
    __in LOCK_OPERATION Operation
    )
{
}

VOID MmUnlockPages(
  __inout  PMDLX MemoryDescriptorList
)
{
}

VOID MmUnmapLockedPages(
  __in  PVOID BaseAddress,
  __in  PMDL MemoryDescriptorList
)
{
}

VOID MmFreeContiguousMemory(
  __in  PVOID BaseAddress
)
{

}

PVOID MmAllocateNonCachedMemory(
  __in  SIZE_T NumberOfBytes
)
{
    LONG choice = SdvMakeChoice();
    PVOID p;
    switch (choice) 
    {
        case 0: p= (PVOID) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}

PMDL MmAllocatePagesForMdl(
  __in  PHYSICAL_ADDRESS LowAddress,
  __in  PHYSICAL_ADDRESS HighAddress,
  __in  PHYSICAL_ADDRESS SkipBytes,
  __in  SIZE_T TotalBytes
)
{
    LONG choice = SdvMakeChoice();
    PMDL p;
    switch (choice) 
    {
        case 0: p= (PMDL) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}

HANDLE MmSecureVirtualMemory(
  __in  PVOID Address,
  __in  SIZE_T Size,
  __in  ULONG ProbeMode
)
{
    LONG choice = SdvMakeChoice();
    HANDLE p;
    switch (choice) 
    {
        case 0: p= (HANDLE) malloc(1);
                return p;
                break;
        default:return NULL;
                break;
    }
}

VOID MmFreeNonCachedMemory(
  __in  PVOID BaseAddress,
  __in  SIZE_T NumberOfBytes
)
{
}

VOID MmFreePagesFromMdl(
  __in  PMDLX MemoryDescriptorList
)
{
}

VOID MmUnsecureVirtualMemory(
  __in  HANDLE SecureHandle
)
{
}

#ifdef SDV_Include_NTDDK
NTSTATUS
MmAddPhysicalMemory (
     PPHYSICAL_ADDRESS StartAddress,
     PLARGE_INTEGER NumberOfBytes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif

NTSTATUS
MmCreateMirror (
    void
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

#ifdef SDV_Include_NTDDK

PPHYSICAL_MEMORY_RANGE
MmGetPhysicalMemoryRanges (
    void
    ) 
{
    return (PPHYSICAL_MEMORY_RANGE)malloc(1);
}

#endif
PVOID
MmGetSystemRoutineAddress (
     PUNICODE_STRING SystemRoutineName
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return (PVOID)malloc(1); break;
        default: return NULL; break;
    }
}


NTSTATUS
MmRemovePhysicalMemory (
     PPHYSICAL_ADDRESS StartAddress,
     PLARGE_INTEGER NumberOfBytes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

PVOID MmAllocateContiguousMemorySpecifyCache(
  __in      SIZE_T NumberOfBytes,
  __in      PHYSICAL_ADDRESS LowestAcceptableAddress,
  __in      PHYSICAL_ADDRESS HighestAcceptableAddress,
  __in_opt  PHYSICAL_ADDRESS BoundaryAddressMultiple,
  __in      MEMORY_CACHING_TYPE CacheType
)
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return (PVOID)malloc(1); break;
        default: return NULL; break;
    }
}

PMDL MmAllocatePagesForMdlEx(
  __in  PHYSICAL_ADDRESS LowAddress,
  __in  PHYSICAL_ADDRESS HighAddress,
  __in  PHYSICAL_ADDRESS SkipBytes,
  __in  SIZE_T TotalBytes,
  __in  MEMORY_CACHING_TYPE CacheType,
  __in  ULONG Flags
)
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return (PMDL)malloc(1); break;
        default: return NULL; break;
    }

}


PMDL MmCreateMdl(
    IN PMDL  MemoryDescriptorList  OPTIONAL,
    IN PVOID  Base,
    IN SIZE_T  Length
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return (PMDL)malloc(1); break;
        default: return NULL; break;
    }

}

VOID MmUnmapIoSpace(
  __in  PVOID BaseAddress,
  __in  SIZE_T NumberOfBytes
)
{
}


/*void * sdv_memcpy(
   void *dest,
   const void *src,
   size_t count 
)
{
    return dest;
}*/

/* ntddk-mm.c end */

/* ntddk-nc.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTSTATUS
NTAPI
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}

#ifdef SDV_Include_NTDDK
NTSTATUS
NTAPI
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}
#endif/* ntddk-nc.c end */

/* ntddk-ob.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

LONG_PTR
sdv_ObDereferenceObject (
    IN PVOID Object
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
    return p;
}

NTSTATUS
ObGetObjectSecurity(
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    ) 
{ 
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INSUFFICIENT_RESOURCES ;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;

    }
}

LONG_PTR
sdv_ObReferenceObject(
    IN PVOID Object
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
    return p;
}

NTKERNELAPI                                                     
NTSTATUS                                                        
ObReferenceObjectByHandle(                                      
    IN HANDLE Handle,                                           
    IN ACCESS_MASK DesiredAccess,                               
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PVOID *Object,                                          
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
    )
{
    int x = SdvMakeChoice();
    switch (x) { 
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_HANDLE;break;
#else
        default: return STATUS_UNSUCCESSFUL;break;
#endif
    }
}          

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    ) 
{ 
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_OBJECT_TYPE_MISMATCH ;break;
    }
}

VOID
ObReleaseObjectSecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    ) 
{  
}

NTSTATUS ObReferenceObjectByPointerWithTag(
  __in      PVOID Object,
  __in      ACCESS_MASK DesiredAccess,
  __in_opt  POBJECT_TYPE ObjectType,
  __in      KPROCESSOR_MODE AccessMode,
  __in      ULONG Tag
)
{
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_OBJECT_TYPE_MISMATCH ;break;
    }
}

NTSTATUS ObReferenceObjectByHandleWithTag(
  __in       HANDLE Handle,
  __in       ACCESS_MASK DesiredAccess,
  __in_opt   POBJECT_TYPE ObjectType,
  __in       KPROCESSOR_MODE AccessMode,
  __in       ULONG Tag,
  __out      PVOID *Object,
  __out_opt  POBJECT_HANDLE_INFORMATION HandleInformation
)
{
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_ACCESS_DENIED;break;
        case 2: return STATUS_INVALID_HANDLE;break;
        default: return STATUS_OBJECT_TYPE_MISMATCH ;break;
    }
}


LONG_PTR
FASTCALL
ObfReferenceObject(
     PVOID Object
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
}

VOID sdv_ObDereferenceObjectWithTag(
  __in  PVOID Object,
  __in  ULONG Tag
)
{
}


VOID sdv_ObReferenceObjectWithTag(
  __in  PVOID Object,
  __in  ULONG Tag
)
{
}


LONG_PTR
FASTCALL
ObfDereferenceObject(
     PVOID Object
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
}

LONG_PTR
FASTCALL
ObfDereferenceObjectWithTag(
     PVOID Object,
     ULONG Tag
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
}

LONG_PTR
FASTCALL
ObfReferenceObjectWithTag(
     PVOID Object,
     ULONG Tag
    )
{
    LONG_PTR p = (LONG_PTR) SdvKeepChoice();
}

NTSTATUS ObRegisterCallbacks(
  POB_CALLBACK_REGISTRATION CallBackRegistration,
  PVOID *RegistrationHandle
)
{
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_ACCESS_DENIED;break;
        case 2: return STATUS_INVALID_PARAMETER;break;
        case 3: return STATUS_FLT_INSTANCE_ALTITUDE_COLLISION;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

/* ntddk-ob.c end */

/* ntddk-po.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTKERNELAPI
NTSTATUS
PoCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    int choice;
    BOOLEAN completion_routine_called=FALSE;
    NTSTATUS status;
    status=STATUS_PENDING;
    choice= SdvMakeChoice();
    switch (choice) 
    {
      case 0:
      Irp->IoStatus.Status = STATUS_SUCCESS;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_SUCCESS;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_success&&sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      case 1:
      Irp->IoStatus.Status = STATUS_CANCELLED;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_CANCELLED;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_cancel&&sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      case 3:
      Irp->IoStatus.Status = STATUS_PENDING;
      Irp->PendingReturned = 1;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_PENDING;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
      default:
      Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
      Irp->PendingReturned = 0;
      if(sdv_IoBuildDeviceIoControlRequest_irp==Irp)
      {
        sdv_IoBuildDeviceIoControlRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      if(sdv_IoBuildSynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildSynchronousFsdRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      if(sdv_IoBuildAsynchronousFsdRequest_irp==Irp)
      {
        sdv_IoBuildAsynchronousFsdRequest_IoStatusBlock->Status=STATUS_UNSUCCESSFUL;
      }
      #ifdef SDV_HARNESS_COMPLETION_ROUTINE
      if(sdv_invoke_on_error&&sdv_compFset)
      {
          sdv_RunIoCompletionRoutines(sdv_p_devobj_fdo, Irp, sdv_context,&completion_routine_called);
      }
      #endif
      break;
   }
   return status;
}


NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection(
    IN PDEVICE_OBJECT     DeviceObject,
    IN ULONG              ConservationIdleTime,
    IN ULONG              PerformanceIdleTime,
    IN DEVICE_POWER_STATE State
    ) 
{ 
    PULONG r = (PULONG) malloc(1);
    return r; 
}

NTKERNELAPI
PVOID
PoRegisterSystemState(
    IN PVOID StateHandle,
    IN EXECUTION_STATE Flags
    ) 
{ 
    PVOID r= (PVOID) malloc(1);
    return r; 
}

NTKERNELAPI
NTSTATUS
PoRequestPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PREQUEST_POWER_COMPLETE CompletionFunction,
    IN PVOID Context,
    OUT PIRP *Irp OPTIONAL
    ) 
{
    PIO_STACK_LOCATION sdv_power_ps;
    LONG x = SdvMakeChoice();
    BOOLEAN powercompletionrun=FALSE;
    NTSTATUS status;
    if((MinorFunction != IRP_MN_QUERY_POWER)&&(MinorFunction != IRP_MN_SET_POWER)&&(MinorFunction != IRP_MN_WAIT_WAKE))
    {
        sdv_power_irp->IoStatus.Status = STATUS_INVALID_PARAMETER_2;
        sdv_power_irp->PendingReturned = 0;
        return STATUS_INVALID_PARAMETER_2;
    }
    else
    {
      switch (x) 
      {
          case 0:
             sdv_power_irp->IoStatus.Status = STATUS_PENDING;
             status=STATUS_PENDING;
             sdv_power_irp->PendingReturned = 1;
             #if ((SDV_HARNESS==SDV_SMALL_START_SEQUENCE_HARNESS)||(SDV_HARNESS==SDV_SMALL_POWERDOWN_HARNESS)||(SDV_HARNESS==SDV_SMALL_SMALL_POWERUP_HARNESS)||(SDV_HARNESS==SDV_FLAT_DISPATCH_HARNESS_WITH_PNP_LINKED_CALLBACKS_WITHOUT_REMOVE_DEVICE))
                 sdv_power_irp->CancelRoutine = NULL;
                 sdv_power_irp->Cancel = FALSE;
                 sdv_power_ps = SDV_MACRO_IOGETCURRENTIRPSTACKLOCATION(sdv_power_irp);
                 sdv_power_ps->MinorFunction = MinorFunction;
                 sdv_power_ps->CompletionRoutine = NULL;
                 sdv_power_ps->MajorFunction = IRP_MJ_POWER;
                 sdv_power_ps->Parameters.Power.Type = DevicePowerState;
                 sdv_power_ps->Parameters.Power.State.DeviceState = PowerState.DeviceState;
                 status=fun_IRP_MJ_POWER(sdv_p_devobj_fdo,sdv_power_irp);
             #endif
             #ifdef SDV_HARNESS_POWER_COMPLETION_ROUTINE
                  powercompletionrun=sdv_RunPowerCompletionRoutines(DeviceObject,MinorFunction,PowerState,Context,&sdv_power_irp->IoStatus,CompletionFunction);
              #endif
              return STATUS_PENDING;break;
         default: 
            sdv_power_irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            sdv_power_irp->PendingReturned = 0;
            return STATUS_INSUFFICIENT_RESOURCES;break;
      }
    }
}

NTKERNELAPI
POWER_STATE
PoSetPowerState(
    IN PDEVICE_OBJECT   DeviceObject,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE      State
    ) 
{ 
    POWER_STATE r; 
    return r; 
}

NTKERNELAPI
VOID
PoSetSystemState(
    IN EXECUTION_STATE Flags
    ) 
{  
}

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    ) 
{  
}

NTKERNELAPI
VOID
PoUnregisterSystemState(
    PVOID StateHandle
    ) 
{  
}

#if (NTDDI_VERSION >= NTDDI_WIN8)

NTKERNELAPI
NTSTATUS
sdv_PoFxRegisterDevice (
    PDEVICE_OBJECT Pdo,
    PPO_FX_DEVICE Device,
    POHANDLE *Handle
    )
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0: p_sdv_fx_dev_object=Device;
            return STATUS_SUCCESS; break;
    default: return STATUS_INSUFFICIENT_RESOURCES; break;
  } 
}

NTKERNELAPI
VOID
sdv_PoFxStartDevicePowerManagement(
    POHANDLE Handle
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxUnregisterDevice(
    POHANDLE Handle
    )
{
#if ( !SDV_IS_FLAT_SIMPLE_HARNESS() )
    p_sdv_fx_dev_object = &sdv_fx_dev_object;
#endif
}

NTKERNELAPI
VOID
sdv_PoFxActivateComponent (
    POHANDLE Handle,
    ULONG Component,
    ULONG Flags
    )
{
#if ( !SDV_IS_FLAT_SIMPLE_HARNESS() )
    if(Flags&PO_FX_FLAG_BLOCKING)
    {
      #ifdef fun_PO_FX_COMPONENT_ACTIVE_CONDITION_CALLBACK
        fun_PO_FX_COMPONENT_ACTIVE_CONDITION_CALLBACK(p_sdv_fx_dev_object->DeviceContext,Component);
      #endif
    }
#endif
}

NTKERNELAPI
VOID
sdv_PoFxIdleComponent (
    POHANDLE Handle,
    ULONG Component,
    ULONG Flags
    )
{
#if ( !SDV_IS_FLAT_SIMPLE_HARNESS() )
    if(Flags&PO_FX_FLAG_BLOCKING)
    {
      #ifdef fun_PO_FX_COMPONENT_IDLE_CONDITION_CALLBACK
          fun_PO_FX_COMPONENT_IDLE_CONDITION_CALLBACK(Context,sdv_PoRuntime_Component);
      #endif
    }
#endif
}

NTKERNELAPI
VOID
sdv_PoFxSetComponentLatency (
    POHANDLE Handle,
    ULONG Component,
    ULONGLONG Latency
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxSetComponentResidency (
    POHANDLE Handle,
    ULONG Component,
    ULONGLONG Residency
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxSetComponentWake (
    POHANDLE Handle,
    ULONG Component,
    BOOLEAN WakeHint
    )
{
}





NTKERNELAPI
VOID
sdv_PoFxReportDevicePoweredOn(
    POHANDLE Handle
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxCompleteIdleState (
    POHANDLE Handle,
    ULONG Component
    )
{
}


NTKERNELAPI
NTSTATUS
sdv_PoFxPowerControl (
    POHANDLE Handle,
    LPCGUID PowerControlCode,
    PVOID InBuffer,
    SIZE_T InBufferSize,
    PVOID OutBuffer,
    SIZE_T OutBufferSize,
    PSIZE_T BytesReturned
    )
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:  return STATUS_SUCCESS; break;
    default: return STATUS_NOT_IMPLEMENTED; break;
  }
}

NTKERNELAPI
VOID
sdv_PoFxNotifySurprisePowerOn(
      PDEVICE_OBJECT Pdo
     )
{
}

NTKERNELAPI
VOID
sdv_PoFxSetDeviceIdleTimeout (
    POHANDLE Handle,
    ULONGLONG IdleTimeout
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxCompleteIdleCondition (
    POHANDLE Handle,
    ULONG Component
    )
{
}

NTKERNELAPI
VOID
sdv_PoFxCompleteDevicePowerNotRequired (
    POHANDLE Handle
    )
{
}

NTKERNELAPI
NTSTATUS 
PoRegisterPowerSettingCallback(
  IN OPTIONAL PDEVICE_OBJECT DeviceObject,
  IN LPCGUID SettingGuid,
  IN PPOWER_SETTING_CALLBACK Callback,
  IN OPTIONAL PVOID Context,
  OUT PVOID *Handle
)
{
    int x = SdvMakeChoice();
    switch (x)
    {
    case 0:  return STATUS_SUCCESS; break;
    default: return STATUS_INSUFFICIENT_RESOURCES; break;
    }
}

#endif
/* ntddk-po.c end */

/* ntddk-ps.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTKERNELAPI
NTSTATUS
PsCreateSystemThread(
    OUT PHANDLE ThreadHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: 
                  #ifdef fun_KSTART_ROUTINE
                  if((StartRoutine==fun_KSTART_ROUTINE)&&(!sdv_inside_init_entrypoint))
                  {
                      fun_KSTART_ROUTINE(StartContext);
                  }
                  #endif
                  return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

BOOLEAN
PsGetVersion(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

#ifdef SDV_Include_NTDDK
NTSTATUS
PsSetCreateProcessNotifyRoutine(
    IN PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    ) 
{ 
    NTSTATUS r = SdvKeepChoice();
    return r; 
}


NTSTATUS
PsSetCreateThreadNotifyRoutine(
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}

NTSTATUS
PsSetLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}
#endif


NTKERNELAPI
NTSTATUS
PsTerminateSystemThread(
    IN NTSTATUS ExitStatus
    )
{
    int x = SdvMakeChoice();
    switch (x) {
        case 0:
                  SdvAssume(FALSE);
                  return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

PACCESS_TOKEN PsReferenceImpersonationToken(
  __inout  PETHREAD Thread,
  __out    PBOOLEAN CopyOnOpen,
  __out    PBOOLEAN EffectiveOnly,
  __out    PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
)
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return NULL;
            break;
        default:
            return (PACCESS_TOKEN)malloc(1);
            break;
    }

}

PACCESS_TOKEN PsReferencePrimaryToken(
  __inout  PEPROCESS Process
)
{
    return (PACCESS_TOKEN)malloc(1);
}

#ifdef SDV_Include_NTDDK
NTSTATUS PsSetCreateProcessNotifyRoutineEx(
  __in  PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine,
  __in  BOOLEAN Remove
)
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}
#endif
NTSTATUS
PsAssignImpersonationToken(
     PETHREAD Thread,
     HANDLE Token
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

VOID
PsDereferenceImpersonationToken(
     PACCESS_TOKEN ImpersonationToken
    ) 
{
}

VOID
PsDereferencePrimaryToken(
     PACCESS_TOKEN PrimaryToken
    ) 
{
}

BOOLEAN
PsDisableImpersonation(
     PETHREAD Thread,
     PSE_IMPERSONATION_STATE ImpersonationState
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return FALSE; break;
        default: return TRUE; break;
    }
}
 

/*NTSTATUS
PshedRegisterPlugin (
     PWHEA_PSHED_PLUGIN_REGISTRATION_PACKET Packet
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}*/



NTSTATUS
PsImpersonateClient(
     PETHREAD Thread,
     PACCESS_TOKEN Token,
     BOOLEAN CopyOnOpen,
     BOOLEAN EffectiveOnly,
     SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

#ifdef SDV_Include_NTDDK
NTSTATUS
PsRemoveLoadImageNotifyRoutine(
     PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif


VOID
PsRestoreImpersonation(
     PETHREAD Thread,
     PSE_IMPERSONATION_STATE ImpersonationState
    ) 
{
}

VOID
PsRevertToSelf(
    VOID
    ) 
{
}



/* ntddk-ps.c end */

/* ntddk-se.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTKERNELAPI
BOOLEAN
SeAccessCheck(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN BOOLEAN SubjectContextLocked,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN KPROCESSOR_MODE AccessMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTKERNELAPI
NTSTATUS
SeAssignSecurity(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}

NTKERNELAPI
NTSTATUS
SeAssignSecurityEx(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}

NTKERNELAPI
NTSTATUS
SeDeassignSecurity(
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    ) 
{ 
    NTSTATUS r = SdvKeepChoice(); 
    return r; 
}

NTKERNELAPI                                                     
BOOLEAN                                                         
SeSinglePrivilegeCheck(                                         
    LUID PrivilegeValue,                                        
    KPROCESSOR_MODE PreviousMode                                
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}                                                          

NTKERNELAPI
BOOLEAN
SeValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    ) 
{ 
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

VOID SeLockSubjectContext(
  __in  PSECURITY_SUBJECT_CONTEXT SubjectContext
)
{
}

VOID SeReleaseSubjectContext(
  __inout  PSECURITY_SUBJECT_CONTEXT SubjectContext
)
{
}

VOID SeUnlockSubjectContext(
  __in  PSECURITY_SUBJECT_CONTEXT SubjectContext
)
{
}

/* ntddk-se.c end */

/* ntddk-rtl.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    )
{
    int x = SdvMakeChoice();
    switch (x) {
        case 0: 
            DestinationString->Length = SourceString->Length;
            if ( AllocateDestinationString == TRUE ) 
            {
                DestinationString->Buffer = malloc(1); 
                int m = SdvKeepChoice(); 
                SdvAssume (SourceString->Length <= m);
                DestinationString->MaximumLength = m;
                return STATUS_SUCCESS;                  
            } 
            else 
            {
                return STATUS_SUCCESS;
            }
            break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    ) 
{
}

NTSYSAPI                                            
NTSTATUS                                            
NTAPI                                               
RtlCharToInteger(                                  
    PCSZ String,                                    
    ULONG Base,                                     
    PULONG Value                                    
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory(
    const VOID *Source1,
    const VOID *Source2,
    SIZE_T Length
    )
{
    SIZE_T r = SdvKeepChoice();
    return r;
}

NTSYSAPI
VOID
NTAPI
RtlCopyMemory32(
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   ULONG Length
   ) 
{  
}

VOID 
RtlCopyMemory(
			IN VOID UNALIGNED  *Destination,
			IN CONST VOID UNALIGNED  *Source,
			IN SIZE_T  Length
		)
{
}


NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNKNOWN_REVISION;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
ULONG
NTAPI
RtlFindLastBackwardRunClear(
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    )
{
    ULONG r = SdvKeepChoice();
    return r;
}

NTSYSAPI
CCHAR
NTAPI
RtlFindLeastSignificantBit(
    IN ULONGLONG Set
    )
{
    CCHAR r = (CCHAR) SdvKeepChoice();
    return r;
}

NTSYSAPI
CCHAR
NTAPI
RtlFindMostSignificantBit(
    IN ULONGLONG Set
    )
{
    CCHAR r = (CCHAR) SdvKeepChoice();
    return r;
}

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    ) 
{  
}

NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    ) 
{ 
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
RtlGetVersion(
    OUT PRTL_OSVERSIONINFOW lpVersionInformation
    )
{
    return STATUS_SUCCESS;
}

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    ) 
{  
}

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    ) 
{  
}

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    ) 
{  
    /* 
        Disable for RtlInitUnicodeString: C4090: '=' : different
        'const' qualifiers.
      
        This is correctly flagged by the compiler, and would be a serious
        issue if the harness was to be executed rather than simulated.

        However, in this case this is per design in order to simulate
        copy of data:
    */
    #pragma warning(disable:4090)

    if (DestinationString) 
    {
        DestinationString->Buffer = SourceString;
	DestinationString->Length = 100;
	
    }
    if ( SourceString == NULL)
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }
     /* Enable after RtlInitUnicodeString: C4090: */
   #pragma warning(default:4090)
}

NTSYSAPI
NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    ) 
{  
    /* 
        Disable for RtlInitUnicodeString: C4090: '=' : different
        'const' qualifiers.
      
        This is correctly flagged by the compiler, and would be a serious
        issue if the harness was to be executed rather than simulated.

        However, in this case this is per design in order to simulate
        copy of data:
    */
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_NAME_TOO_LONG;break;
        default: 
        #pragma warning(disable:4090)
        if (DestinationString) 
        {
            DestinationString->Buffer = SourceString;
        
        }
        /* Enable after RtlInitUnicodeString: C4090: */
        #pragma warning(default:4090)
        return STATUS_SUCCESS;
        break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlInt64ToUnicodeString(
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {

        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_BUFFER_OVERFLOW ;break;
        case 2: return STATUS_INVALID_PARAMETER ;break;
#else
        default: return STATUS_UNSUCCESSFUL;break;
#endif
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString(
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {

        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_BUFFER_OVERFLOW ;break;
        case 2: return STATUS_INVALID_PARAMETER ;break;
#else
        default: return STATUS_UNSUCCESSFUL;break;
#endif
    }
}

NTSYSAPI
ULONG
NTAPI
RtlLengthSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    ULONG r = SdvKeepChoice();
    return r;
}

NTSYSAPI
VOID
NTAPI
RtlMapGenericMask(
    PACCESS_MASK AccessMask,
    PGENERIC_MAPPING GenericMapping
    ) 
{  
}

NTSYSAPI
VOID
NTAPI
sdv_RtlMoveMemory(
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    ) 
{
}

NTSYSAPI
NTSTATUS
NTAPI
sdv_RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_INVALID_PARAMETER;break;
        case 2: return STATUS_OBJECT_NAME_NOT_FOUND;break;
#endif
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN DaclPresent,
    PACL Dacl,
    BOOLEAN DaclDefaulted
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNKNOWN_REVISION;break;
        default: return STATUS_INVALID_SECURITY_DESCR;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_NO_MEMORY;break;
    }
}

ULONG
FASTCALL
RtlU_longByteSwap(
    IN ULONG Source
    )
{
    ULONG r = SdvKeepChoice();
    return r;
}

ULONGLONG
FASTCALL
RtlU_longlongByteSwap(
    IN ULONGLONG Source
    )
{
    ULONGLONG r = SdvKeepChoice();
    return r;
}

USHORT
FASTCALL
RtlU_shortByteSwap(
    IN USHORT Source
    )
{
    USHORT r = (USHORT) SdvKeepChoice();
    return r;
}

NTSYSAPI
CHAR
NTAPI
RtlUpperChar(
    CHAR Character
    )
{
    CHAR r = (CHAR) SdvKeepChoice();
    return r;
}

NTSYSAPI
BOOLEAN
NTAPI
RtlValidRelativeSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSYSAPI
NTSTATUS
NTAPI
RtlVerifyVersionInfo(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG  ConditionMask
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INVALID_PARAMETER ;break;
        default: return STATUS_REVISION_MISMATCH ;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    )
{
    return STATUS_SUCCESS;
}

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain(
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    ) 
{ 
    ULONG r = SdvKeepChoice(); 
    return r; 
}

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    )
{
    NTSTATUS r = SdvMakeChoice();
    switch (r) {
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_ACCESS_DENIED;break;
        case 2: return STATUS_INVALID_HANDLE;break;
        case 3: return STATUS_INVALID_PARAMETER;break;
        case 4: return STATUS_NO_MEMORY;break;
        case 5: return STATUS_BUFFER_TOO_SMALL;break;
        case 6: return STATUS_INVALID_SID;break;
        case 7: return STATUS_BUFFER_OVERFLOW;break;
#endif
        default: return STATUS_UNSUCCESSFUL;break;
  }
}

NTSYSAPI
VOID
NTAPI
sdv_RtlZeroMemory(
    VOID UNALIGNED *Destination,
    SIZE_T Length
    ) 
{
}

NTSTATUS
RtlULongLongToULong(
    __in ULONGLONG ullOperand,
    __out ULONG* pulResult)
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}

BOOLEAN
NTAPI
RtlCreateUnicodeString(
    __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem))
        PUNICODE_STRING DestinationString,
    __in_z PCWSTR SourceString
    )
{
    LONG choice = SdvMakeChoice();
    switch(choice) 
    {
        case 0:
            return FALSE;
            break;
        default:
            return TRUE;
            break;
    }
}

NTSTATUS
NTAPI
RtlDowncaseUnicodeString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        PUNICODE_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}


NTSTATUS
NTAPI
RtlDuplicateUnicodeString(
    __in ULONG Flags,
    __in PCUNICODE_STRING StringIn,
    __out __drv_at(StringOut->Buffer, __drv_allocatesMem(Mem))
        PUNICODE_STRING StringOut
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}


NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToCountedUnicodeString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        PUNICODE_STRING DestinationString,
    __in PCOEM_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}


NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToUnicodeString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        PUNICODE_STRING DestinationString,
    __in PCOEM_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        PANSI_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}


NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToCountedOemString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        POEM_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}



NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToOemString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        POEM_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}




NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        PUNICODE_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}



__checkReturn
NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToCountedOemString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        POEM_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}


__checkReturn
NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToOemString(
    __drv_when(AllocateDestinationString, __out __drv_at(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    __drv_when(!AllocateDestinationString, __inout)
        POEM_STRING DestinationString,
    __in PCUNICODE_STRING SourceString,
    __in BOOLEAN AllocateDestinationString
    )
{
 
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_INTEGER_OVERFLOW;break;
        default:return STATUS_SUCCESS;	break;
    }
}








/* ntddk-rtl.c end */

/* ntddk-zw.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/





NTSTATUS
ZwCancelTimer(
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    { 
        case 0: return STATUS_SUCCESS;break;
#ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        case 3: return STATUS_INVALID_HANDLE;break;
#endif
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwClose(
    IN HANDLE Handle
    )
{
    /*int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INVALID_HANDLE;break;
        default: return STATUS_HANDLE_NOT_CLOSABLE;break;
    }*/
    return STATUS_SUCCESS;
}

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        case 3: return STATUS_ACCESS_VIOLATION;break;
        default: return STATUS_DATATYPE_MISALIGNMENT;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )
{
    
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: 
        *KeyHandle = (PHANDLE) malloc(4);
        return STATUS_SUCCESS;
        break;
        #ifdef SLAM_PRECISE_STATUS
        case 1: *KeyHandle = NULL;return STATUS_OBJECT_NAME_NOT_FOUND;break;
        case 2: *KeyHandle = NULL;return STATUS_INVALID_PARAMETER;break;
        case 3: *KeyHandle = NULL;return STATUS_BUFFER_TOO_SMALL;break;
        case 4: *KeyHandle = NULL;return STATUS_TOO_LATE;break;
        case 5: *KeyHandle = NULL;return STATUS_INVALID_PARAMETER_4;break;
        case 6: *KeyHandle = NULL;return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 7: *KeyHandle = NULL;return STATUS_ACCESS_DENIED;break;
        case 8: *KeyHandle = NULL;return STATUS_INVALID_HANDLE;break;
        case 9: *KeyHandle = NULL;return STATUS_TOO_LATE;break;
        #endif
        default: 
            *KeyHandle = NULL;
            return STATUS_UNSUCCESSFUL;break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_FILE_LOCK_CONFLICT;break;
        case 3: return STATUS_INVALID_FILE_FOR_SECTION;break;
        case 4: return STATUS_INVALID_PAGE_PROTECTION;break;
        case 5: return STATUS_MAPPED_FILE_SIZE_ZERO;break;
        default: return STATUS_SECTION_TOO_BIG;break;
    }
}

NTSTATUS
ZwCreateTimer(
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        #ifdef SLAM_PRECISE_STATUS
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_INVALID_PARAMETER_4;break;
        case 2: return STATUS_INSUFFICIENT_RESOURCES;break;
        case 3: return STATUS_PRIVILEGE_NOT_HELD;break;
        case 4: return STATUS_OBJECT_NAME_INVALID;break;
        case 5: return STATUS_INSUFFICIENT_RESOURCES;break;
        case 6: return STATUS_INVALID_SID;break;
        case 7: return STATUS_INVALID_ACL;break;
        case 8: return STATUS_UNKNOWN_REVISION;break;
        #endif
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteKey(
    IN HANDLE KeyHandle
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_HANDLE;break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_INVALID_PARAMETER;break;
            case -1: return STATUS_UNSUCCESSFUL;break;
            default: return STATUS_NO_MORE_ENTRIES;break;
        }
    } 
    else if ( L == Length && Length!=0) 
    {
        *ResultLength = L;
        return STATUS_SUCCESS;
    }
    else if ( L > Length ) 
    {
        *ResultLength = L;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else 
    {
        *ResultLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }

}

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_INVALID_PARAMETER;break;
            case -1: return STATUS_UNSUCCESSFUL;break;
            default: return STATUS_NO_MORE_ENTRIES;break;
        }
    } 
    else if ( L == Length && Length!=0) 
    {
        *ResultLength = L;
        return STATUS_SUCCESS;
    }
    else if ( L > Length ) 
    {
        *ResultLength = L;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else 
    {
        *ResultLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushKey(
    IN HANDLE KeyHandle
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwMakeTemporaryObject(
    IN HANDLE Handle
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Win32Protect
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_CONFLICTING_ADDRESSES;break;
        case 3: return STATUS_INVALID_PAGE_PROTECTION;break;
        default: return STATUS_SECTION_PROTECTION;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    int choice = SdvMakeChoice();
    
    switch (choice) 
    {
        case 0: 
        *KeyHandle = (PHANDLE) malloc(4);
        return STATUS_SUCCESS;
        break;
        #ifdef SLAM_PRECISE_STATUS
        case 1: *KeyHandle = NULL;return STATUS_TOO_LATE;break;
        case 2: *KeyHandle = NULL;return STATUS_INVALID_PARAMETER_4;break;
        case 3: *KeyHandle = NULL;return STATUS_INVALID_PARAMETER;break;
        case 4: *KeyHandle = NULL;return STATUS_INSUFFICIENT_RESOURCES;break;
        case 5: *KeyHandle = NULL;return STATUS_OBJECT_PATH_SYNTAX_BAD;break;
        case 6: *KeyHandle = NULL;return STATUS_NO_MEMORY;break;
        case 7: *KeyHandle = NULL;return STATUS_INVALID_INFO_CLASS;break;
        #endif
        default: 
            *KeyHandle = NULL;
            return STATUS_PRIVILEGE_NOT_HELD;break;
    }
    
}

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_HANDLE;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSTATUS
ZwOpenTimer(
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{

    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: *TimerHandle = (PHANDLE) malloc(4);return STATUS_SUCCESS;break;
        #ifdef SLAM_PRECISE_STATUS
        case 1: *TimerHandle = NULL;return STATUS_INSUFFICIENT_RESOURCES;break;
        case 2: *TimerHandle = NULL;return STATUS_OBJECT_NAME_INVALID;break;
        case 3: *TimerHandle = NULL;return STATUS_OBJECT_PATH_SYNTAX_BAD;break;
        case 4: *TimerHandle = NULL;return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 5: *TimerHandle = NULL;return STATUS_OBJECT_NAME_NOT_FOUND;break;
        case 6: *TimerHandle = NULL;return STATUS_OBJECT_NAME_INVALID;break;
        case 7: *TimerHandle = NULL;return OBJ_NAME_PATH_SEPARATOR;break;
        case 8: *TimerHandle = NULL;return STATUS_NO_MEMORY;break;
        case 9: *TimerHandle = NULL;return STATUS_INVALID_SID;break;
        case 10: *TimerHandle = NULL;return STATUS_INVALID_ACL;break;
        case 11: *TimerHandle = NULL;return STATUS_UNKNOWN_REVISION;break;
        case 12: *TimerHandle = NULL;return STATUS_REPARSE;break;
        case 13: *TimerHandle = NULL;return STATUS_REPARSE_OBJECT;break;
        #endif
        default: *TimerHandle = NULL;return STATUS_INVALID_PARAMETER;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_INVALID_PARAMETER;break;
            default: return STATUS_UNSUCCESSFUL;break;
        }
    } 
    else if ( L == Length && Length!=0) 
    {
        *ResultLength = L;
        return STATUS_SUCCESS;
    }
    else if ( L > Length ) 
    {
        *ResultLength = L;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else 
    {
        *ResultLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }

        
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySymbolicLinkObject(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_BUFFER_TOO_SMALL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
{
    /* 
      This model for ZwQueryValueKey has a custom body that supports a
      common pattern in drivers: To initially call ZwQueryValueKey
      with Length==0 in order to determine a ResultLenght x, then
      allocate this amount x of memory and thereafter make a
      subsequent call to ZwQueryValueKey with new Length==x.  In the
      first case the driver can assume that STATUS_SUCCESS is not
      returned.
    */
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_INVALID_PARAMETER;break;
            default: return STATUS_UNSUCCESSFUL;break;
        }
    } 
    else if ( L == Length && Length!=0) 
    {
        *ResultLength = L;
        return STATUS_SUCCESS;
    }
    else if ( L > Length ) 
    {
        *ResultLength = L;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else 
    {
        *ResultLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

#if defined(SDV_Include_NTIFS) || defined(SDV_Include_NTDDK)
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_INFO_LENGTH_MISMATCH;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}


NTSTATUS
ZwSetTimer(
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    IN BOOLEAN WakeTimer,
    IN LONG Period OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_SUCCESS;break;
        #ifdef SLAM_PRECISE_STATUS
        case 1: return STATUS_INVALID_PARAMETER_6;break;
        case 2: return STATUS_TIMER_RESUME_IGNORED;break;
        case 3: return STATUS_OBJECT_TYPE_MISMATCH;break;
        case 4: return STATUS_ACCESS_DENIED;break;
        case 5: return STATUS_INVALID_HANDLE;break;
        case 6: return STATUS_UNSUCCESSFUL;break;
        case 7: return STATUS_INVALID_PARAMETER;break;
        case 8: return STATUS_INSUFFICIENT_RESOURCES;break;
        #endif
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}
#endif

NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        case 2: return STATUS_ACCESS_DENIED;break;
        default: return STATUS_INVALID_HANDLE;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        case 1: return STATUS_UNSUCCESSFUL;break;
        default: return STATUS_INVALID_PARAMETER;break;
    }
}

NTSYSAPI
NTSTATUS
NTAPI
ZwWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
    int choice = SdvMakeChoice();
    switch (choice) {
        case 0: return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}



_When_(Timeout == NULL, _IRQL_requires_max_(APC_LEVEL))
_When_(Timeout->QuadPart != 0, _IRQL_requires_max_(APC_LEVEL))
_When_(Timeout->QuadPart == 0, _IRQL_requires_max_(DISPATCH_LEVEL))
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForSingleObject(
  __in      HANDLE Handle,
  __in      BOOLEAN Alertable,
  __in_opt  PLARGE_INTEGER Timeout
)
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return STATUS_ACCESS_DENIED;break;
        case 2: return STATUS_ALERTED;break;
        case 3: return STATUS_INVALID_HANDLE;break;
        case 4: return STATUS_SUCCESS;break;
        case 5: return STATUS_USER_APC;break;
        default: return STATUS_TIMEOUT;break;
    }
}








_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateLocallyUniqueId(
     PLUID Luid
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
_When_(return==0, __drv_allocatesMem(Region))
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory(
     HANDLE ProcessHandle,
     PVOID *BaseAddress,
     ULONG_PTR ZeroBits,
     PSIZE_T RegionSize,
     ULONG AllocationType,
     ULONG Protect
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCommitComplete (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCommitEnlistment (
     HANDLE EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCommitTransaction (
     HANDLE  TransactionHandle,
     BOOLEAN Wait
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCreateEnlistment (
     PHANDLE EnlistmentHandle,
     ACCESS_MASK DesiredAccess,
     HANDLE ResourceManagerHandle,
     HANDLE TransactionHandle,
     POBJECT_ATTRIBUTES ObjectAttributes,
     ULONG CreateOptions,
     NOTIFICATION_MASK NotificationMask,
     PVOID EnlistmentKey
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: *EnlistmentHandle = (PHANDLE) malloc(4);return STATUS_SUCCESS; break;
        default:*EnlistmentHandle = NULL; return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateEvent (
     PHANDLE EventHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     EVENT_TYPE EventType,
     BOOLEAN InitialState
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: *EventHandle = (PHANDLE) malloc(4);return STATUS_SUCCESS; break;
        default: *EventHandle = NULL;return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
ZwCreateKeyTransacted(
     PHANDLE KeyHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     ULONG TitleIndex,
     PUNICODE_STRING Class,
     ULONG CreateOptions,
     HANDLE TransactionHandle,
     PULONG Disposition
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *KeyHandle = (PHANDLE) malloc(4);
            return STATUS_SUCCESS; break;
        default: 
            *KeyHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCreateResourceManager (
     PHANDLE ResourceManagerHandle,
     ACCESS_MASK DesiredAccess,
     HANDLE TmHandle,
     LPGUID ResourceManagerGuid,
     POBJECT_ATTRIBUTES ObjectAttributes,
     ULONG CreateOptions,
     PUNICODE_STRING Description
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0:*ResourceManagerHandle = (PHANDLE) malloc(4); return STATUS_SUCCESS; break;
        default:*ResourceManagerHandle = NULL; return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwCreateTransactionManager (
     PHANDLE TmHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     PUNICODE_STRING LogFileName,
     ULONG CreateOptions,
     ULONG CommitStrength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0:*TmHandle = (PHANDLE) malloc(4); return STATUS_SUCCESS; break;
        default:*TmHandle = NULL; return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteFile(
     POBJECT_ATTRIBUTES ObjectAttributes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile(
     HANDLE FileHandle,
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     ULONG IoControlCode,
     PVOID InputBuffer,
     ULONG InputBufferLength,
     PVOID OutputBuffer,
     ULONG OutputBufferLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwDisplayString(
     PUNICODE_STRING String
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateObject(
     HANDLE SourceProcessHandle,
     HANDLE SourceHandle,
     HANDLE TargetProcessHandle,
     PHANDLE TargetHandle,
     ACCESS_MASK DesiredAccess,
     ULONG HandleAttributes,
     ULONG Options
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwEnumerateTransactionObject (
     HANDLE            RootObjectHandle,
         KTMOBJECT_TYPE    QueryType,
     PKTMOBJECT_CURSOR ObjectCursor,
         ULONG             ObjectCursorLength,
        PULONG            ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
ZwFlushBuffersFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
_When_(return==0, __drv_freesMem(Region))
NTSYSAPI
NTSTATUS
NTAPI
ZwFreeVirtualMemory(
     HANDLE ProcessHandle,
     PVOID *BaseAddress,
     PSIZE_T RegionSize,
     ULONG FreeType
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwFsControlFile(
     HANDLE FileHandle,
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     ULONG FsControlCode,
     PVOID InputBuffer,
     ULONG InputBufferLength,
     PVOID OutputBuffer,
     ULONG OutputBufferLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwGetNotificationResourceManager (
     HANDLE             ResourceManagerHandle,
     PTRANSACTION_NOTIFICATION TransactionNotification,
     ULONG              NotificationLength,
     PLARGE_INTEGER         Timeout,
     PULONG                    ReturnLength,
     ULONG                          Asynchronous,
     ULONG_PTR                  AsynchronousContext
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
     PUNICODE_STRING DriverServiceName
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwLockFile(
     HANDLE FileHandle,
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     PLARGE_INTEGER ByteOffset,
     PLARGE_INTEGER Length,
     ULONG Key,
     BOOLEAN FailImmediately,
     BOOLEAN ExclusiveLock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwNotifyChangeKey(
     HANDLE KeyHandle,
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     ULONG CompletionFilter,
     BOOLEAN WatchTree,
     PVOID Buffer,
     ULONG BufferSize,
     BOOLEAN Asynchronous
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
ZwNotifyChangeMultipleKeys(
     HANDLE MasterKeyHandle,
     ULONG Count,
     OBJECT_ATTRIBUTES SubordinateObjects[],
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     ULONG CompletionFilter,
     BOOLEAN WatchTree,
     PVOID Buffer,
     ULONG BufferSize,
     BOOLEAN Asynchronous
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject(
     PHANDLE DirectoryHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwOpenEnlistment (
     PHANDLE EnlistmentHandle,
     ACCESS_MASK DesiredAccess,
     HANDLE RmHandle,
     LPGUID EnlistmentGuid,
     POBJECT_ATTRIBUTES ObjectAttributes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwOpenEvent (
     PHANDLE EventHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *EventHandle = (PHANDLE) malloc(4);
            return STATUS_SUCCESS; break;
        default: 
            *EventHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKeyEx(
     PHANDLE KeyHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     ULONG OpenOptions
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *KeyHandle = (PHANDLE) malloc(4);
            return STATUS_SUCCESS; break;
        default: 
            *KeyHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKeyTransacted(
     PHANDLE KeyHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     HANDLE TransactionHandle
    ) 
{
    int choice = SdvMakeChoice();
    

    switch (choice)
    {
        case 0: 
            *KeyHandle = (PHANDLE) malloc(4);    
            return STATUS_SUCCESS; break;
        default: 
            *KeyHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKeyTransactedEx(
     PHANDLE KeyHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     ULONG OpenOptions,
     HANDLE TransactionHandle
    ) 
{
    int choice = SdvMakeChoice();
    

    switch (choice)
    {
        case 0: 
            *KeyHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *KeyHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess (
     PHANDLE ProcessHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     PCLIENT_ID ClientId
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *ProcessHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *ProcessHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessTokenEx(
     HANDLE ProcessHandle,
     ACCESS_MASK DesiredAccess,
     ULONG HandleAttributes,
     PHANDLE TokenHandle
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *TokenHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *TokenHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwOpenResourceManager (
     PHANDLE ResourceManagerHandle,
     ACCESS_MASK DesiredAccess,
     HANDLE TmHandle,
     LPGUID ResourceManagerGuid,
     POBJECT_ATTRIBUTES ObjectAttributes
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *ResourceManagerHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *ResourceManagerHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadTokenEx(
     HANDLE ThreadHandle,
     ACCESS_MASK DesiredAccess,
     BOOLEAN OpenAsSelf,
     ULONG HandleAttributes,
     PHANDLE TokenHandle
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *TokenHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *TokenHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }

}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwOpenTransactionManager (
     PHANDLE TmHandle,
     ACCESS_MASK DesiredAccess,
     POBJECT_ATTRIBUTES ObjectAttributes,
     PUNICODE_STRING LogFileName,
     LPGUID TmIdentity,
     ULONG OpenOptions
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *TmHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *TmHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwPowerInformation(
     POWER_INFORMATION_LEVEL InformationLevel,
     PVOID InputBuffer,
     ULONG InputBufferLength,
     PVOID OutputBuffer,
     ULONG OutputBufferLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwPrepareComplete (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwPrepareEnlistment (
     HANDLE EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwPrePrepareComplete (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwPrePrepareEnlistment (
     HANDLE EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryFile(
     HANDLE FileHandle,
     HANDLE Event,
     PIO_APC_ROUTINE ApcRoutine,
     PVOID ApcContext,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID FileInformation,
     ULONG Length,
     FILE_INFORMATION_CLASS FileInformationClass,
     BOOLEAN ReturnSingleEntry,
     PUNICODE_STRING FileName,
     BOOLEAN RestartScan
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            return STATUS_SUCCESS; break;
        default: 
            return STATUS_UNSUCCESSFUL; break;
    }
}
NTSTATUS
ZwQueryEaFile (
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID Buffer,
     ULONG Length,
     BOOLEAN ReturnSingleEntry,
     PVOID EaList,
     ULONG EaListLength,
     PULONG EaIndex,
     BOOLEAN RestartScan
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryFullAttributesFile(
     POBJECT_ATTRIBUTES ObjectAttributes,
     PFILE_NETWORK_OPEN_INFORMATION FileInformation
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwQueryInformationEnlistment (
     HANDLE EnlistmentHandle,
     ENLISTMENT_INFORMATION_CLASS EnlistmentInformationClass,
     PVOID EnlistmentInformation,
     ULONG EnlistmentInformationLength,
     PULONG ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwQueryInformationResourceManager (
     HANDLE ResourceManagerHandle,
     RESOURCEMANAGER_INFORMATION_CLASS ResourceManagerInformationClass,
     PVOID ResourceManagerInformation,
     ULONG ResourceManagerInformationLength,
     PULONG ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwQueryInformationTransactionManager (
     HANDLE TransactionManagerHandle,
     TRANSACTIONMANAGER_INFORMATION_CLASS TransactionManagerInformationClass,
     PVOID TransactionManagerInformation,
     ULONG TransactionManagerInformationLength,
     PULONG ReturnLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
ZwQueryMultipleValueKey(
     HANDLE KeyHandle,
     PKEY_VALUE_ENTRY ValueEntries,
     ULONG EntryCount,
     PVOID ValueBuffer,
     PULONG BufferLength,
     PULONG RequiredBufferLength
    ) 
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_INVALID_PARAMETER;break;
            default: return STATUS_UNSUCCESSFUL;break;
        }
    } 
    else if ( L == RequiredBufferLength && BufferLength!=0) 
    {
        *RequiredBufferLength = L;
        return STATUS_SUCCESS;
    }
    else if ( L > BufferLength ) 
    {
        *RequiredBufferLength = L;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else 
    {
        *RequiredBufferLength = L;
        return STATUS_BUFFER_TOO_SMALL;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySecurityObject(
     HANDLE Handle,
     SECURITY_INFORMATION SecurityInformation,
     PSECURITY_DESCRIPTOR SecurityDescriptor,
     ULONG Length,
     PULONG LengthNeeded
    ) 
{
    ULONG L = SdvKeepChoice();
    if ( L <= 0 ) {
        switch ( L ) {
            case 0: return STATUS_ACCESS_DENIED;break;
            case -1: return STATUS_INVALID_HANDLE;break;
            default: return STATUS_OBJECT_TYPE_MISMATCH;break;
        }
    } 
    else if ( L == Length && Length!=0) 
    {
        *LengthNeeded = L;
        return STATUS_SUCCESS;
    }
    else 
    {
        *LengthNeeded = L;
        return STATUS_BUFFER_TOO_SMALL;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVolumeInformationFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID FsInformation,
     ULONG Length,
     FS_INFORMATION_CLASS FsInformationClass
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwReadOnlyEnlistment (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRecoverEnlistment (
     HANDLE EnlistmentHandle,
     PVOID EnlistmentKey
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRecoverTransactionManager (
     HANDLE TransactionManagerHandle
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwRenameKey(
     HANDLE           KeyHandle,
     PUNICODE_STRING  NewName
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRollbackComplete (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRollbackEnlistment (
     HANDLE EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRollbackTransaction (
     HANDLE  TransactionHandle,
     BOOLEAN Wait
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwRollforwardTransactionManager (
     HANDLE TransactionManagerHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
ZwSetEaFile (
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID Buffer,
     ULONG Length
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwSetInformationEnlistment (
     HANDLE EnlistmentHandle,
     ENLISTMENT_INFORMATION_CLASS EnlistmentInformationClass,
     PVOID EnlistmentInformation,
     ULONG EnlistmentInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationKey(
     HANDLE KeyHandle,
     KEY_SET_INFORMATION_CLASS KeySetInformationClass,
     PVOID KeySetInformation,
     ULONG KeySetInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwSetInformationResourceManager (
     HANDLE ResourceManagerHandle,
     RESOURCEMANAGER_INFORMATION_CLASS ResourceManagerInformationClass,
     PVOID ResourceManagerInformation,
     ULONG ResourceManagerInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}


_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwSetInformationTransactionManager (
     HANDLE TmHandle,
     TRANSACTIONMANAGER_INFORMATION_CLASS TransactionManagerInformationClass,
     PVOID TransactionManagerInformation,
     ULONG TransactionManagerInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwSetQuotaInformationFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID Buffer,
     ULONG Length
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSecurityObject(
     HANDLE Handle,
     SECURITY_INFORMATION SecurityInformation,
     PSECURITY_DESCRIPTOR SecurityDescriptor
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

#if defined(SDV_Include_NTIFS) || defined(SDV_Include_NTDDK)
NTSTATUS
ZwSetTimerEx (
     HANDLE TimerHandle,
     TIMER_SET_INFORMATION_CLASS TimerSetInformationClass,
     PVOID TimerSetInformation,
     ULONG TimerSetInformationLength
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwSetVolumeInformationFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PVOID FsInformation,
     ULONG Length,
     FS_INFORMATION_CLASS FsInformationClass
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSCALLAPI
NTSTATUS
NTAPI
ZwSinglePhaseReject (
     HANDLE            EnlistmentHandle,
     PLARGE_INTEGER TmVirtualClock
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwTerminateProcess (
     HANDLE ProcessHandle,
     NTSTATUS ExitStatus
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadDriver(
     PUNICODE_STRING DriverServiceName
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSYSAPI
NTSTATUS
NTAPI
ZwUnlockFile(
     HANDLE FileHandle,
     PIO_STATUS_BLOCK IoStatusBlock,
     PLARGE_INTEGER ByteOffset,
     PLARGE_INTEGER Length,
     ULONG Key
    ) 
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
NTAPI
ZwCreateTransaction (
    __out PHANDLE TransactionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt LPGUID Uow,
    __in_opt HANDLE TmHandle,
    __in_opt ULONG CreateOptions,
    __in_opt ULONG IsolationLevel,
    __in_opt ULONG IsolationFlags,
    __in_opt PLARGE_INTEGER Timeout,
    __in_opt PUNICODE_STRING Description
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *TransactionHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *TransactionHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}

NTSTATUS
NTAPI
ZwOpenTransaction (
    __out PHANDLE TransactionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in LPGUID Uow,
    __in_opt HANDLE TmHandle
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: 
            *TransactionHandle = (PHANDLE) malloc(4);        
            return STATUS_SUCCESS; break;
        default: 
            *TransactionHandle = NULL;
            return STATUS_UNSUCCESSFUL; break;
    }
}
    
NTSTATUS
NTAPI
ZwQueryInformationTransaction (
    __in HANDLE TransactionHandle,
    __in TRANSACTION_INFORMATION_CLASS TransactionInformationClass,
    __out_bcount(TransactionInformationLength) PVOID TransactionInformation,
    __in ULONG TransactionInformationLength,
    __out_opt PULONG ReturnLength
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
  
NTSTATUS
NTAPI
ZwSetInformationTransaction (
    __in HANDLE TransactionHandle,
    __in TRANSACTION_INFORMATION_CLASS TransactionInformationClass,
    __in PVOID TransactionInformation,
    __in ULONG TransactionInformationLength
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}   

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushVirtualMemory(
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __out PIO_STATUS_BLOCK IoStatus
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }

}


NTSYSAPI
NTSTATUS
NTAPI
ZwSetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    )
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }

}

#ifdef SDV_Include_NTDDK
NTSTATUS 
ZwQueryInformationProcess(
  HANDLE ProcessHandle,
  PROCESSINFOCLASS ProcessInformationClass,
  PVOID ProcessInformation,
  ULONG ProcessInformationLength,
  PULONG ReturnLength
)
{
    int choice = SdvMakeChoice();
    switch (choice)
    {
        case 0: return STATUS_SUCCESS; break;
        default: return STATUS_UNSUCCESSFUL; break;
    }
}
#endif

/* ntddk-misc.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

PCHAR sdv_containing_record(
    IN PCHAR Address,
	IN ULONG_PTR FieldOffset
    )
{
    PCHAR record = Address - FieldOffset;
    return record;
}

VOID
NTAPI
DbgBreakPoint(
    VOID
    )
{
}

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    )
{
}

VOID
sdv_InitializeObjectAttributes(
     OUT POBJECT_ATTRIBUTES p,
     IN PUNICODE_STRING n,
     IN ULONG a,
     IN HANDLE r,
     IN PSECURITY_DESCRIPTOR s
     )
{
}

NTKERNELAPI
LONG
FASTCALL
sdv_InterlockedDecrement(
    IN LONG volatile *Addend
    )
{
    (*Addend)--;
    return *Addend;
}

NTKERNELAPI
LONG
FASTCALL
sdv_InterlockedIncrement(
    IN LONG volatile *Addend
    )
{
    (*Addend)++;
    return *Addend;
}

NTKERNELAPI
VOID
NTAPI
ProbeForRead(
    IN CONST VOID *Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    )
{
}

NTKERNELAPI
VOID
NTAPI
ProbeForWrite (
    IN PVOID Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    )
{
}

ULONGLONG
NTAPI
VerSetConditionMask(
        IN  ULONGLONG   ConditionMask,
        IN  ULONG   TypeMask,
        IN  UCHAR   Condition
    )
{
    ULONGLONG r = SdvKeepChoice();
    return r;
}



BOOLEAN sdv_NT_ERROR (
    NTSTATUS Status
    )
{
    return (Status <= 0xFFFFFFFF && Status >= 0xC0000000 ); 
}

NTSTATUS
EtwWrite(    
    __in REGHANDLE  RegHandle,    
    __in PCEVENT_DESCRIPTOR  EventDescriptor,    
    __in_opt LPCGUID  ActivityId,    
    __in ULONG  UserDataCount,    
    __in_opt PEVENT_DATA_DESCRIPTOR  UserData
    )
{
    int choice;
    if(RegHandle==NULL)
    {
        return STATUS_INVALID_HANDLE;
    }
    else if(UserDataCount>128)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else 
    {
        if(ActivityId!=NULL)
        {
            choice = SdvMakeChoice();
            switch (choice) 
            {
                case 0: return STATUS_SUCCESS;break;
                default: return STATUS_NO_MEMORY;break;
            }
        }
        else
        {
            return STATUS_SUCCESS;
        }
    }
}

NTSTATUS
EtwWriteEndScenario(
    IN REGHANDLE RegHandle,
    IN PCEVENT_DESCRIPTOR EventDescriptor,
    IN LPCGUID ActivityId,
    IN ULONG UserDataCount,
    IN OPTIONAL PEVENT_DATA_DESCRIPTOR UserData
    )

{
    int choice;
    if(RegHandle==NULL)
    {
        return STATUS_INVALID_HANDLE;
    }
    else if(UserDataCount>128)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else 
    {
        if(ActivityId!=NULL)
        {
            choice = SdvMakeChoice();
            switch (choice) 
            {
                case 0: return STATUS_SUCCESS;break;
                default: return STATUS_NO_MEMORY;break;
            }
        }
        else
        {
            return STATUS_SUCCESS;
        }
    }
}

NTKERNELAPI
NTSTATUS
EtwWriteStartScenario(
    IN REGHANDLE RegHandle,
    IN PCEVENT_DESCRIPTOR EventDescriptor,
    IN OUT LPGUID ActivityId,
    IN ULONG UserDataCount,
    IN OPTIONAL PEVENT_DATA_DESCRIPTOR UserData
    )

{
    int choice;
    if(RegHandle==NULL)
    {
        return STATUS_INVALID_HANDLE;
    }
    else if(UserDataCount>128)
    {
        return STATUS_INVALID_PARAMETER;
    }
    else 
    {
        if(ActivityId!=NULL)
        {
            choice = SdvMakeChoice();
            switch (choice) 
            {
                case 0: return STATUS_SUCCESS;break;
                default: return STATUS_NO_MEMORY;break;
            }
        }
        else
        {
            return STATUS_SUCCESS;
        }
    }
}


FORCEINLINE
BOOLEAN   
sdv_RemoveEntryList(    
    IN PLIST_ENTRY  Entry
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return TRUE;break;
        default: return FALSE;break;
    }
}

BOOLEAN   
sdv_IsListEmpty(
    IN PLIST_ENTRY  ListHead    
    )
{
    int choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0: return TRUE;break;
        default: return FALSE;break;
    }
}

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
sdv_InterlockedPopEntrySList (
    __inout PSLIST_HEADER ListHead
    )
{
  PVOID p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PVOID)malloc(1);break;
        default:return  p=NULL;break;
  }
}



NTKERNELAPI
PSLIST_ENTRY
sdv_InterlockedPushEntrySList (
    PSLIST_HEADER ListHead,
    PSLIST_ENTRY ListEntry
    )
{
  PVOID p;
  int x = SdvMakeChoice();
  switch (x) 
  {
        case 0: return p=(PVOID)malloc(1);break;
        default:return  p=NULL;break;
  }
}

LONG sdv_InterlockedCompareExchangeAcquire(
  __inout  LONG volatile *Destination,
  __in     LONG Exchange,
  __in     LONG Comparand
)
{
    return (LONG)SdvKeepChoice();
}


NTSTATUS EtwUnregister(
  __in  REGHANDLE RegHandle
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}


NTSTATUS EtwRegister(
  __in      LPCGUID ProviderId,
  __in_opt  PETWENABLECALLBACK EnableCallback,
  __in_opt  PVOID CallbackContext,
  __out     PREGHANDLE RegHandle
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_INVALID_PARAMETER;break;
        case 1:return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}


NTSTATUS
NTAPI
NtCreateFile (
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_opt PLARGE_INTEGER AllocationSize,
    __in ULONG FileAttributes,
    __in ULONG ShareAccess,
    __in ULONG CreateDisposition,
    __in ULONG CreateOptions,
    __in_bcount_opt(EaLength) PVOID EaBuffer,
    __in ULONG EaLength
    )
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_INVALID_PARAMETER;break;
        case 1:return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSTATUS
NTAPI
NtReadFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    )
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}

NTSTATUS
NTAPI
NtWriteFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    )
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_UNSUCCESSFUL;break;
    }
}
    
BOOLEAN sdv_AbnormalTermination(VOID)
{
    return FALSE;
}


__forceinline
ULONG64
sdv_READ_REGISTER_ULONG64(
    volatile ULONG64 *Register
    )
{
    return *Register;
}

__forceinline
VOID
sdv_READ_REGISTER_BUFFER_ULONG64 (
    volatile ULONG64 *Register,
    _Out_writes_all_(Count) PULONG64 Buffer,
    ULONG Count
    )
{
    ULONG64 x = SdvMakeChoice();
    *Buffer = x;
    return;
}

__forceinline
VOID
sdv_WRITE_REGISTER_ULONG64 (
    volatile ULONG64 *Register,
    ULONG64 Value
    )
{

    *Register = Value;
    return;
}
/* ntddk-misc.c end */

/* ntddk-dma.c begin */
/* These are internal functions within the driver and are registered as function pointers within DMA_OPERATIONS structure.
   These functions are called via function pointer from within the driver.
   Perhaps we should use roletypes for these function pointers*/
 
 
NTSTATUS SDV_GetScatterGatherList(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PDEVICE_OBJECT DeviceObject,
  __in  PMDL Mdl,
  __in  PVOID CurrentVa,
  __in  ULONG Length,
  __in  PDRIVER_LIST_CONTROL ExecutionRoutine,
  __in  PVOID Context,
  __in  BOOLEAN WriteToDevice
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        case 1:return STATUS_BUFFER_TOO_SMALL;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

VOID SDV_PutScatterGatherList(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PSCATTER_GATHER_LIST ScatterGather,
  __in  BOOLEAN WriteToDevice
)
{
}

VOID SDV_FreeMapRegisters(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PVOID MapRegisterBase,
  __in  ULONG NumberOfMapRegisters
)
{
}

VOID SDV_FreeAdapterChannel(
  __in  PDMA_ADAPTER DmaAdapter
)
{
}

NTSTATUS SDV_AllocateAdapterChannel(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PDEVICE_OBJECT DeviceObject,
  __in  ULONG NumberOfMapRegisters,
  __in  PDRIVER_CONTROL ExecutionRoutine,
  __in  PVOID Context
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

VOID SDV_PutDmaAdapter(
  __in  PDMA_ADAPTER DmaAdapter
)
{
}

PVOID SDV_AllocateCommonBuffer(
  __in   PDMA_ADAPTER DmaAdapter,
  __in   ULONG Length,
  __out  PPHYSICAL_ADDRESS LogicalAddress,
  __in   BOOLEAN CacheEnabled
)
{
}

VOID SDV_FreeCommonBuffer(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  ULONG Length,
  __in  PHYSICAL_ADDRESS LogicalAddress,
  __in  PVOID VirtualAddress,
  __in  BOOLEAN CacheEnabled
)
{
}

BOOLEAN SDV_FlushAdapterBuffers(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PMDL Mdl,
  __in  PVOID MapRegisterBase,
  __in  PVOID CurrentVa,
  __in  ULONG Length,
  __in  BOOLEAN WriteToDevice
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return TRUE;break;
        default: return FALSE;break;
    }
}


PHYSICAL_ADDRESS SDV_MapTransfer(
  __in     PDMA_ADAPTER DmaAdapter,
  __in     PMDL Mdl,
  __in     PVOID MapRegisterBase,
  __in     PVOID CurrentVa,
  __inout  PULONG Length,
  __in     BOOLEAN WriteToDevice
)
{

}

ULONG SDV_GetDmaAlignment(
  __in  PDMA_ADAPTER DmaAdapter
)
{
    return (ULONG)SdvKeepChoice();
}


ULONG SDV_ReadDmaCounter(
  __in  PDMA_ADAPTER DmaAdapter
)
{
    return (ULONG)SdvKeepChoice();
}

NTSTATUS SDV_CalculateScatterGatherList(
  __in       PDMA_ADAPTER DmaAdapter,
  __in_opt   PMDL Mdl,
  __in       PVOID CurrentVa,
  __in       ULONG Length,
  __out      PULONG ScatterGatherListSize,
  __out_opt  PULONG NumberOfMapRegisters
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTSTATUS SDV_BuildScatterGatherList(
  __in  PDMA_ADAPTER DmaAdapter,
  __in  PDEVICE_OBJECT DeviceObject,
  __in  PMDL Mdl,
  __in  PVOID CurrentVa,
  __in  ULONG Length,
  __in  PDRIVER_LIST_CONTROL ExecutionRoutine,
  __in  PVOID Context,
  __in  BOOLEAN WriteToDevice,
  __in  PVOID ScatterGatherBuffer,
  __in  ULONG ScatterGatherBufferLength
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

NTSTATUS SDV_BuildMdlFromScatterGatherList(
  __in   PDMA_ADAPTER DmaAdapter,
  __in   PSCATTER_GATHER_LIST ScatterGather,
  __in   PMDL OriginalMdl,
  __out  PMDL *TargetMdl
)
{
    LONG choice = SdvMakeChoice();
    switch (choice) 
    {
        case 0:return STATUS_SUCCESS;break;
        default: return STATUS_INSUFFICIENT_RESOURCES;break;
    }
}

/* ntddk-dma.c end */

/* ntddk-hal.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

#ifdef SDV_Include_NTDDK

NTSTATUS
sdv_HalQuerySystemInformation(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG  BufferSize,
    OUT PVOID  Buffer,
    OUT PULONG  ReturnedLength
    )
{
    NTSTATUS r = SdvKeepChoice();

    SdvAssume(r != STATUS_PENDING);
    return r;
}

#endif

VOID
FASTCALL
HalExamineMBR(
     PDEVICE_OBJECT DeviceObject,
     ULONG SectorSize,
     ULONG MBRTypeIdentifier,
     PVOID *Buffer
    ) 
{
}

PADAPTER_OBJECT
HalGetAdapter (
     PDEVICE_DESCRIPTION DeviceDescription,
     PULONG NumberOfMapRegisters
    ) 
{
    return (PADAPTER_OBJECT)malloc(1);
}

ULONG
HalGetInterruptVector (
     INTERFACE_TYPE  InterfaceType,
     ULONG BusNumber,
     ULONG BusInterruptLevel,
     ULONG BusInterruptVector,
     PKIRQL Irql,
     PKAFFINITY Affinity
    ) 
{
    return (ULONG)SdvKeepChoice();
}

FORCEINLINE
PVOID
sdv_HalAllocateCommonBuffer(
    __in PDMA_ADAPTER DmaAdapter,
    __in ULONG Length,
    __out PPHYSICAL_ADDRESS LogicalAddress,
    __in BOOLEAN CacheEnabled
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return malloc(1);break;
        default: return NULL;break;
    }
}


NTHALAPI
PVOID
sdv_HalAllocateCrashDumpRegisters (
    __in PADAPTER_OBJECT AdapterObject,
    __inout PULONG NumberOfMapRegisters
    )
{
    int x = SdvMakeChoice();
    switch (x) 
    {
        case 0: return malloc(1);break;
        default: return NULL;break;
    }
}

FORCEINLINE
VOID
sdv_HalFreeCommonBuffer(
    __in PDMA_ADAPTER DmaAdapter,
    __in ULONG Length,
    __in PHYSICAL_ADDRESS LogicalAddress,
    __in PVOID VirtualAddress,
    __in BOOLEAN CacheEnabled
    )
{
   
}
/* ntddk-hal.c end */

/* ntddk-wmi.c begin */
/*
	Copyright (c) Microsoft Corporation.  All rights reserved.
*/

typedef enum {
        IrpProcessed,
        IrpNotCompleted,
        IrpNotWmi,
        IrpForward
   } SYSCTL_IRP_DISPOSITION, *PSYSCTL_IRP_DISPOSITION;

NTSTATUS
WmiSystemControl(
    IN PVOID  WmiLibInfo,
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  pirp,
    OUT PSYSCTL_IRP_DISPOSITION  IrpDisposition
    )
{
    int x = SdvMakeChoice();
    int y = SdvMakeChoice();
    NTSTATUS s;
    switch (x) {
        case 0: 
            *IrpDisposition = IrpProcessed;
            s = STATUS_SUCCESS;
            sdv_stub_WmiIrpProcessed(pirp);
            break;
        case 1:
            *IrpDisposition = IrpNotCompleted;
            s = STATUS_SUCCESS;
            sdv_stub_WmiIrpNotCompleted(pirp);
            break;
        case 2:
            *IrpDisposition = IrpForward;
            s = STATUS_SUCCESS;
            sdv_stub_WmiIrpForward(pirp);
            break;
        default:    
            *IrpDisposition = IrpNotWmi;
            sdv_stub_WmiIrpForward(pirp);
            switch (y){
                case 0:
                s = STATUS_SUCCESS;
                break;
            default:
                s = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
    }
    return s;
}
/* ntddk-wmi.c end */

/* sdv_usbd.c begin */
#ifndef USBD_STATUS
typedef LONG USBD_STATUS;
#endif

#ifndef USBD_STATUS_SUCCESS
#define USBD_STATUS_SUCCESS                  ((USBD_STATUS)0x00000000L)
#endif

#ifndef USBD_STATUS_BUFFER_TOO_SMALL
#define USBD_STATUS_BUFFER_TOO_SMALL         ((USBD_STATUS)0xC0003000L)
#endif

#ifdef USB_CONFIGURATION_DESCRIPTOR

USBD_STATUS
USBD_ValidateConfigurationDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc,
    ULONG BufferLength,
    USHORT Level,
    PUCHAR *Offset,
    ULONG Tag)
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:return USBD_STATUS_SUCCESS; break;
    case 1:return USBD_STATUS_BUFFER_TOO_SMALL; break;
    default: return STATUS_INVALID_PARAMETER; break;
  } 
}
#endif

ULONG
USBD_CalculateUsbBandwidth(
    ULONG MaxPacketSize,
    UCHAR EndpointType,
    BOOLEAN LowSpeed
    )
{
	return (ULONG)SdvKeepChoice();
}

#ifdef USB_INTERFACE_DESCRIPTOR
ULONG
USBD_GetInterfaceLength(
    PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor,
    PUCHAR BufferEnd
    )
{
	return (ULONG)SdvKeepChoice();
}
#endif

NTSTATUS
USBD_GetPdoRegistryParameter(
    PDEVICE_OBJECT PhysicalDeviceObject,
    PVOID Parameter,
    ULONG ParameterLength,
    PWSTR KeyName,
    ULONG KeyNameLength
    )
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:return STATUS_SUCCESS; break;
    default: return STATUS_UNSUCCESSFUL; break;
  } 
}

NTSTATUS
USBD_GetRegistryKeyValue (
    HANDLE Handle,
    PWSTR KeyNameString,
    ULONG KeyNameStringLength,
    PVOID Data,
    ULONG DataLength
    )
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:return STATUS_SUCCESS; break;
    default: return STATUS_UNSUCCESSFUL; break;
  } 
}


NTSTATUS
USBD_QueryBusTime(
    PDEVICE_OBJECT RootHubPdo,
    PULONG CurrentFrame
    )
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:return STATUS_SUCCESS; break;
    default: return STATUS_UNSUCCESSFUL; break;
  } 
}

VOID
USBD_ReleaseHubNumber (
    ULONG HubNumber
    )
{
}

ULONG
USBD_AllocateHubNumber (
    )
/*++

Routine Description:

    Allocates a globally unique hub number

Arguments:

Return Value:

    Allocated Hub Number.  Zero if unable to allocate number.

--*/
{
	return (ULONG)SdvKeepChoice();
}


VOID
USBD_RemoveDeviceFromGlobalList (
     PVOID ChildInstance
    )
/*++

Routine Description:

    Removes a child device from the global child device list

Arguments:

    ChildInstance - Unique value representing the child device (i.e. PDO address, etc.).

Return Value:

    VOID

--*/
{
}

VOID
USBD_MarkDeviceAsDisconnected (
    PVOID ChildInstance
    )
/*++

Routine Description:

    Marks a device on the global child list as disconnected

Arguments:

    ChildInstance - Unique value representing the child device (i.e. PDO address, etc.).

Return Value:

    VOID

--*/
{
}

#ifdef USBD_CHILD_STATUS



USBD_CHILD_STATUS
USBD_AddDeviceToGlobalList (
    PVOID ChildInstance,
    PVOID HubInstance,
    USHORT PortNumber,
    ULONG ConnectorId,
    USHORT IdVendor,
    USHORT IdProduct,
    USHORT BcdVersion,
    PUSB_ID_STRING SerialNumber
    )
	/*++

Routine Description:

    Attempts to add a new USB device to the global list

Arguments:

    ChildInstance - Unique value representing the child device (i.e. PDO address, etc.).
    
    HubInstance - Unique value representing the parent hub (i.e. FDO address, etc.).
    
    PortNumber - Port number the device child device is attached to.
    
    ConnectorId - Connector ID for the port.  0 if no connector ID exists.
    
    IdVendor - USB vendor code for the device.
    
    IdProduct - USB product code for the device.
    
    BcdVersion- BCD version number for the device.

    SerialNumber - Serial number for the device.

Return Value:

    USBD_CHILD_STATUS_INSERTED indicates device was successfully added
    
    USBD_CHILD_STATUS_DUPLICATE_PENDING_REMOVAL indicates a device with an identical VID/PID/Rev and serial number is pending removal
    
    USBD_CHILD_STATUS_DUPLICATE_FOUND indicates a device with an indentical VID/PID/Rev and serial number was found and is not pending removal
    
    USBD_CHILD_STATUS_FAILURE indicates unable to allocate new list entry or other general error

--*/
{
  int x = SdvMakeChoice();
  switch (x)
  {
    case 0:return USBD_CHILD_STATUS_INSERTED; break;
	case 1:return USBD_CHILD_STATUS_DUPLICATE_PENDING_REMOVAL; break;
	case 2:return USBD_CHILD_STATUS_DUPLICATE_FOUND; break;
    default: return USBD_CHILD_STATUS_FAILURE; break;
  } 
}
#endif

#ifdef USB_INTERFACE_DESCRIPTOR
PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptorEx(
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    PVOID StartPosition,
    LONG InterfaceNumber,
    LONG AlternateSetting,
    LONG InterfaceClass,
    LONG InterfaceSubClass,
    LONG InterfaceProtocol
    )
/*++

Routine Description:

    Parses a standard USB configuration descriptor (returned from a device)
    for a specific interface, alternate setting class subclass or protocol
    codes

Arguments:

    ConfigurationDescriptor -
    StartPosition -
    InterfaceNumber -
    AlternateSetting
    InterfaceClass -
    InterfaceSubClass -
    InterfaceProtocol -
Return Value:

    NT status code.

--*/
{
  int x = SdvMakeChoice();
  switch (x) 
  {
      case 0: return malloc(1);break;
      default: return NULL;break;
  }
}
#endif

#ifdef USB_COMMON_DESCRIPTOR

    
PUSB_COMMON_DESCRIPTOR
USBD_ParseDescriptors(
    PVOID DescriptorBuffer,
    ULONG TotalLength,
    PVOID StartPosition,
    LONG DescriptorType
)
/*++

Routine Description:

    Parses a group of standard USB configuration descriptors (returned
    from a device) for a specific descriptor type.

Arguments:

    DescriptorBuffer - pointer to a block of contiguous USB desscriptors
    TotalLength - size in bytes of the Descriptor buffer
    StartPosition - starting position in the buffer to begin parsing,
                    this must point to the begining of a USB descriptor.
    DescriptorType - USB descritor type to locate.


Return Value:

    pointer to a usb descriptor with a DescriptorType field matching the
            input parameter or NULL if not found.

--*/
{
  int x = SdvMakeChoice();
  switch (x) 
  {
      case 0: return malloc(1);break;
      default: return NULL;break;
  }
}

#endif/* sdv_usbd.c end */

