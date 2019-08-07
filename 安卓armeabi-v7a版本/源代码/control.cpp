#include "control.h"
#include <cstring>
#include <iostream>
#include <ffmopr.h>

static void vCapShow(ShowVCap **cap)
{

}

Control::Control()
{
    FFmOpr::initFFM();
    _pool = QThreadPool::globalInstance();
    _aPktList = new PktList(_aPktListSize);
    _vPktList = new PktList(_vPktListSize);

}

void Control::set_Args()
{
    // 视频编码器参数
    memset(&_vArgs, 0, sizeof(_vArgs));
    _vArgs.thread_count = 6;		// 用于编码的线程数, 系统所有线程数
    _vArgs.bit_rate = 5 * 1024 * 8;			// 码率, 越大视频越清晰
    _vArgs.gop_size = 50;						// 关键帧周期
    _vArgs.max_b_frames = 0;					// 最大b帧数, 有b帧就会有延迟
    _vArgs.inWidth = _inSize.width();
    _vArgs.inHeight = _inSize.height();
    _vArgs.outWidth = _outSize.width();
    _vArgs.outHeight = _outSize.height();
    _vArgs.in_pixFmt = _inFmt;
    _vArgs.out_pixFmt = _outFmt;	// 输入帧的像素格式

    // 音频编码器参数
    memset(&_aArgs, 0, sizeof(_aArgs));
}

#if 0
    SwsNv21ToYuv420p = sws_getCachedContext(SwsNv21ToYuv420p, \
            _inSize.width(), _inSize.height(), _inFmt, \
            _outSize.width(), _outSize.height(), _outFmt, \
            SWS_FAST_BILINEAR, NULL, NULL, NULL);
#endif

bool Control::init()
{
    if ((_inFmt != AV_PIX_FMT_NV21) || (_outFmt != AV_PIX_FMT_YUV420P) ) {
        qWarning()<<"cannot unsupport cap fmt";
        exit(0);
    }
    CUR;
    set_Args();
    _encode = new MediaEncode(_vArgs, _aArgs);
    _cap = new QtVideoCap(_inSize, _fps, QVideoFrame::Format_NV21);
    outMedia = new OutMedia(outUr, streamFmt, _encode);
    CUR;
    //outMedia->init(outUr, streamFmt, _encode);

    // 连接视频采集信号
    connect(_cap->_probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(newVideoFrame(const QVideoFrame &)));
    CUR;
    // 连接显示视频信号
    //_vCap = new ShowVCap();
    //_vCap->show();
    //connect(this, SIGNAL(showVideoFrame(QVideoFrame)), _vCap, SLOT(showVideoFrame(QVideoFrame)));

    CUR;
    return true;
}

void Control::newVideoFrame(const QVideoFrame &frame)
{
    qInfo()<<"MediaEncode::newYuv420p";
    QVideoFrame curFrame = frame;
    curFrame.map(QAbstractVideoBuffer::ReadOnly);
    if (!curFrame.isValid()) {
        qWarning()<<"curFrame inValid";
        return;
    }
    VTask *vTask = new VTask(curFrame, _encode, _vPktList);
    _pool->start(vTask);
    //vTask->run();
    //emit showVideoFrame(curFrame);
}

static void workPushStream(PktList *aPktList, PktList *vPktList, OutMedia *outMedia)
{
    while(1)
    {
        AVPacket *aPkt;
        AVPacket *vPkt;

        //aPkt = mediaEncode.gain_aPkt();
        //qInfo()<<"vPktList.size="<<vPktList->size();
        vPkt = vPktList->fontPkt();
        /* 必须要这样, 否则音频卡顿 */
        if ( !vPkt)
            continue;
#if 0
        if (aPkt)
            outMedia.send_aPkt(aPkt);
#endif
        if (vPkt)
            outMedia->send_vPkt(vPkt);
        //av_usleep(1000 * 20);
    }
}

void Control::pushStream()
{
    std::thread th(workPushStream, _aPktList, _vPktList, outMedia);
    th.detach();
}

