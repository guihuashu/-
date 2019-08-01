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

#include <QPainter>
#include <test.h>


class VideoPlay : public QWidget
{
	Q_OBJECT
public:
	VideoPlay(QWidget *parent = 0);
	virtual void paintEvent(QPaintEvent *e);
	void paintRgb32(Rgb32 *rgb32);
	std::mutex paintMutex;
	virtual ~VideoPlay();
	QThreadPool *pool;
	Rgb32 *curRgb32 = NULL;
	bool painting_pic_flag = false;
};


#endif // VIDEO_PLAY