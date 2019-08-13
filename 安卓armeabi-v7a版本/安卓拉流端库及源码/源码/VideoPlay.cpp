#include "VideoPlay.h"

#include <thread>
//using namespace std;


VideoPlay::VideoPlay(QWidget *parent)
    : QWidget(parent)
{
    this->show();
}
void VideoPlay::paintEvent(QPaintEvent *e)
{

    if (!_curRgb24 || !painting_pic_flag) {
		return;
	}
    QImage img(_curRgb24->data, _curRgb24->w, _curRgb24->h, QImage::Format_RGB888);
    img = img.scaled(1280,960);
    QPainter p(this);
	p.drawImage(0,0,img);
	//CUR;
    free_Rgb24(_curRgb24);
	//CUR;
	painting_pic_flag = false;

	//paintMutex.unlock();
	//CUR;
	//paintMutex.unlock();

}
void VideoPlay::paintRgb24(Rgb24 *rgb24)
{
    if (!rgb24)
        return;
    _curRgb24 = rgb24;
    painting_pic_flag = true;
    update();
}


VideoPlay::~VideoPlay()
{

}

