from ctypes import *

__lib = CDLL("../src/cmake-build-debug/libsmipc.dll")

OP_SUCCEED = 0
OP_FAILED = -1
OPPOSITE_END_CLOSED = -2

CHAN_R = 0
CHAN_W = 1

def initLibrary():
    __lib.initLibrary()

def cleanLibrary():
    __lib.cleanLibrary()

__openChannel = __lib.openChannel
__openChannel.argtypes = [c_char_p, c_int, c_int]
__openChannel.restype = c_int
def openChannel(cid, mode, chanSz=128):
    return __openChannel(cid, mode, chanSz)

__writeChannel = __lib.writeChannel
__writeChannel.argtypes = [c_char_p, c_char_p, c_int]
__writeChannel.restype = c_int
def writeChannel(cid, data, len):
    return __writeChannel(cid, data, len)

__readChannel = __lib.readChannel
__readChannel.argtypes = [c_char_p, c_char_p, c_int, c_int]
def readChannel(cid, buf, n, blocking=False):
    return __readChannel(cid, buf, n, 1 if blocking else 0)

__printChannelStatus = __lib.printChannelStatus
__printChannelStatus.argtypes = [c_char_p]
__printChannelStatus.restype = c_int
def printChannelStatus(cid):
    return __printChannelStatus(cid)

__closeChannel = __lib.closeChannel
__closeChannel.argtypes = [c_char_p]
__closeChannel.restype = c_int
def closeChannel(cid):
    return __closeChannel(cid)