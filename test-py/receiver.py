from smipc import *


def print_buffer(c_buf, sz):
    p = addressof(c_buf)
    print(string_at(p), sz)


cid = b'test-chan'

init_library()

with open_channel(cid, CHAN_R, 128) as c:
    buf = create_string_buffer(9)
    c.print_status()
    for i in range(3):
        c.read(buf, 9, True)
        print_buffer(buf, 9)
        c.print_status()

clean_library()
