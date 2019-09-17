
"""

Simple command line client to test the broker exposed features.

This shouldn't be used in other cases than testing.

"""

from enum import Enum, unique
from ctypes import *
from struct import *

def ok(x): print("[+] {0}".format(x))
def p16(x): return pack("<H", x)
def p32(x): return pack("<I", x)
def u16(x): return unpack(">H", x)[0]
def u32(x): return unpack(">I", x)[0]

def hexdump(source, length=0x10, separator=".", base=0x00, align=10):
    result = []

    for i in range(0, len(source), length):
        chunk = bytearray(source[i:i + length])
        hexa = " ".join([b for b in chunk])
        text = "".join([chr(b) if 0x20 <= b < 0x7F else separator for b in chunk])
        result.append("{addr:#0{aw}x}     {data:<{dw}}    {text}".format(aw=align,
                                                                         addr=base+i,
                                                                         dw=3*length,
                                                                         data=hexa,
                                                                         text=text)
                      )
    return "\n".join(result)


TEST_DRIVER_NAME = b"vboxguest.sys"


@unique
class TaskType(Enum):
    IoctlResponse = 1
    HookDriver = 2
    UnhookDriver = 3
    GetDriverInfo = 4
    NumberOfDriver = 5
    NotifyEventHandle = 6
    EnableMonitoring = 7
    DisableMonitoring = 8



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



def PipeConnect():
    """
    Performs the following actions:
    1. connect to the pipe
    2. hook a test driver (set by global `TEST_DRIVER_NAME`)
    3. enable monitoring
    4. read one dumped IRP
    5. disable monitoring
    6. unhook test driver
    7. close handle
    """

    # 0. define some windows constants
    GENERIC_READ = 0x80000000
    GENERIC_WRITE = 0x40000000
    OPEN_EXISTING = 0x3
    INVALID_HANDLE_VALUE = -1
    PIPE_READMODE_MESSAGE = 0x2
    ERROR_PIPE_BUSY = 231
    ERROR_MORE_DATA = 234
    BUFSIZE = 512


    # 1. connect to the pipe
    ok("Starting Step 1")

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

    ## extra pipe mode setting
    dwMode = c_ulong(PIPE_READMODE_MESSAGE)
    windll.kernel32.SetNamedPipeHandleState(hPipe, byref(dwMode), None, None)

    ok("Step 1 ok")


    # 2. hook vboxguest.sys
    ok("Starting Step 2")

    ## send request
    tlv_msg = PrepareTlvMessage(TaskType.HookDriver, TEST_DRIVER_NAME)
    cbWritten = c_ulong(0)
    bSuccess = windll.kernel32.WriteFile(hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)
    assert bSuccess 
    assert len(tlv_msg) == cbWritten.value
    ok("data sent: %d" % cbWritten.value)


    ## recv response
    cbRead = c_ulong(0)
    szBuf = create_string_buffer(BUFSIZE)
    bSuccess = windll.kernel32.ReadFile(hPipe, szBuf, BUFSIZE, byref(cbRead), None)
    assert bSuccess
    ok("data recv: %d" % cbRead.value)

    assert cbRead.value == 0 # hookdriver doesn't return any data

    ok("Step 2 ok")
    

    # 3. enable monitoring
    ok("Starting Step 3")

    tlv_msg = PrepareTlvMessage(TaskType.EnableMonitoring)
    cbWritten = c_ulong(0)
    
    bSuccess = windll.kernel32.WriteFile(hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)

    assert bSuccess 
    assert len(tlv_msg) == cbWritten.value

    ok("Step 3 ok")


    # 4. read one dumped IRP
    #ok("Starting Step 4")
    #chBuf = create_string_buffer(BUFSIZE)
    #cbRead = c_ulong(0)
    #fSuccess = windll.kernel32.ReadFile(hPipe, chBuf, BUFSIZE, byref(cbRead), None)
    #ok("Step 4 ok")


    # 5. disable monitoring
    ok("Starting Step 5")

    tlv_msg = PrepareTlvMessage(TaskType.DisableMonitoring)
    cbWritten = c_ulong(0)  
    bSuccess = windll.kernel32.WriteFile(hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)

    assert bSuccess 
    assert len(tlv_msg) == cbWritten.value

    ok("Step 5 ok")


    # 6. unhook test driver
    ok("Starting Step 6")

    tlv_msg = PrepareTlvMessage(TaskType.UnhookDriver, TEST_DRIVER_NAME)
    cbWritten = c_ulong(0)
    bSuccess = windll.kernel32.WriteFile(hPipe, tlv_msg, len(tlv_msg), byref(cbWritten), None)

    assert bSuccess 
    assert len(tlv_msg) == cbWritten.value

    ok("Step 6 ok")


    # 7. close handle
    ok("Starting Step 7")
    windll.kernel32.CloseHandle(hPipe)
    ok("Step 7 ok")

    return True


if __name__ == "__main__":
    assert PipeConnect()