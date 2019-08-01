#include "VideoCap.h"
#include <iostream>
using namespace std;

//using namespace cv;

VideoCap::VideoCap()
{
	cout << "---------------  VideoCapture  --------" << endl;
}

bool VideoCap::init_videoCap()
{
	//camera.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
	//camera.set(CV_CAP_PROP_FPS, fps);			// 帧率, 设置后摄像头有延迟
	camera.set(CV_CAP_PROP_FRAME_WIDTH, width);  //宽度 
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, height); //高度
	return camera.open(cv::CAP_ANY);			// 让系统自动探测摄像头的类型
}
cv::Mat *VideoCap::get_mat()
{
	if (!camera.read(mat))
		return NULL;
	return &mat;
}

int VideoCap::get_width()
{
	return this->width;
}
int VideoCap::get_height()
{
	return this->height;
}


VideoCap::~VideoCap()
{
}
