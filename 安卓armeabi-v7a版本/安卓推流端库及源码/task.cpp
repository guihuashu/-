#include "task.h"

VTask::VTask(QVideoFrame &curFrame, MediaEncode *encode, PktList *vlist) :
    _curFrame(curFrame),
    _encode(encode),
    _vlist(vlist)
{
    AVPixelFormat inFmt = _encode->_vArgs.in_pixFmt;
    AVPixelFormat outFmt = _encode->_vArgs.out_pixFmt;
    //CUR;
#if 1
    if ((inFmt != AV_PIX_FMT_NV21) || (outFmt != AV_PIX_FMT_YUV420P)){
        qWarning()<<"QRunnable unsupported inFmt or outFmt";
        getchar();
    }
#endif
}

VTask::~VTask()
{
   // sws_freeContext(_swsNv21toBgr24);
    //sws_freeContext(_swsNv21toYuv420p);
}

void VTask::run()
{
    //QThread::currentThread()->setPriority(QThread::HighestPriority);
    //CUR;
    //AVFrame *bgr24 = NULL;
    AVFrame *yuv420p= NULL;
    int ret;
    AVPixelFormat inFmt = _encode->_vArgs.in_pixFmt;
    AVPixelFormat outFmt = _encode->_vArgs.out_pixFmt;
    int outW = _encode->_vArgs.outWidth;
    int outH = _encode->_vArgs.outHeight;
    SwsContext *swsNv21toYuv420p = _encode->_swsNv21toYuv420p;

#if 1
    if ((inFmt != AV_PIX_FMT_NV21) || (outFmt != AV_PIX_FMT_YUV420P)){
        qWarning()<<"QRunnable unsupported inFmt or outFmt";
        getchar();
    }

    ret = FFmOpr::allocVFrame(&yuv420p, outW, outH, AV_PIX_FMT_YUV420P);
    if (!ret){
        qWarning()<<"allocFrame err";
        return;
    }

    _encode->_swsMutex.lock();
    ret = FFmOpr::Nv21toYuv420p(swsNv21toYuv420p, _curFrame, yuv420p);
    _encode->_swsMutex.unlock();
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
    pkt->pts = av_gettime() - _encode->_sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间
    _encode->_vEncodeMutex.lock();
    bool br = _encode->vEncode(yuv420p, pkt);
    _encode->_vEncodeMutex.unlock();
    if (!br) {
        qWarning()<<"ERR: vEncode()";
        FFmOpr::freeFrame(&yuv420p);
        FFmOpr::freePkt(&pkt);
        return;
    }
    FFmOpr::freeFrame(&yuv420p);
    _vlist->pushBackPkt(pkt);
    //qInfo()<<"_vlist="<<_vlist->size();
}



/////////////////////////////// 音频任务 ///////////////////////////////////////
ATask::ATask(AudioFrame &aframe, MediaEncode *encode, PktList *alist) :
    _audioFrame(&aframe),
    _encode(encode),
    _alist(alist)
{
    AVSampleFormat sample_fmt = _encode->_aArgs.sample_fmt;
    AVSampleFormat resample_fmt = _encode->_aArgs.resample_fmt;
    if ((sample_fmt != AV_SAMPLE_FMT_S16) || (resample_fmt != AV_SAMPLE_FMT_FLTP)){
        CUR;
        qWarning()<<"unsupported sample_fmt or resample_fmt";
        getchar();
    }
}

ATask::~ATask()
{

}

void ATask::run()
{
    AVFrame *fltp;
    int ret;
    int channels = _encode->_aArgs.channels;
    int nb_samples = _encode->_aArgs.nb_samples;
    enum AVSampleFormat resample_fmt = _encode->_aArgs.resample_fmt;
    SwrContext *swrS16toFltp = _encode->_swrS16toFltp;

    if (!FFmOpr::allocAFrame(&fltp, channels, nb_samples, resample_fmt)) {
        CUR;
        qWarning()<<"ERR:allocAFrame ";
        delete[] _audioFrame->data;
        return;
    }

    //_encode->_swrMutex.lock();
    ret = FFmOpr::S16toFltp(swrS16toFltp, _audioFrame, fltp);   // s16转fltp
    //_encode->_swrMutex.unlock();
    //qInfo()<<"S16toFltp: ret="<<ret;
    if (ret <=0){
        qWarning()<<"ERR:S16toFltp ";
        delete[] _audioFrame->data;
        return;
    }
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        delete[] _audioFrame->data;
        return;
    }
    av_init_packet(pkt);
    pkt->pts = av_gettime() - _encode->_sysPts;	// 包的时间戳(毫秒) = 当前时间-开始记录的时间
    //_encode->_vEncodeMutex.lock();
    bool br = _encode->aEncode(fltp, pkt);
    //_encode->_vEncodeMutex.unlock();
    if (!br) {
        qWarning()<<"ERR: aEncode()";
        FFmOpr::freePkt(&pkt);
        delete[] _audioFrame->data;
        FFmOpr::freeFrame(&fltp);
        return;
    }
    delete[] _audioFrame->data;
    FFmOpr::freeFrame(&fltp);
    _alist->pushBackPkt(pkt);
    //qInfo()<<"_alist="<<_alist->size();
}
