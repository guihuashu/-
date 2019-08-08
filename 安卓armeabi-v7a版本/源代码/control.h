#ifndef CONTROL_H
#define CONTROL_H
#include <QThreadPool>
#include <QVideoFrame>
#include <QObject>
#include <thread>

#include <qtvideocap.h>
#include <pktlist.h>
#include <MediaEncode.h>
#include <task.h>
#include <debug.h>
#include <OutMedia.h>
#include <string>
#include <showvcap.h>



class Control : QObject
{
    Q_OBJECT
public:
    Control();
    bool init();

public slots:
    void newVideoFrame(const QVideoFrame &frame);
    void pushStream();
    void setVParam();
    void setAParam();
    void set_paintVcapFlag(bool flag);  // 设置是否显示采集视频的标记
signals:
    void showVideoFrame(QVideoFrame frame);

public:
    int TaskThreadNum = 50; // 处理任务的最多线程数

    // 包缓存队列参数
    PktList *_aPktList;
    PktList *_vPktList;
    unsigned int _aPktListSize = 10;
    unsigned int _vPktListSize = 3;

    // 视频采集参数
    QSize _inSize = {320,240};
    QSize _outSize = _inSize;
    AVPixelFormat _inFmt = AV_PIX_FMT_NV21;
    AVPixelFormat _outFmt = AV_PIX_FMT_YUV420P;
    QtVideoCap *_cap;

    // 音视频编码参数
    MediaEncode *_encode;
    vEncodeArgs _vArgs;
    aEncodeArgs _aArgs;

    // 推流参数
    OutMedia *outMedia;
    string outUr = "rtmp://hdlcontrol.com/live/stream";
    string streamFmt = "flv";
    QThreadPool *_pool;

    // 显示采集视频
    ShowVCap *_showVCap;
    bool _paintVcapFlag;    // 是否显示视频

};

#endif // CONTROL_H
