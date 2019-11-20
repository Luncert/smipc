from ctypes import *

lib = cdll.LoadLibrary('cmake-build-debug/libsmipc.dll')
lib.testMap()