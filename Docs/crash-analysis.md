# Crash analysis

Found a crash?

* Validate the cause

```
kd> !analyze -v
kd> .load \path\to\msec.dll ; !exploitable
```

* IRPs are handled sequentially, therefore the crash has to be related to at least the last
 IRP sent. CFB will store the last IRP fuzzed in `IrpDumper!g_LastTestCase` in the following 
 format:
```
struct {
UINT32 SizeOfBuffer;
BYTE Buffer[SizeOfBuffer];
}
```

So you can easily dump the last IRP from WinDbg in 3 simple steps (which can easily automated in
a WinDbg JS script):

* First get the length, for example:

```
kd> dd poi(IrpDumper!g_LastTestCase) l1
ffffe088`f37ae000  00000218
```

The data length is 0x218 bytes.


* Then you can confirm by viewing those data
```
kd> db poi(IrpDumper!g_LastTestCase+4) l218
ffffe088`f37ae004  5c 00 44 00 6f 00 73 00-44 00 65 00 76 00 69 00  \.D.o.s.D.e.v.i.
ffffe088`f37ae014  63 00 65 00 73 00 5c 00-50 00 68 00 79 00 73 00  c.e.s.\.P.h.y.s.
[...]
```

* And finally store them on the debugger host.
```
kd> .writemem C:\Whatever.raw poi(IrpDumper!g_LastTestCase+4) l218
```

All the other info (like device name, ioctl number, etc.) can be retrieved from `analyze -v` above
