import cv2
import smipc
import time
from frame_transport import FrameTransport

# smipc.init_library()
# t = FrameTransport("frame-transport", smipc.CHAN_W, 1920 * 1080 * 3)
# t.open()
# source = cv2.VideoCapture("demo.mp4")
# try:
#     while source.isOpened():
#         ret, f = source.read()
#         gray = cv2.cvtColor(f, cv2.COLOR_BGR2GRAY)
#         t.send_frame(gray)
# except Exception as e:
#     if isinstance(e, KeyboardInterrupt):
#         print("Exit gracefully.")
#     else:
#         raise e
# finally:
#     t.close()
#     source.release()
#     smipc.clean_library()

chan_sz = (1920 * 1080 + 4) * 10

smipc.init_library()
source = cv2.VideoCapture("demo.mp4")
with FrameTransport("frame-transport", smipc.CHAN_W, chan_sz) as ft:
    count = 10
    while count > 0:
        _, f = source.read()
        gray = cv2.cvtColor(f, cv2.COLOR_BGR2GRAY)
        st = time.time()
        ft.send_frame(gray)
        print('wt: %f' % (time.time() - st))
        count -= 1
source.release()
smipc.clean_library()
