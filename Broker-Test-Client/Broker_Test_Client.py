
"""

Simple command line client to test the broker exposed features.

This shouldn't be used in other cases than testing.

"""

from enum import Enum, unique
from ctypes import *
from struct import *

import json, pprint, sys


def ok(x): print("[+] {0}".format(x))
def p16(x): return pack("<H", x)
def p32(x): return pack("<I", x)
def u16(x): return unpack("<H", x)[0]
def u32(x): return unpack("<I", x)[0]


def hexdump(source, length=0x10, separator=".", base=0x00, align=10):
    result = []
    for i in range(0, len(source), length):
        chunk = bytearray(source[i:i + length])
        hexa = " ".join(["%.02x" % b for b in chunk])
        text = "".join([chr(b) if 0x20 <= b < 0x7F else separator for b in chunk])
        msg = "{addr:#0{aw}x}     {data:<{dw}}    {text}".format(aw=align,addr=base+i,dw=3*length,data=hexa,text=text)
        result.append(msg)
    return "\n".join(result)


TEST_DRIVER_NAME = "\\driver\\vboxguest"
BUFSIZE = 4096

#
# some windows constants
#
GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 0x3
INVALID_HANDLE_VALUE = -1
PIPE_READMODE_MESSAGE = 0x2
ERROR_SUCCESS = 0
ERROR_PIPE_BUSY = 231
ERROR_MORE_DATA = 234


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



def PrepareTlvMessage(dwType, *args):
    length = 0
    value = b""
    for arg in args:
        length += len(arg)
        value += arg
    msg = b""
    msg+= p32(dwType.value)
    msg+= p32(length)
    msg+= value
    return msg




class BrokerTestMethods:
    def __init__(self):
        self.hPipe = None
        return
    

    def test_OpenPipe(self):
        hPipe = windll.kernel32.CreateFileA(
            b"\\\\.\\pipe\\CFB", 
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            None, 
            OPEN_EXISTING, 
            0, 
            None
        )
        assert hPipe != INVALID_HANDLE_VALUE 
        assert windll.kernel32.GetLastError() != ERROR_PIPE_BUSY 
        dwMode = c_ulong(PIPE_READMODE_MESSAGE)
        windll.kernel32.SetNamedPipeHandleState(hPipe, byref(dwMode), None, None)
        self.hPipe = hPipe
        return


    def test_ClosePipe(self):
        assert windll.kernel32.CloseHandle(self.hPipe)
        return


    def test_HookDriver(self):
        ## send request
        lpszDriverName = TEST_DRIVER_NAME.encode("utf-16")[2:]
        tlv_msg = PrepareTlvMessage(TaskType.HookDriver, lpszDriverName)
        cbWritten = c_ulong(0)
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) == cbWritten.value
        
        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(BUFSIZE)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, BUFSIZE, byref(cbRead), None)
         # hookdriver doesn't return any data (header only = 2*uint32_t + uint32_t for GLE)
        assert cbRead.value == 12
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        return


    def test_UnhookDriver(self):
        ## send request
        lpszDriverName = TEST_DRIVER_NAME.encode("utf-16")[2:]
        tlv_msg = PrepareTlvMessage(TaskType.UnhookDriver, lpszDriverName)
        cbWritten = c_ulong(0)
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) ==  cbWritten.value

        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(BUFSIZE)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, BUFSIZE, byref(cbRead), None)
        assert cbRead.value == 12
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        return


    def test_EnableMonitoring(self):
        ## send request
        tlv_msg = PrepareTlvMessage(TaskType.EnableMonitoring)
        cbWritten = c_ulong(0)
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) ==  cbWritten.value

        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(BUFSIZE)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, BUFSIZE, byref(cbRead), None)
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        assert u32(szBuf[8:12]) == ERROR_SUCCESS
        return

    
    def test_DisableMonitoring(self):
        ## send request
        tlv_msg = PrepareTlvMessage(TaskType.DisableMonitoring)
        cbWritten = c_ulong(0)  
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) == cbWritten.value

        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(BUFSIZE)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, BUFSIZE, byref(cbRead), None)
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        assert u32(szBuf[8:12]) == ERROR_SUCCESS
        return


    def test_GetInterceptedIrps(self):
        ## send request
        tlv_msg = PrepareTlvMessage(TaskType.GetInterceptedIrps)
        cbWritten = c_ulong(0)  
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) == cbWritten.value

        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(BUFSIZE)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, BUFSIZE, byref(cbRead), None)
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        print(hexdump(szBuf))
        input("> ")
        assert u32(szBuf[8:12]) == ERROR_SUCCESS
        assert json.loads(szBuf[12:])
        return


if __name__ == '__main__':
    r = BrokerTestMethods()
    r.test_OpenPipe()
    r.test_HookDriver()
    r.test_EnableMonitoring()
    r.test_GetInterceptedIrps()
    r.test_DisableMonitoring()
    r.test_UnhookDriver()
    r.test_ClosePipe()
