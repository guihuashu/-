#include "control.h"
#include <cstring>
#include <iostream>
#include <ffmopr.h>


Control::Control()
{
    FFmOpr::initFFM();
    _pool = QThreadPool::globalInstance();
    _pool->setMaxThreadCount(TaskThreadNum);
    _aPktList = new PktList(_aPktListSize);
    _vPktList = new PktList(_vPktListSize);

    QThread *curTh = QThread::currentThread();
    curTh->setStackSize(curTh->stackSize() * 2); // 线程栈大小放大2倍
    curTh->setPriority(QThread::HighPriority);
}


void Control::setVParam()
{
    // 视频编码器参数
    memset(&_vArgs, 0, sizeof(_vArgs));
    _vArgs.thread_count = 8;		// 用于编码的线程数, 系统所有线程数
    _vArgs.bit_rate = 50 * 1024 * 8;			// 码率, 越大视频越清晰
    _vArgs.gop_size = 50;						// 关键帧周期
    _vArgs.fps = 15;
    _vArgs.max_b_frames = 0;					// 最大b帧数, 有b帧就会有延迟
    _vArgs.inWidth = _inSize.width();
    _vArgs.inHeight = _inSize.height();
    _vArgs.outWidth = _outSize.width();
    _vArgs.outHeight = _outSize.height();
    _vArgs.in_pixFmt = _inFmt;
    _vArgs.out_pixFmt = _outFmt;	// 输入帧的像素格式
    _vArgs.CRF = "crf=20";          // h264压缩率
    // 音频编码器参数
}

void Control::setAParam()
{
    memset(&_aArgs, 0, sizeof(_aArgs));
    _aArgs.sample_rate = 44100;                 // 音频采样率
    _aArgs.channels = 2;                        // 音频采样通道数
    _aArgs.sample_fmt = AV_SAMPLE_FMT_S16;      // 采样格式, signed 16 bits
    _aArgs.resample_fmt = AV_SAMPLE_FMT_FLTP;   // 重采样格式
    _aArgs.thread_count = 8;					// 用于编码的线程数
    _aArgs.bit_rate = 50 * 1024 * 8;            // 码率, 越大声音越清晰
    _aArgs.frameDateSize = 4096
}

void Control::set_paintVcapFlag(bool flag)
{
    _paintVcapFlag = flag;
}




bool Control::init()
{
    if ((_inFmt != AV_PIX_FMT_NV21) || (_outFmt != AV_PIX_FMT_YUV420P) ) {
        qWarning()<<"cannot unsupport cap fmt";
        exit(0);
    }

    setVParam();
    setAParam();
    set_paintVcapFlag(false);
    _encode = new MediaEncode(_vArgs, _aArgs);
    _cap = new QtVideoCap(_inSize, _vArgs.fps, QVideoFrame::Format_NV21);
    outMedia = new OutMedia(outUr, streamFmt, _encode);

    // 连接视频采集信号
    connect(_cap->_probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(newVideoFrame(const QVideoFrame &)));

    // 连接显示视频的槽函数
    if (_paintVcapFlag) {
        _showVCap = new ShowVCap();
        _showVCap->show();
        connect(this, SIGNAL(showVideoFrame(QVideoFrame)), _showVCap, SLOT(showVideoFrame(QVideoFrame)));
    }
    CUR;
    return true;
}

void Control::newVideoFrame(const QVideoFrame &frame)
{
    //qInfo()<<"newVideoFrame";
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
        AVPacket *vPkt = vPktList->fontPkt();
        /* 必须要这样, 否则音频卡顿 */
        if ( !vPkt)
            continue;
        //CUR;
#if 0
        if (aPkt)
            outMedia.send_aPkt(aPkt);
#endif
        if (vPkt) {
            //CUR;
            outMedia->send_vPkt(vPkt);
            av_usleep(1000);    // 最好不要让包中有缓存
        }
        //av_usleep(1000 * 20);
    }
}

void Control::pushStream()
{
    std::thread th(workPushStream, _aPktList, _vPktList, outMedia);
    th.detach();
}

