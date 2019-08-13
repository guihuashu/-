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

    //QThread *curTh = QThread::currentThread();
    //curTh->setStackSize(curTh->stackSize() * 2); // 线程栈大小放大2倍
    //curTh->setPriority(QThread::HighPriority);
}


void Control::setVargs()
{
    // 视频编码器参数
    memset(&_vArgs, 0, sizeof(_vArgs));
    _vArgs.thread_count = 8;		// 用于编码的线程数, 系统所有线程数
    _vArgs.bit_rate = 20 * 1024 * 8;			// 码率, 越大视频越清晰
    _vArgs.gop_size = 50;						// 关键帧周期
    _vArgs.fps = 30;
    _vArgs.max_b_frames = 0;					// 最大b帧数, 有b帧就会有延迟
    _vArgs.inWidth = _inSize.width();
    _vArgs.inHeight = _inSize.height();
    _vArgs.outWidth = _outSize.width();
    _vArgs.outHeight = _outSize.height();
    _vArgs.in_pixFmt = _inFmt;
    _vArgs.out_pixFmt = _outFmt;	// 输入帧的像素格式
    _vArgs.CRF = "crf=25";          // h264压缩率
}

void Control::setAargs()
{
    memset(&_aArgs, 0, sizeof(_aArgs));
    _aArgs.sample_rate = 8000;                 // 音频采样率, 8000,44100
    _aArgs.channels = 2;                        // 音频采样通道数
    _aArgs.sample_fmt = AV_SAMPLE_FMT_S16;      // 采样格式, signed 16 bits
    _aArgs.resample_fmt = AV_SAMPLE_FMT_FLTP;   // 重采样格式
    _aArgs.thread_count = 6;					// 用于编码的线程数
    _aArgs.bit_rate = 20 * 1024 * 8;            // 码率, 越大声音越清晰
    _aArgs.nb_samples = 1024;
    if (_aArgs.sample_fmt != AV_SAMPLE_FMT_S16) {
        CUR;
        qWarning()<<"unsupport sample_fmt:";
    }
    _aArgs.frameDateSize = _aArgs.nb_samples * _aArgs.channels * (16 /8); // s16表示两个字节
}

void Control::set_paintVcapFlag(bool flag)
{
    _paintVcapFlag = flag;
}




bool Control::init()
{ 
    setVargs();
    setAargs();
    set_paintVcapFlag(false);
    _encode = new MediaEncode(_vArgs, _aArgs);
    _vCap = new QtVideoCap(_inSize, _vArgs.fps, QVideoFrame::Format_NV21);
    _aCap = new QtAudioRecord(_aArgs);
    _aCap->start();         // 使用线程采集音频

    _outMedia = new OutMedia(_outUr, _muxerFmt, _encode);

    // 连接视频.音频采集信号
    connect(_vCap->_probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(newVideoFrame(const QVideoFrame &)));
    connect(_aCap, SIGNAL(newAudioFrame(AudioFrame &)), this, SLOT(newAudioFrame(AudioFrame &)));

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
    curFrame.map(QAbstractVideoBuffer::ReadOnly); // 需要映射才能读取其累不数据
    if (!curFrame.isValid()) {
        qWarning()<<"curFrame inValid";
        return;
    }

    VTask *vTask = new VTask(curFrame, _encode, _vPktList);
    //vTask->run();
    //delete vTask;
    _pool->start(vTask);

}

void Control::newAudioFrame(AudioFrame &audioFrame)
{
    //qInfo()<<"newAudioFrame: "<<audioFrame.dataSize;
    ATask aTask(audioFrame, _encode, _aPktList);
    aTask.run();
}


static void workPushStream(PktList *aPktList, PktList *vPktList, OutMedia *outMedia)
{
    while(1)
    {
        //CUR;
        AVPacket *aPkt, *vPkt;
        //if (aPktList->size() == aPktList->_listSize)
            aPkt = aPktList->fontPkt();
//        else {
//            aPkt = NULL;
//        }
        //if (vPktList->size() == vPktList->_listSize)
            vPkt = vPktList->fontPkt();
       // else {
        //    vPkt = NULL;
        //}
        //if (!aPkt && !vPkt) {
           // av_usleep(1);
        //}
        if (aPkt) {
            outMedia->send_aPkt(aPkt);
            //av_usleep(10);
        }
        if (vPkt) {
            outMedia->send_vPkt(vPkt);
            //av_usleep(1000);    // 最好不要让包中有缓存
        }
    }
}

void Control::pushStream()
{
    std::thread th(workPushStream, _aPktList, _vPktList, _outMedia);
    th.detach();
}

