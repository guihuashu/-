#ifndef TEST_H
#define TEST_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/mediacodec.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>

}
#include <opencv2/highgui.hpp>		//imread, 包含界面的一些文件
#include <list>
#include <thread>
#include <mutex>
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QThread>
#include <QIODevice>
#include <QImage>
#include <Widget.h>
#include <QThreadPool>

void dump_ffmpeg_verson();
int test(Widget *w);

#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

#define CUR qDebug()<<__FILE__<<","<<__LINE__



#endif
