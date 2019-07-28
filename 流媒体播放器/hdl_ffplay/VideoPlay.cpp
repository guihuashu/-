#include "VideoPlay.h"

#include <thread>
//using namespace std;


VideoPlay::VideoPlay(QWidget *parent)
{
	this->show();
}
void VideoPlay::paintEvent(QPaintEvent *e)
{

	if (!curRgb32 || !painting_pic_flag) {
		return;
	}
	//CUR;
	//paintMutex.lock();
	//CUR;
	QImage img(curRgb32->data, curRgb32->w, curRgb32->h, QImage::Format_RGB32);
	QPainter p(this);
	p.drawImage(0,0,img);
	//CUR;
	free_Rgb32(curRgb32);
	//CUR;
	curRgb32 = NULL;
	painting_pic_flag = false;

	//paintMutex.unlock();
	//CUR;
	//paintMutex.unlock();

}
void VideoPlay::paintRgb32(Rgb32 *rgb32)
{
	//paintMutex.lock();
	//CUR;
	if (!rgb32)
		return;
	curRgb32 = rgb32;
	painting_pic_flag = true;
	update();
	//repaint();	// 立即绘制
	//CUR;
}


VideoPlay::~VideoPlay()
{

}

