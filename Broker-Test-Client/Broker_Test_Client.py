
"""

Simple command line client to test the broker exposed features.

This shouldn't be used in other cases than testing.

"""

from enum import Enum, unique
from ctypes import *
from ctypes.wintypes import *
from struct import *

import json, pprint, sys


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

FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100
FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000
FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200

LCID_ENGLISH = (0x00 & 0xFF) | (0x01 & 0xFF) << 16


#
# some helpers
#
def ok(x): print("[+] {0}".format(x))
def p16(x): return pack("<H", x)
def p32(x): return pack("<I", x)
def u16(x): return unpack("<H", x)[0]
def u32(x): return unpack("<I", x)[0]


def Hexdump(source, length=0x10, separator=".", base=0x00, align=10):
    result = []
    for i in range(0, len(source), length):
        chunk = bytearray(source[i:i + length])
        hexa = " ".join(["%.02x" % b for b in chunk])
        text = "".join([chr(b) if 0x20 <= b < 0x7F else separator for b in chunk])
        msg = "{addr:#0{aw}x}     {data:<{dw}}    {text}".format(aw=align,addr=base+i,dw=3*length,data=hexa,text=text)
        result.append(msg)
    return "\n".join(result)


def FormatMessage(dwMessageId):
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


TEST_DRIVER_NAME = "\\driver\\vboxguest"
BUFSIZE = 4096



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
        szBuf = create_string_buffer(12)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, 12, byref(cbRead), None)
         # hookdriver doesn't return any data (header only = 2*uint32_t + uint32_t for GLE)
        assert cbRead.value == 12
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        #dwGle = u32(szBuf[8:12]); assert dwGle == ERROR_SUCCESS, "test_HookDriver::GetLastError() = 0x%x (%s)" % (dwGle, FormatMessage(dwGle))
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
        szBuf = create_string_buffer(12)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, 12, byref(cbRead), None)
        assert cbRead.value == 12
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        #dwGle = u32(szBuf[8:12]); assert dwGle == ERROR_SUCCESS, "test_UnhookDriver::GetLastError() = 0x%x (%s)" % (dwGle, FormatMessage(dwGle))
        return


    def test_EnableMonitoring(self):
        ## send request
        tlv_msg = PrepareTlvMessage(TaskType.EnableMonitoring)
        cbWritten = c_ulong(0)
        assert windll.kernel32.WriteFile(self.hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
        assert len(tlv_msg) ==  cbWritten.value

        ## recv response
        cbRead = c_ulong(0)
        szBuf = create_string_buffer(12)
        assert windll.kernel32.ReadFile(self.hPipe, szBuf, 12, byref(cbRead), None)
        assert u32(szBuf[0:4]) == TaskType.IoctlResponse.value
        assert u32(szBuf[4:8]) == 4
        dwGle = u32(szBuf[8:12]); assert dwGle == ERROR_SUCCESS, "test_EnableMonitoring::GetLastError() = 0x%x (%s)" % (dwGle, FormatMessage(dwGle))
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
        dwGle = u32(szBuf[8:12]); assert dwGle == ERROR_SUCCESS, "test_DisableMonitoring::GetLastError() = 0x%x (%s)" % (dwGle, FormatMessage(dwGle))
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
        dwGle = u32(szBuf[8:12]); assert dwGle == ERROR_SUCCESS, "test_GetInterceptedIrps::GetLastError() = 0x%x (%s)" % (dwGle, FormatMessage(dwGle))
        assert json.loads(szBuf[12:cbRead.value])
        return


if __name__ == '__main__':
    r = BrokerTestMethods()
    r.test_OpenPipe()
    print("OpenPipe() success")
    r.test_HookDriver()
    print("OpenPipe() success")
    r.test_EnableMonitoring()
    print("EnableMonitoring() success")
    #r.test_GetInterceptedIrps()
    #print("GetInterceptedIrps() success")
    r.test_DisableMonitoring()
    print("DisableMonitoring() success")
    r.test_UnhookDriver()
    print("UnhookDriver() success")
    r.test_ClosePipe()
    print("ClosePipe() success")
