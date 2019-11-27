from smipc import *


cid = b'test-chan'
chanSz = 20
init_library(LOG_ALL)
with open_channel(cid, CHAN_W, chanSz) as c:
    c.print_status()
    c.write(b'hi')
    c.print_status()
clean_library()
