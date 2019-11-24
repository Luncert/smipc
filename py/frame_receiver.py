import cv2
import smipc
import time
from frame_transport import FrameTransport

chan_sz = (1920 * 1080 + 4) * 10

smipc.init_library()
with FrameTransport("frame-transport", smipc.CHAN_R, chan_sz) as ft:
    count = 10
    while count > 0:
        st = time.time()
        frame = ft.read_frame()
        print('rt: %f' % (time.time() - st))
        count -= 1
        cv2.imshow("Frame", frame)
        if cv2.waitKey(1) & 0xff == 'q':
            break
smipc.clean_library()
cv2.destroyAllWindows()
