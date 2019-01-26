# Crash analysis

Found a crash ?

* Validate the cause

```
kd> !analyze -v
kd> .load \path\to\msec.dll ; !exploitable
```

* CFB will store the last IRP fuzzed in `IrpDumper!g_LastTestCase` in the following format:
```
struct {
UINT32 SizeOfBuffer;
BYTE Buffer[SizeOfBuffer];
}
```

To retrieve the content in WinDbg:
```
# Get the length
kd> dd poi(IrpDumper!g_LastTestCase) l1
ffffe088`f37ae000  00000218

# Visualize the content
kd> db poi(IrpDumper!g_LastTestCase) l218
ffffe088`f37ae004  5c 00 44 00 6f 00 73 00-44 00 65 00 76 00 69 00  \.D.o.s.D.e.v.i.
ffffe088`f37ae014  63 00 65 00 73 00 5c 00-50 00 68 00 79 00 73 00  c.e.s.\.P.h.y.s.
[...]

# Store it locally
kd> .writemem C:\Whatever.raw poi(IrpDumper!g_LastTestCase) l218
```

All the other info (like device name, ioctl number, etc.) can be retrieved from `analyze -v` above
