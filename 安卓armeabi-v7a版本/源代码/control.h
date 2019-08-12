#ifndef CONTROL_H
#define CONTROL_H
#include <QThreadPool>
#include <QVideoFrame>
#include <QObject>
#include <thread>
#include <string>

#include <qtvideocap.h>
#include <pktlist.h>
#include <MediaEncode.h>
#include <task.h>
#include <debug.h>
#include <OutMedia.h>
#include <showvcap.h>
#include <qtaudiorecord.h>


class Control : QObject
{
    Q_OBJECT
public:
    Control();
    bool init();
    void set_paintVcapFlag(bool flag);  // 设置是否显示采集视频的标记

public slots:
    void newVideoFrame(const QVideoFrame &frame);   // 采集到视频帧
    void newAudioFrame(AudioFrame &audioFrame);     // 采集到音频帧
    void pushStream();
    void setVargs();
    void setAargs();

signals:
    void showVideoFrame(QVideoFrame frame);

public:
    int TaskThreadNum = 50; // 处理任务的最多线程数

    // 包缓存队列参数
    PktList *_aPktList;
    PktList *_vPktList;
    unsigned int _aPktListSize = 1;
    unsigned int _vPktListSize = 1;

    // 视频采集参数
    QSize _inSize = {640,480};
    QSize _outSize = _inSize;
    AVPixelFormat _inFmt = AV_PIX_FMT_NV21;
    AVPixelFormat _outFmt = AV_PIX_FMT_YUV420P;
    QtVideoCap *_vCap;

    // 音频采集参数
    QtAudioRecord *_aCap;

    // 音视频编码参数
    MediaEncode *_encode;
    vEncodeArgs _vArgs;
    aEncodeArgs _aArgs;

    // 推流参数
    OutMedia *_outMedia;
    string _outUr = "rtmp://hdlcontrol.com/live/stream";
    string _muxerFmt = "flv";
    QThreadPool *_pool;

    // 显示采集视频
    ShowVCap *_showVCap;
    bool _paintVcapFlag;    // 是否显示视频

};

#endif // CONTROL_H
