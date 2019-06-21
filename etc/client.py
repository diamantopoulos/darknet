import cv2
import io
import socket
import struct
import time
import pickle
import zlib
from PIL import Image
import numpy as np

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

cam = cv2.VideoCapture(0)

cam.set(3, 640);
cam.set(4, 480);

img_counter = 0

while True:
    ret, frame = cam.read()

    size = len(frame)

    print("{}: {}".format(img_counter, size))
    bbytee = client_socket.send(frame.data)
    if (bbytee < 0):
        print("send failed")
    else:
        print("sent bytes " + str(bbytee))

    data_received = recvall(client_socket, bbytee)
#    client_socket.recvall(data_received, bbytee)
    if (bbytee < 0):
        print("recv failed")
    else:
        print("recv bytes " + str(len(data_received)))

#    frame.data = data_received

    imgSize = (640,480)# the image size
    img = Image.frombytes('RGB', imgSize, data_received)
    imgnp = np.asarray(img)
    cv2.imshow('ImageWindow',imgnp)
    cv2.waitKey(1)

    img_counter += 1

cam.release()
