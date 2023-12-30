# Usage

A Windows 7+ machine ([Windows 10 SDK VM](https://developer.microsoft.com/en-us/windows/downloads/virtual-machines) is recommended)

On this target machine, simply enable BCD test signing flag (in `cmd.exe` as Admin):

```
bcdedit.exe /set {whatever-profile} testsigning on
```

If using in Debug mode, `IrpDumper.sys` will provide a lot more valuable information as to what's being hooked (the price of performance). All those info can be visible via tools like `DebugView.exe` or a kernel debugger like WinDbg. In either case, you must enable kernel debug BCD flag (in `cmd.exe` as Admin):

```
bcdedit.exe /set {whatever-profile} debug on
```

It is also recommended to edit the KD verbosity level, via:
  - the registry for a permanent effect (`reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /v DEFAULT /t REG_DWORD /d 0xf`)
  - directly from WinDbg for only the current session (`ed nt!Kd_Default_Mask 0xf`)


If you plan on (re-)compiling any of the tools, you must install VS (2019 preferred). If using the Release binaries, you only need VS C++ Redist installed(x86 or x64 depending on your VM architecture).

Follow the indications in the `Docs/` folder to improve your setup.