# Canadian Fuzzy Bear

## Concept

_TODO add pretty diagram here_


## Build

### GUI

Clone the repository, and build the `CFB.sln` at the project root with Visual Studio (Debug - very verbose - or Release).



### Command line

Clone the repository and in a VS prompt run

```
> msbuild CFB.sln /p:Configuration=Release
```


## Use

Load the driver (using OSR DriverLoader for instance). Then run the client in a privilege command 
prompt (must have the `SeDebugPrivilege`). The client will open a handle to the driver. To hook a driver
simply enter

```
CFB >>> hook <MyDriver>
```

For instance
```
CFB >>> hook \driver\tcpip
```

All `DeviceIoControl()` from user-land will be hooked through our driver, which will capture the IRP user data 
(if any). This data will be passed back to the monitor in userland (i.e. the client) that'll inject a thread 
into the process who emitted that interrupt request, and fuzz the data using a few known strategies like:

 - LSB/MSB {D,Q}WORD bitflip
 - {D,Q}WORD replacement with large known values (0xffffffff, 0x7fffffff, etc.)

Once the fuzzing is done, the thread will terminate cleanly.

If a bug is found, the original IRP data will be stored inside a directory `.\captured` (by default). You can use 
the tool `replay.exe .\captured\<IrpDataFile>` to narrow down the faulty IRP. 
In any case, it is **highly** recommended to run this tool on a VM with KD attached to it to facilitate the work 
of triage (`!exploitable` still lives!).

Hope you'll enjoy the tool !!
