import cv2
import socket
from PIL import Image
import numpy as np
import time

def recvall(sock, n):
    # Helper function to recv n bytes or return None if EOF is hit
    data = b''
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data += packet
    return data

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('9.4.125.67', 5555))
#connection = client_socket.makefile('wb')

width=640
height=480
cam = cv2.VideoCapture(0)

cam.set(3, width);
cam.set(4, height);

img_counter = 0

while True:
    ret, frame = cam.read()

    size = len(frame)

    print("{}: {}".format(img_counter, len(frame.data)))
    start_time = time.time()

    bbytee = client_socket.send(frame.data)
    if (bbytee < 0):
        print("send failed")
    else:
        print("sent bytes " + str(bbytee))

    data_received = recvall(client_socket, bbytee)

    time_elapsed = time.time() - start_time
    bandwidth = (bbytee*2*4 / time_elapsed) / 1e6;
    fps = 1 / (time_elapsed);
    print("fps= " + str(fps) + ", BW= " + str(bandwidth) + " MB/s")

    if (bbytee < 0):
        print("recv failed")
    #else:
    #    print("recv bytes " + str(len(data_received)))

    imgSize = (width,height)# the image size
    img = Image.frombytes('RGB', imgSize, data_received)
    imgnp = np.asarray(img)
    #print(type(imgnp))
    #print(str(len(imgnp)))

    cv2.imshow('ImageWindow',imgnp)
    cv2.waitKey(1)

    img_counter += 1

cam.release()
