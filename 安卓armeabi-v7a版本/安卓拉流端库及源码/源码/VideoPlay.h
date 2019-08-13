#ifndef VIDEO_PLAY
#define VIDEO_PLAY

#include <QWidget>
#include <QPainter>
#include <QMutex>
#include <Decode.h>
#include <mutex>
#include <QImage>
#include <QThreadPool>
#include <QRunnable>
#include <QtWidgets/QHBoxLayout>

#include <QPainter>
#include <test.h>
#include <QLabel>
//#include <ui_Widget.h>


class VideoPlay : public QWidget
{
	Q_OBJECT
public:
	VideoPlay(QWidget *parent = 0);
	virtual void paintEvent(QPaintEvent *e);
    void paintRgb24(Rgb24 *rgb24);
	std::mutex paintMutex;
	virtual ~VideoPlay();
	QThreadPool *pool;
    Rgb24 *_curRgb24 = NULL;
	bool painting_pic_flag = false;
private:
};


#endif // VIDEO_PLAY
