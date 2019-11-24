from smipc import *

cid = b'test-chan'

init_library()

with open_channel(cid, CHAN_W, 128) as c:
    c.print_status()
    for i in range(3):
        c.write("test/id=" + str(i))
        c.print_status()

clean_library()
