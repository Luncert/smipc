from ctypes import *

_lib = CDLL("../core-dist/libsmipc.dll")

_OP_SUCCEED = 0
_OP_FAILED = -1
_OPPOSITE_END_CLOSED = -2

CHAN_R = 0
CHAN_W = 1


def init_library(allow_log=True):
    _lib.initLibrary(1 if allow_log else 0)


def clean_library():
    _lib.cleanLibrary()


def _check_ret(ret, msg):
    if ret != _OP_SUCCEED:
        raise OperationFailedError(msg + ' Ret=' + str(ret))


class SmipcError(Exception):
    pass


class ParameterTypeError(SmipcError):
    pass


class OppositeEndClosedError(SmipcError):
    def __init__(self):
        super().__init__("Opposite end has been closed.")


class OperationFailedError(SmipcError):
    pass


class Channel(object):
    def __init__(self, cid, mode, chan_sz=128):
        if isinstance(cid, str):
            cid = cid.encode('utf-8')
        elif not isinstance(cid, bytes):
            raise ParameterTypeError('cid must be str or bytes.')
        self.cid = cid
        self.mode = mode
        self.chan_sz = chan_sz

    def __enter__(self):
        ret = _lib.openChannel(self.cid, self.mode, self.chan_sz)
        _check_ret(ret, "Failed to open channel.")
        return self

    def write(self, data):
        if isinstance(data, str):
            data = data.encode('utf-8')
        elif not isinstance(data, bytes):
            raise ParameterTypeError('Data must be str or bytes.')
        ret = _lib.writeChannel(self.cid, data, len(data))
        if ret == _OPPOSITE_END_CLOSED:
            raise OppositeEndClosedError()
        elif ret == _OP_FAILED:
            raise OperationFailedError("Failed to write channel.")

    def read(self, buf, n, blocking=False):
        """
        read channel
        :param buf: buf (create by ctypes.create_string_buffer)
        :param n: num bytes wanna read
        :param blocking: True or False
        :return: num bytes read
        """
        ret = _lib.readChannel(self.cid, buf, n, 1 if blocking else 0)
        if ret == _OPPOSITE_END_CLOSED:
            raise OppositeEndClosedError()
        elif ret == _OP_FAILED:
            raise OperationFailedError("Failed to read channel.")
        return n

    def print_status(self):
        ret = _lib.printChannelStatus(self.cid)
        _check_ret(ret, "Failed to print channel status.")

    def __exit__(self, exc_type, exc_val, exc_tb):
        ret = _lib.closeChannel(self.cid)
        _check_ret(ret, "Failed to close channel.")


def open_channel(cid, mode, chan_sz):
    return Channel(cid, mode, chan_sz)

