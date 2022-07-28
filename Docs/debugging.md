
# Debugging CFB

## The Driver


### Enumerate the hooked drivers

```
dx @$drivers=Debugger.Utility.Collections.FromListEntry( IrpMonitor!Globals->DriverManager.m_Entries.m_ListHead, "IrpMonitor!CFB::Driver::HookedDriver", "Next")
```

### Check if a driver is set for data capture

```
dx @$drivers.First().Enabled == true && @$drivers.First().State == 1
```

### Enumerate the captured IRPs

```
dx @$irps=Debugger.Utility.Collections.FromListEntry( IrpMonitor!Globals->IrpManager.m_Entries.m_ListHead, "IrpMonitor!CFB::Driver::CapturedIrp", "Next")
```
