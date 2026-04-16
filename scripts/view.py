import numpy as np
import cv2

w, h = 640, 480

img = np.fromfile("data/output/out.rgb", dtype=np.uint8)
img = img.reshape((h, w, 3))

cv2.imwrite("out.png", img)

cv2.imshow("gray", img)
cv2.waitKey(0)
cv2.destroyAllWindows()