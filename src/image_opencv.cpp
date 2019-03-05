#ifdef OPENCV

#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "image.h"

using namespace cv;

extern "C" {

IplImage *image_to_ipl(image im)
{
    int x,y,c;
    IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    int step = disp->widthStep;
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    return disp;
}

image ipl_to_image(IplImage* src)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image im = make_image(w, h, c);
    unsigned char *data = (unsigned char *)src->imageData;
    int step = src->widthStep;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return im;
}

Mat image_to_mat(image im)
{
    image copy = copy_image(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    IplImage *ipl = image_to_ipl(copy);
    Mat m = cvarrToMat(ipl, true);
    cvReleaseImage(&ipl);
    free_image(copy);
    return m;
}

image mat_to_image(Mat m)
{
    IplImage ipl = m;
    image im = ipl_to_image(&ipl);
    rgbgr_image(im);
    return im;
}

void *open_video_stream(const char *f, int c, int w, int h, int fps)
{
    VideoCapture *cap;
    if(f) cap = new VideoCapture(f);
    else cap = new VideoCapture(c);
    if(!cap->isOpened()) return 0;
    if(w) cap->set(CV_CAP_PROP_FRAME_WIDTH, w);
    if(h) cap->set(CV_CAP_PROP_FRAME_HEIGHT, w);
    if(fps) cap->set(CV_CAP_PROP_FPS, w);
    return (void *) cap;
}

image get_image_from_stream(void *p)
{
    VideoCapture *cap = (VideoCapture *)p;
    Mat m;
    *cap >> m;
    if(m.empty()) return make_empty_image(0,0,0);
    return mat_to_image(m);
}


#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <cstdio>


#define TCP_SOCK

    int global_once = 0;
    int sock, listener;
    struct sockaddr_in addr, client_addr;
    unsigned int addr_len;

image get_image_from_stream_over_net(void *p, image im)
{








    if (global_once == 0) {
    //int sock, listener;
    //struct sockaddr_in addr;
    std::cerr << "canme to global_once" << std::endl;

#ifdef TCP_SOCK
    if( (listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[server] socket() failed");
        exit(1);
    }
#else
    if ((listener = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    addr_len = sizeof(struct sockaddr);
#endif
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555); // 3425
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[server] binding faild!");
        exit(2);
    }
#ifdef TCP_SOCK
    listen(listener, 1);
#endif
    }

    int X = 640; //640
    int Y = 480;  //480

    int num_of_recv_bytes, num_of_send_bytes;
    //VideoWriter outputVideo;
    Size S = Size((int) X,(int) Y);
    //outputVideo.open("receive.avi", CV_FOURCC('M','J','P','G'), 30, S, true);

    int imgSize = Y*X*3;
    Mat frame = Mat::zeros(Y, X, CV_8UC3);
    //Mat frame_draw = Mat::zeros(Y, X, CV_8UC3);
    Mat frame_draw_resized = Mat::zeros(Y, X, CV_8UC3);
    uchar *iptr = frame.data;

    int key;

    int cnt=0;
    //while(1){
     if (global_once == 0) {

        std::cout << ++cnt<<std::endl;
#ifdef TCP_SOCK
        sock = accept(listener, NULL, NULL);
        if(sock < 0){
            perror("[server] accept() faild!");
            exit(3);
        }
#endif
	global_once++;
     }

        while(key != 'q') {
#ifdef TCP_SOCK
            if( num_of_recv_bytes = recv(sock, iptr, imgSize, MSG_WAITALL) == -1 ) {
                std::cerr << "tcp recv failed, received bytes = " << num_of_recv_bytes << std::endl;
            }
#else
          if( num_of_recv_bytes = recvfrom(sock, iptr, imgSize, 0, (struct sockaddr *)&client_addr, &addr_len) == -1 ) {
                std::cerr << "udp recv failed, received bytes = " << num_of_recv_bytes << std::endl;
            }
#endif

            Mat frame_draw = image_to_mat(im);

           // Resize src so that is has the same size as img
           //printf("height:%d, width=%d\n", frame_draw.size().height, frame_draw.size().width);
           if (frame_draw.size().height > 0)
                resize(frame_draw, frame_draw_resized, frame.size());

            uchar *iptr_draw= frame_draw_resized.data;
            if( num_of_send_bytes = send(sock, iptr_draw, imgSize, 0) < 0 ) {
                std::cerr << "send failed, send bytes = " << num_of_send_bytes << std::endl;
            }

	    std::cout << "waiting, received bytes = " << num_of_recv_bytes << std::endl;

            //outputVideo<< frame;
            //imshow("server", frame);
            //if (key = waitKey(100) >= 0) break;
            break;
        }
        //outputVideo.release();
        //close(sock);
        //break;
    //}
    //return 0;










    //VideoCapture *cap = (VideoCapture *)p; //p
    Mat m = frame;
    //*cap >> m;
    if(m.empty()) {
        std::cout << "image is empty" << std::endl;
        return make_empty_image(0,0,0);
    }
    return mat_to_image(m);
}

image load_image_cv(char *filename, int channels)
{
    int flag = -1;
    if (channels == 0) flag = -1;
    else if (channels == 1) flag = 0;
    else if (channels == 3) flag = 1;
    else {
        fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
    }
    Mat m;
    m = imread(filename, flag);
    if(!m.data){
        fprintf(stderr, "Cannot load image \"%s\"\n", filename);
        char buff[256];
        sprintf(buff, "echo %s >> bad.list", filename);
        system(buff);
        return make_image(10,10,3);
        //exit(0);
    }
    image im = mat_to_image(m);
    return im;
}

int show_image_cv(image im, const char* name, int ms)
{
    Mat m = image_to_mat(im);
    imshow(name, m);
    int c = waitKey(ms);
    if (c != -1) c = c%256;
    return c;
}

void make_window(char *name, int w, int h, int fullscreen)
{
    namedWindow(name, WINDOW_NORMAL); 
    if (fullscreen) {
        setWindowProperty(name, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    } else {
        resizeWindow(name, w, h);
        if(strcmp(name, "Demo") == 0) moveWindow(name, 0, 0);
    }
}

}

#endif
