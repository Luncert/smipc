from smipc import *
import random


cid = b'test-chan'
chanSz = 20
dataSz = 64
init_library(LOG_ALL)
testData = bytes([random.randint(0, 255) for _ in range(dataSz)])
with open_channel(cid, CHAN_W, chanSz) as c:
    c.print_status()
    c.write(testData)
    c.print_status()
print(testData)
clean_library()
