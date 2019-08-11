#ifndef TASK_H
#define TASK_H

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}
#include <QSize>
#include <QVideoFrame>
#include <QDebug>
#include <iostream>
#include <QRunnable>
#include <QThread>

#include <ffmopr.h>
#include <pktlist.h>
#include <MediaEncode.h>
#include <list>
#include <qtaudiorecord.h>



class VTask : public QRunnable
{
public:
    VTask(QVideoFrame &curFrame, MediaEncode *encode, PktList *vlist);
    ~VTask();
    virtual void run();

private:
    QVideoFrame _curFrame;
    MediaEncode *_encode;
    SwsContext *_swsNv21toYuv420p = NULL;
    //SwsContext *_swsNv21toBgr24 = NULL;
    //SwsContext *_swsBrg24toYuv420p = NULL;

    PktList *_vlist;
};

class ATask : public QRunnable
{
public:
    ATask(AudioFrame &aframe,  MediaEncode *encode, PktList *alist);
    ~ATask();
    virtual void run();
private:
    AudioFrame _audioFrame;
    MediaEncode *_encode;
    PktList *_alist;
};


#endif // TASK_H







