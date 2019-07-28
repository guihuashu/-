#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H
#include <opencv2/highgui.hpp>

/* 用相机上支持的某种格式去采集 */
typedef enum CapType {
	MJPEG,
	RGB
}CapType;

class VideoCap
{
public:
	VideoCap();
	bool set_videoCapture();
	bool init_videoCap();
	cv::Mat *get_mat();
	int get_width();
	int get_height();
	~VideoCap();
private:
	cv::VideoCapture camera;	// opencv相机
	cv::Mat mat;				// opencv相机一帧
	int width = 640;			// 设置相机输入宽度
	int height = 480;			// 设置相机输入高度
	//int fps = 0;				// 设置相机的输入fps
	//CapType capType;			// 
};

#endif // VIDEOCAPTURE_H