# Concept

`IrpDumper.sys` is the driver part of the CFB Broker that will auto-extract and install when launched. The driver will be responsible for hooking the IRP Major Function table of
the driver that is requested to be hooked, via an IOCTL passed from the Broker.
Upon success, the IRP table of the driver will then be pointing to `IrpDumper.sys` interception routine, as we can easily see with a debugger or tools like [`WinObjEx64`](https://github.com/hfiref0x/WinObjEx64).

![img](https://i.imgur.com/dYqHE6q.png)

`IrpDumper.sys` in itself then acts a rootkit, proxy-ing all calls to the targeted driver(s). When a `DeviceIoControl` is sent to a hooked driver, `IrpDumper` will simply capture the data if any, and push a message to the user-land agent (`Broker`), and yield the execution back to the legitimate drivers, allowing the intended code to continue as expected.
The `Broker` stores all this data in user-land waiting for a event to ask for them.
