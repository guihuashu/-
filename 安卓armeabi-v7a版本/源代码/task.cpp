#include "task.h"

VTask::VTask(QVideoFrame &curFrame, MediaEncode *encode, PktList *list) :
    _curFrame(curFrame),
    _encode(encode),
    _list(list)
{
    AVPixelFormat inFmt = _encode->_vArgs.in_pixFmt;
    AVPixelFormat outFmt = _encode->_vArgs.out_pixFmt;
    //CUR;
#if 0
    if ((inFmt != AV_PIX_FMT_NV21) || (outFmt != AV_PIX_FMT_YUV420P)){
        qWarning()<<"QRunnable unsupported inFmt or outFmt";
        exit(0);
    }
#endif
}

VTask::~VTask()
{
   // sws_freeContext(_swsNv21toBgr24);
    sws_freeContext(_swsNv21toYuv420p);
}

void VTask::run()
{
    //CUR;
    AVFrame *bgr24 = NULL;
    AVFrame *yuv420p= NULL;
    int ret;
    AVPixelFormat inFmt = _encode->_vArgs.in_pixFmt;
    AVPixelFormat outFmt = _encode->_vArgs.out_pixFmt;
    int inW = _encode->_vArgs.inWidth;
    int inH = _encode->_vArgs.inHeight;
    int outW = _encode->_vArgs.outWidth;
    int outH = _encode->_vArgs.outHeight;
    long long sysPts = _encode->_sysPts;

    QThread *curTh = QThread::currentThread();
    curTh->setStackSize(curTh->stackSize() * 2); // 线程栈大小放大2倍

    //CUR;
#if 0
    _swsNv21toBgr24 = sws_getCachedContext(_swsNv21toBgr24, \
        inW, inH, AV_PIX_FMT_NV21, \
        outW, outH, AV_PIX_FMT_BGR24, \
        SWS_BICUBIC, NULL, NULL, NULL);
    _swsBrg24toYuv420p = sws_getCachedContext(_swsBrg24toYuv420p, \
        inW, inH, AV_PIX_FMT_BGR24, \
        outW, outH, AV_PIX_FMT_YUV420P, \
        SWS_BICUBIC, NULL, NULL, NULL);
    if (!_swsBrg24toYuv420p || !_swsNv21toBgr24) {
        qWarning()<<"ERR: sws_getCachedContext";
        return;
    }
    // 分配rgb24
    bool r1 = FFmOpr::allocFrame(&bgr24, outW, outH, AV_PIX_FMT_BGR24);
    bool r2 = FFmOpr::allocFrame(&yuv420p, outW, outH, AV_PIX_FMT_YUV420P);
    if (!r1 || !r2){
        qWarning()<<"allocFrame err";
        return;
    }
    int ret1 = FFmOpr::NV21toBgr24(_swsNv21toBgr24, _curFrame, bgr24);
    int ret2 = FFmOpr::Bgrb24toYuv420P(_swsBrg24toYuv420p, bgr24, yuv420p);
    if ((ret1 <=0) || (ret2 <= 0)) {
        qWarning()<<"ERR: NV21toRgb24 or Rrgb24toYuv420P";
    }
    qInfo()<<"ret1="<<ret1<<", ret2="<<ret2;
    FFmOpr::freeFrame(&bgr24);
#endif
#if 1

    if ((inFmt != AV_PIX_FMT_NV21) || (outFmt != AV_PIX_FMT_YUV420P)){
        qWarning()<<"QRunnable unsupported inFmt or outFmt";
        exit(0);
    }
    /* 初始化格式转换上下文(Format_NV21->AV_PIX_FMT_YUV420P) */
    _swsNv21toYuv420p = sws_getCachedContext(_swsNv21toYuv420p, \
            inW, inH, inFmt, \
            outW, outH, outFmt, \
            SWS_FAST_BILINEAR, NULL, NULL, NULL);

    ret = FFmOpr::allocFrame(&yuv420p, outW, outH, AV_PIX_FMT_YUV420P);
    if (!ret){
        qWarning()<<"allocFrame err";
        return;
    }

    ret = FFmOpr::Nv21toYuv420p(_swsNv21toYuv420p, _curFrame, yuv420p);

    //qInfo()<<"ret ="<< ret;
    if (ret <= 0) {
        qWarning()<<"Nv21toYuv420p Err";
        FFmOpr::freeFrame(&yuv420p);
        return;
    }
#endif
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        FFmOpr::freeFrame(&yuv420p);
        return;
    }
    av_init_packet(pkt);
    pkt->pts = av_gettime() - sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间
    if (!(_encode->vEncode(yuv420p, pkt))) {
        qWarning()<<"ERR: vEncode()";
        FFmOpr::freeFrame(&yuv420p);
        FFmOpr::freePkt(&pkt);
        return;
    }
    FFmOpr::freeFrame(&yuv420p);
    _list->pushBackPkt(pkt);
}
