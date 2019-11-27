#!/usr/bin/python3

"""

Simple command line client to test the broker exposed features.

This shouldn't be used in other cases than testing.

"""

from enum import Enum, unique
from struct import *

import base64, json, pprint, sys, time, socket, datetime

MAX_MESSAGE_SIZE = 65536
MAX_ACCEPTABLE_MESSAGE_SIZE = MAX_MESSAGE_SIZE-2


#
# some helpers
#
DEBUG = True

def u8 (x): return struct.unpack("<B", x)[0]
def u16(x): return struct.unpack("<H", x)[0]
def u32(x): return struct.unpack("<I", x)[0]
def u64(x): return struct.unpack("<Q", x)[0]

def p8 (x): return struct.pack("<B", x)
def p16(x): return struct.pack("<H", x)
def p32(x): return struct.pack("<I", x)
def p64(x): return struct.pack("<Q", x)

def log(x): print(x)
def dbg(x): log(f"[*] {x}") if DEBUG else None
def ok(x): log(f"[+] {x}")
def err(x): log(f"[-] {x}")
def warn(x): log(f"[!] {x}")


#
# some windows constants
#
GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 0x3
INVALID_HANDLE_VALUE = -1
PIPE_READMODE_BYTE = 0x0
PIPE_READMODE_MESSAGE = 0x2
ERROR_SUCCESS = 0
ERROR_PIPE_BUSY = 231
ERROR_MORE_DATA = 234

FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100
FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000
FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200

LCID_ENGLISH = (0x00 & 0xFF) | (0x01 & 0xFF) << 16


def convert_GetSystemTime(t):
    unix_ts = (t / 10000000) - 11644473600
    return datetime.datetime.fromtimestamp(unix_ts)



def hexdump(source, length=0x10, separator=".", base=0x00, align=10):
    result = []
    for i in range(0, len(source), length):
        chunk = bytearray(source[i:i + length])
        hexa = " ".join(["%.02x" % b for b in chunk])
        text = "".join([chr(b) if 0x20 <= b < 0x7F else separator for b in chunk])
        msg = "{addr:#0{aw}x}     {data:<{dw}}    {text}".format(aw=align,addr=base+i,dw=3*length,data=hexa,text=text)
        result.append(msg)
    return "\n".join(result)


PATH_PIPE_LOCAL = b"\\\\.\\pipe\\CFB"
PATH_PIPE_REMOTE = b"\\\\10.0.0.63\\pipe\\CFB"
PATH_TCP_REMOTE = ("10.0.0.63", 1337)

TEST_DRIVER_NAME = "\\driver\\lxss\0"
TEST_DRIVER_NAME = "\\driver\\condrv\0"


@unique
class TaskType(Enum):
    """
    See TaskType in ..\Broker\Task.h
    """
    IoctlResponse = 1
    HookDriver = 2
    UnhookDriver = 3
    GetDriverInfo = 4
    NumberOfDriver = 5
    NotifyEventHandle = 6
    EnableMonitoring = 7
    DisableMonitoring = 8
    GetInterceptedIrps = 9
    ReplayIrp = 10
    StoreTestCase = 11
    EnumerateDrivers = 12

def IrpMajorType(x):
    IrpMajorTypes = {
        0x00: "IRP_MJ_CREATE",
        0x01: "IRP_MJ_CREATE_NAMED_PIPE",
        0x02: "IRP_MJ_CLOSE",
        0x03: "IRP_MJ_READ",
        0x04: "IRP_MJ_WRITE",
        0x05: "IRP_MJ_QUERY_INFORMATION",
        0x06: "IRP_MJ_SET_INFORMATION",
        0x07: "IRP_MJ_QUERY_EA",
        0x08: "IRP_MJ_SET_EA",
        0x09: "IRP_MJ_FLUSH_BUFFERS",
        0x0a: "IRP_MJ_QUERY_VOLUME_INFORMATION",
        0x0b: "IRP_MJ_SET_VOLUME_INFORMATION",
        0x0c: "IRP_MJ_DIRECTORY_CONTROL",
        0x0d: "IRP_MJ_FILE_SYSTEM_CONTROL",
        0x0e: "IRP_MJ_DEVICE_CONTROL",
        0x0f: "IRP_MJ_INTERNAL_DEVICE_CONTROL",
        0x10: "IRP_MJ_SHUTDOWN",
        0x11: "IRP_MJ_LOCK_CONTROL",
        0x12: "IRP_MJ_CLEANUP",
        0x13: "IRP_MJ_CREATE_MAILSLOT",
        0x14: "IRP_MJ_QUERY_SECURITY",
        0x15: "IRP_MJ_SET_SECURITY",
        0x16: "IRP_MJ_POWER",
        0x17: "IRP_MJ_SYSTEM_CONTROL",
        0x18: "IRP_MJ_DEVICE_CHANGE",
        0x19: "IRP_MJ_QUERY_QUOTA",
        0x1a: "IRP_MJ_SET_QUOTA",
        0x1b: "IRP_MJ_PNP",    
    }
    return IrpMajorTypes.get(i, "")
    


def PrepareRequest(dwType, *args):
    j = {
        "header": {},
        "body":{
            "type": dwType.value,
        }
    }
    data_length = 0
    data = b""
    for arg in args:
        data_length += len(arg)
        data += arg
    j["body"]["data_length"] = data_length
    j["body"]["data"] = base64.b64encode(data).decode("utf-8")
    return json.dumps(j).encode("ascii")



class BrokerTestMethods:

    def test_OpenPipe(self):
        raise NotImplementedError("OpenPipe() must be redefined")

    def test_ClosePipe(self):
        raise NotImplementedError("ClosePipe() must be redefined")

    def sr(self, _type, *args):
        raise NotImplementedError("sr() must be redefined")

    def test_EnumerateDrivers(self):
        js = self.sr(TaskType.EnumerateDrivers)
        ok("EnumerateDrivers -> " + json.dumps(js, indent=4, sort_keys=True))

    def test_HookDriver(self):
        lpszDriverName = TEST_DRIVER_NAME.encode("utf-16")[2:]
        res = self.sr(TaskType.HookDriver, lpszDriverName)
        ok("hook -> " + json.dumps(res, indent=4, sort_keys=True))

    def test_UnhookDriver(self):
        lpszDriverName = TEST_DRIVER_NAME.encode("utf-16")[2:]
        res = self.sr(TaskType.UnhookDriver, lpszDriverName)
        ok("unhook -> " + json.dumps(res, indent=4, sort_keys=True))

    def test_EnableMonitoring(self):
        res = self.sr(TaskType.EnableMonitoring)
        ok("enable_monitoring -> " + json.dumps(res, indent=4, sort_keys=True))
    
    def test_DisableMonitoring(self):
        res = self.sr(TaskType.DisableMonitoring)
        ok("disable_monitoring -> " + json.dumps(res, indent=4, sort_keys=True))

    def test_GetInterceptedIrps(self):
        res = self.sr(TaskType.GetInterceptedIrps)
        # sanitize some fields
        for i in range(res["body"]["nb_entries"]):
            res["body"]["entries"][i]["header"]["DeviceName"] = "'{}'".format("".join(map(chr, res["body"]["entries"][i]["header"]["DeviceName"])))
            res["body"]["entries"][i]["header"]["DriverName"] = "'{}'".format("".join(map(chr, res["body"]["entries"][i]["header"]["DriverName"])))
            res["body"]["entries"][i]["header"]["TimeStamp"] = "'{}'".format(convert_GetSystemTime(res["body"]["entries"][i]["header"]["TimeStamp"]).strftime("%F-%H-%m-%S"))
            res["body"]["entries"][i]["header"]["Type"] = "'{}'".format(IrpMajorType(res["body"]["entries"][i]["header"]["Type"]))
        ok("get_irps -> " + json.dumps(res, indent=4, sort_keys=True))



class BrokerTestPipeMethods(BrokerTestMethods):
    def __init__(self):
        from ctypes import c_ulong, create_string_buffer, windll
        self.hPipe = None
        return

    def FormatMessage(self, dwMessageId):
        lpSource = LPWSTR()

        nb_chars = windll.kernel32.FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
            None, 
            dwMessageId, 
            LCID_ENGLISH, 
            byref(lpSource), 
            0, 
            None
        )
        if nb_chars == 0:
            raise Exception("Failed to get message for error code: %d" % dwMessageId)

        lpErrorMessage = lpSource.value[:].strip()
        windll.kernel32.LocalFree(lpSource)
        return lpErrorMessage

    def sr(self, _type, *args):
        hPipe = self.hPipe
        ## send
        req = PrepareRequest(_type, *args)
        cbWritten = c_ulong(0)
        assert windll.kernel32.WriteFile(hPipe, req, len(req), byref(cbWritten), None)
        assert len(req) == cbWritten.value
        
        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(MAX_MESSAGE_SIZE)
        assert windll.kernel32.ReadFile(hPipe, szBuf, MAX_ACCEPTABLE_MESSAGE_SIZE, byref(cbRead), None)
        res = json.loads(szBuf.value)
        return res

    def test_OpenPipe(self):
        hPipe = windll.kernel32.CreateFileA(PATH_PIPE_REMOTE, GENERIC_READ | GENERIC_WRITE, 0, None, OPEN_EXISTING, 0, None)
        assert hPipe != INVALID_HANDLE_VALUE 
        assert windll.kernel32.GetLastError() != ERROR_PIPE_BUSY 
        dwMode = c_ulong(PIPE_READMODE_BYTE)
        windll.kernel32.SetNamedPipeHandleState(hPipe, byref(dwMode), None, None)
        self.hPipe = hPipe
        return

    def test_ClosePipe(self):
        assert windll.kernel32.CloseHandle(self.hPipe)




class BrokerTestTcpMethods(BrokerTestMethods):
    def __init__(self):
        import signal
        self.hSock = None
        self.dwTimeout = 20
        signal.signal(signal.SIGALRM, self.throw_timeout_exception)
        return

    def throw_timeout_exception(self, signum, frame):
        import socket, errno, os
        raise socket.timeout(os.strerror(errno.ETIME))

    def sr(self, _type, *args):
        import signal
        ## send
        req = PrepareRequest(_type, *args)
        signal.alarm(self.dwTimeout)
        ret = self.hSock.sendall(req)
        signal.alarm(0)
        assert ret is None
        
        ## recv response
        signal.alarm(self.dwTimeout)
        res = b""
        while True:
            # fucking ip fragmentation
            res += self.hSock.recv(MAX_MESSAGE_SIZE)
            assert res is not None
            res = res[::]
            try:
                js = json.loads(res)
                return js
            except Exception as e:
                err("Exception {:s} when parsing '''\n{:s}\n'''".format(str(e),res.decode("utf-8")))

    def test_OpenPipe(self):
        import signal
        self.hSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.hSock.connect(PATH_TCP_REMOTE)
        ok("tcp socket connected")
        return

    def test_ClosePipe(self):
        self.hSock.close()
        ok("tcp socket disconnect")
        return



def test_method(r):
    r.test_OpenPipe()
    ok("OpenPipe() success")
    r.test_EnumerateDrivers()
    ok("EnumerateDrivers() success")
    r.test_HookDriver()
    ok("HookDriver() success")
    r.test_EnableMonitoring()
    ok("EnableMonitoring() success")
    while True:
        try:
            r.test_GetInterceptedIrps()
            time.sleep(1)
        except KeyboardInterrupt:
            break
    ok("GetInterceptedIrps() success")
    r.test_DisableMonitoring()
    ok("DisableMonitoring() success")
    r.test_UnhookDriver()
    ok("UnhookDriver() success")
    r.test_ClosePipe()
    ok("ClosePipe() success")
    return

def test_pipe(r):
    if len(sys.argv) == 2:
        PATH_PIPE_REMOTE = sys.argv[1]
    ok("connecting to {:s}".format(PATH_PIPE_REMOTE))
    test_method( BrokerTestTcpMethods() )

def test_tcp():
    global PATH_TCP_REMOTE
    if len(sys.argv) == 3:
        PATH_TCP_REMOTE = (sys.argv[1], int(sys.argv[2]))
    ok("connecting to {:s}:{:d}".format(*PATH_TCP_REMOTE))
    test_method( BrokerTestTcpMethods() )


if __name__ == '__main__':
    test_tcp()
    #test_pipe()

    input("test end...")
    sys.exit(0)