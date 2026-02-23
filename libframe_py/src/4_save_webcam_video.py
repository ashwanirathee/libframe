import numpy as np
import cv2 as cv
 
cap = cv.VideoCapture(0)
 
# Define the codec and create VideoWriter object
# MORE INFO HERE: https://docs.opencv.org/4.x/dd/d43/tutorial_py_video_display.html
fourcc = cv.VideoWriter_fourcc(*'DIVX')
out = cv.VideoWriter('vtest.avi', fourcc, 20.0, (1280, 720))
 
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break
    frame = cv.flip(frame, 0)
 
    # write the flipped frame
    out.write(frame)
 
    cv.imshow('frame', frame)
    if cv.waitKey(1) == ord('q'):
        break
 
# Release everything if job is finished
cap.release()
out.release()
cv.destroyAllWindows()