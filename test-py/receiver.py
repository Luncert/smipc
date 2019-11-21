from smipc import *

cid = b'test-chan'

initLibrary()
openChannel(cid, CHAN_R, 128)

buf = b'\0' * 9
readChannel(cid, buf, 9, True)
printChannelStatus(cid)

closeChannel(cid)
cleanLibrary()