import sys

sys.path.append('..')

from smipc import *


cid = b'test-chan'
chanSz = 20
dataSz = 1024
init_library()
buf = create_string_buffer(dataSz)
with open_channel(cid, CHAN_R, chanSz) as c:
    c.print_status()
    n = c.read(buf, dataSz, True)
    print('read', n)
    c.print_status()
print(bytes(buf))
clean_library()
