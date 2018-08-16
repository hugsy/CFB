# Canadian Fuzzy Bear

## Concept

_TODO add pretty diagram here_

## Show Me!

_TODO add pretty screenshot here_


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
CFB >>> hook <DriverPath>
```

For instance using the brilliant [HEVD](https://github.com/hacksysteam/HackSysExtremeVulnerableDriver) driver
```
CFB >>> hook \driver\hevd
```

The driver `IRP_MJ_DEVICE_CONTROL` will then be intercepted by the IrpDumper driver: every call to `DeviceIoControl()` 
to this driver which will be captured and its the IRP user data (if any) pushed to an internal named pipe (by default `\Devices\PIPE\CFB`).

It is now possible to start the fuzzer GUI that offers several possibilities:

  - Passive role, by simply monitoring all IRPs in the DataView
  - Automatically fuzz a specific IOCTL code on a driver using some well-known fuzzing patterns (BitFlip, ByteFlip, Random, etc.)
  - Hexview, save to disk, modify and/or manually replay a specific IOCTL.

Once the fuzzing is done, the thread will terminate cleanly.

If a bug is found, the original IRP data will be stored inside a directory `.\captured` (by default). You can use 
the tool `replay.exe .\captured\<IrpDataFile>` to narrow down the faulty IRP. 
In any case, it is **highly** recommended to run this tool on a VM with KD attached to it to facilitate the work 
of triage (`!exploitable` still lives!).

Hope you'll enjoy the tool !!

-- hugsy