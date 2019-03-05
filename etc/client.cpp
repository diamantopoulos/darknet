//!2017.12.19 19:19:01 CST
//!2017.12.19 22:19:38 CST
//!2017.12.19 22:39:37 CST
// 客户端
// 使用 OpenCV 读取视频（并处理），然后使用 SOCKET 上传到服务器。

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
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>


#define TCP_SOCK

using namespace std;
using namespace cv;

double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int main()
{
    int sock;
    struct sockaddr_in addr, server_addr;
    double time_start, time_end, time_elapsed;
#ifdef TCP_SOCK
    sock = socket(AF_INET, SOCK_STREAM, 0);
#else
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if(sock < 0){
        perror("socket");
        exit(1);
    }

    const char *IP = "9.4.125.67";
    //const char *IP = "9.4.69.71";

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555); // 3425
    addr.sin_addr.s_addr = inet_addr(IP); //htonl(INADDR_LOOPBACK)
#ifdef TCP_SOCK
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("connect");
        exit(2);
    }
#endif
    int bbytee;
    cout << "before open the cam" << endl;

    VideoCapture cap(0);

    if(!cap.isOpened()) {
        cout<< "Could not open the camera" <<  endl;
        close(sock);
        return -1;
    }


    int Y=480;  //480
    int X=640; //640
    Mat frame, frame_recv;
    frame = Mat::zeros(Y, X, CV_8UC3);
    frame_recv = Mat::zeros(Y, X, CV_8UC3);
    int imgSize = frame.cols*frame.rows*3;
    int imgSizeBytes = frame.total()*frame.elemSize();
    std::ostringstream str;

    int cnt=0;
    unsigned int total_bytes = 2*imgSize;
    float bandwidth, fps;
    //Mat frame;
    while(1) {
        cap >> frame;
        if(frame.empty()) {
            cerr<<"[client] VideoCapture(0) error!"<<endl;
        }

        //cout<< ++cnt << ":"<< frame.isContinuous()<<"," <<frame.size()<<endl;;
        //cout<< "1" << endl;

        time_start = what_time_is_it_now();

#ifdef TCP_SOCK
        if( (bbytee = send(sock, frame.data, imgSize, 0)) < 0 ) {
            cout<< "2" << endl;
	          cerr<< "bytes = " << bbytee << endl;
            break;
        }
#else
        if (bbytee = sendto(sock, frame.data, imgSize, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0 ) {
            cout<< "2" << endl;
            cerr<< "bytes = " << bbytee << endl;
            //break;
        }
#endif

	      //cout<< "3" << endl;
        if( (bbytee = recv(sock, frame_recv.data, imgSize, MSG_WAITALL)) == -1 ) {
            cout<< "2" << endl;
            cerr<< "bytes = " << bbytee << endl;
            break;
        }

        time_end = what_time_is_it_now();
        time_elapsed = time_end - time_start;
        bandwidth = (total_bytes / time_elapsed) / 1e6;
        fps = 1 / (time_elapsed);
        cout << fixed << setw(5) << setprecision(3) << "FPS: " << fps << ", BW: " << bandwidth << " MB/s (" << bandwidth*8 << " Mbps)" << endl;
        str << "FPS: " << fps << ", BW: " << bandwidth << " MB/s (" << bandwidth*8 << " Mbps)";
        //cout<< "4" << endl;
        cv::putText(frame_recv,
                    str.str(),
                    cv::Point(0,15), // Coordinates
                    cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                    1.0, // Scale. 2.0 = 2x bigger
                    cv::Scalar(255,0,0));

        cv::imshow("Sent stream", frame);
        cv::imshow("Received stream", frame_recv);
        if(cv::waitKey(1) == 'q') {
            break;
        }
        str.str("");

    }
    close(sock);
    return 0;
}
