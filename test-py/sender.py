from smipc import *

cid = b'test-chan'

initLibrary()
openChannel(cid, CHAN_W, 128)

writeChannel(cid, b"test/id=1", 9)
printChannelStatus(cid)

closeChannel(cid)
cleanLibrary()