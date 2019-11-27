from smipc import *
import time


def on_data(data):
    print('read', data)


cid = b'test-chan'
chanSz = 20

init_library(LOG_ALL)
with open_channel(cid, CHAN_R, chanSz) as c:
    t = c.read_async(on_data)
    time.sleep(3)
    t.stop()
    c.print_status()
clean_library()
