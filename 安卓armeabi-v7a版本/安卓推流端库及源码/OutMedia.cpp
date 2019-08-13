#include "OutMedia.h"

OutMedia::OutMedia(string outUrl, string streamFmt, MediaEncode *encode)
{
    qInfo() << "------------------  OutMedia  ------------------";
    this->_outUrl = outUrl;
    this->_muxerFmt = streamFmt;

    if (0 > avformat_alloc_output_context2(&_outFmtCtx, NULL, "flv", "rtmp://hdlcontrol.com/live/stream")) {
        qWarning()<< "ERR: avformat_alloc_output_context2";
        getchar();
    }
#if 0   // 这个不能设置, 否则丢失音频流
    _outFmtCtx->max_interleave_delta = 0;			// 交叉存取的最大延迟
#endif
    _outFmtCtx->max_delay = 0;
    /* 增加音频流和视频流 */
    if (!addStream(encode->_vEncodeCtx)) {// 增加视频流
        qWarning()<<"ERR: add video Stream";
        getchar();
    }
    if (!addStream(encode->_aEncodeCtx)) {    // 增加音频流
        qWarning()<<"ERR: add audio Stream";
        getchar();
    }

    CUR;
    dump_outMediaFmt();
    if (!write_headerInfo()) {
        qWarning()<<"ERR: write_headerInfo";
        getchar();
    }
    CUR;
}


bool OutMedia::addStream(AVCodecContext *encodeCtx)
{
	if (!encodeCtx)
		return false;
	AVStream *stream;

    stream = avformat_new_stream(_outFmtCtx, NULL);	// outFmtCtx中添加视频流
	if (0 > avcodec_parameters_from_context(stream->codecpar, encodeCtx)) { // 从encoderCtx中获取编码参数
		cout << "avcodec_parameters_from_context err";
		return false;
	}

	if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        CUR;
        vStream = stream;
		vEncodeCtx = encodeCtx;
	}
	else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        CUR;
		aStream = stream;
		aEncodeCtx = encodeCtx;
	}
	return true;
}

void OutMedia::dump_outMediaFmt()
{
    for (unsigned int i = 0; i < _outFmtCtx->nb_streams; ++i) {
        av_dump_format(_outFmtCtx, i, this->_outUrl.c_str(), 1);		// 输出outFmtCtx中的视频流信息
        CUR;
        qInfo()<<i;
    }
}
bool OutMedia::write_headerInfo()
{
    avio_open(&_outFmtCtx->pb, _outUrl.c_str(), AVIO_FLAG_WRITE);
    if (avformat_write_header(_outFmtCtx, NULL) < 0) {
        CUR<<"ERR: write_headerInfo";
        return false;
    }
    return true;
}

bool OutMedia::send_vPkt(AVPacket *vPkt)
{
    int i=0;
	if (!vPkt)
		return false;
    int64_t start = av_gettime();
    int64_t mid;
    // 流索引
	vPkt->stream_index = vStream->index;
	// 将包时间戳.由基于编码器的时间基数.转换为基于流的时间基数
    vPkt->pts = av_rescale_q(vPkt->pts, sysTimebase, vStream->time_base);
	vPkt->dts = av_rescale_q(vPkt->dts, sysTimebase, vStream->time_base);
	vPkt->duration = av_rescale_q(vPkt->duration, sysTimebase, vStream->time_base);
	/* 将编码后的数据包发送到输出格式上下文中 */
resend:
    if (av_interleaved_write_frame(_outFmtCtx, vPkt)) {
//        if (vPkt->flags & AV_PKT_FLAG_KEY ) {   // 是关键帧就必须重发,不然播放端gop个帧都解码不了
////            mid = av_gettime() - start;
////            vPkt->pts += mid;
////            vPkt->dts += mid;
////            vPkt->duration += mid;
//            //if (++i <= 3)
//            goto resend;
//        }
        qWarning()<<"ERR: send_vPkt";
        av_packet_unref(vPkt);	// 包中数据的引用计数-1
        av_packet_free(&vPkt);
		return false;
	}
    //qInfo() << "#" << vPkt->size << " " << flush;
    av_packet_unref(vPkt);	// 包中数据的引用计数-1
    av_packet_free(&vPkt);
	return true;
}
bool OutMedia::send_aPkt(AVPacket *aPkt)
{
	if (!aPkt)
		return false;
	if (aPkt->size <= 0) {
		av_packet_unref(aPkt);	// 包中数据的引用计数-1
		av_packet_free(&aPkt);
		aPkt = NULL;
		return false;
	}

    // 流索引
	aPkt->stream_index = aStream->index;
	// 将包时间戳.由基于编码器的时间基数.转换为基于流的时间基数
	aPkt->pts = av_rescale_q(aPkt->pts, sysTimebase, aStream->time_base);
	aPkt->dts = av_rescale_q(aPkt->dts, sysTimebase, aStream->time_base);
	aPkt->duration = av_rescale_q(aPkt->duration, sysTimebase, aStream->time_base);
    if (av_interleaved_write_frame(_outFmtCtx, aPkt)) {
        qWarning()<<"ERR: send_aPkt";
		av_packet_unref(aPkt);	// 包中数据的引用计数-1
		av_packet_free(&aPkt);
		aPkt = NULL;
		return false;
	}
    //qInfo() << "*" << aPkt->size << " ";
    av_packet_unref(aPkt);	// 包中数据的引用计数-1
	av_packet_free(&aPkt);
	aPkt = NULL;
	return true;
}


OutMedia::~OutMedia()
{
    avformat_free_context(this->_outFmtCtx);
}
AVStream *OutMedia::get_vStream()
{
	return this->vStream;
}

AVStream *OutMedia::get_aStream()
{
	return this->aStream;
}
AVFormatContext *OutMedia::get_outFmtCtx()
{
    return this->_outFmtCtx;
}
int OutMedia::get_vStreamIndex()
{
	return vStream->index;
}
int OutMedia::get_aStreamIndex()
{
	return aStream->index;
}
