#include "OutMedia.h"
#include <util.h>

OutMedia::OutMedia()
{
	cout << "------------------  OutMedia  ------------------" << endl;
}

bool OutMedia::init_outMedia(string outUrl, string streamFmt)
{
	this->outUrl = outUrl;
	this->streamFmt = streamFmt;

	if (0 > avformat_alloc_output_context2(&this->outFmtCtx, NULL, streamFmt.c_str(), outUrl.c_str())) {
		cout << "ERR: avformat_alloc_output_context2" << endl;
		return false;
	}

	// 推流延迟优化
	outFmtCtx->max_interleave_delta = 0;			// 交叉存取的最大延迟
	outFmtCtx->max_delay = 0;
	return true;
}


bool OutMedia::addStream(AVCodecContext *encodeCtx)
{
	if (!encodeCtx)
		return false;
	AVStream *stream;

	stream = avformat_new_stream(this->outFmtCtx, NULL);	// outFmtCtx中添加视频流
	if (0 > avcodec_parameters_from_context(stream->codecpar, encodeCtx)) { // 从encoderCtx中获取编码参数
		cout << "avcodec_parameters_from_context err";
		return false;
	}

	if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
		vStream = stream;
		vEncodeCtx = encodeCtx;
	}
	else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
		aStream = stream;
		aEncodeCtx = encodeCtx;
	}
	return true;
	//else if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
	//	subtitleStream = stream;
	//	subtitleEncodeCtx = encodeCtx;
	//}
}

void OutMedia::dump_outMediaFmt()
{
	for (int i = 0; i < this->outFmtCtx->nb_streams; ++i) {
		av_dump_format(outFmtCtx, i, this->outUrl.c_str(), 1);		// 输出outFmtCtx中的视频流信息
	}
}
bool OutMedia::write_headerInfo()
{
	avio_open(&this->outFmtCtx->pb, this->outUrl.c_str(), AVIO_FLAG_WRITE);
	return (0 < avformat_write_header(this->outFmtCtx, NULL));
}

bool OutMedia::send_vPkt(AVPacket *vPkt)
{
	if (!vPkt)
		return false;
	if (vPkt->size <= 0) {
		av_packet_unref(vPkt);	// 包中数据的引用计数-1
		av_packet_free(&vPkt);
		vPkt = NULL;
		return false;
	}
	// 流索引
	vPkt->stream_index = vStream->index;
	// 将包时间戳.由基于编码器的时间基数.转换为基于流的时间基数
	vPkt->pts = av_rescale_q(vPkt->pts, sysTimebase, vStream->time_base);
	vPkt->dts = av_rescale_q(vPkt->dts, sysTimebase, vStream->time_base);
	vPkt->duration = av_rescale_q(vPkt->duration, sysTimebase, vStream->time_base);

		cout << "#" << vPkt->size << " " << flush;
	/* 将编码后的数据包发送到输出格式上下文中 */
	if (av_interleaved_write_frame(outFmtCtx, vPkt)) {
		av_packet_unref(vPkt);	// 包中数据的引用计数-1
		av_packet_free(&vPkt);
		vPkt = NULL;
		return false;
	}
	av_packet_unref(vPkt);	// 包中数据的引用计数-1
	av_packet_free(&vPkt);
	vPkt = NULL;
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
	cout << "*" << aPkt->size << " " << flush;
	if (av_interleaved_write_frame(outFmtCtx, aPkt)) {
		av_packet_unref(aPkt);	// 包中数据的引用计数-1
		av_packet_free(&aPkt);
		aPkt = NULL;
		return false;
	}
	av_packet_unref(aPkt);	// 包中数据的引用计数-1
	av_packet_free(&aPkt);
	aPkt = NULL;
	return true;
}


OutMedia::~OutMedia()
{
	avformat_free_context(this->outFmtCtx);
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
	return this->outFmtCtx;
}
int OutMedia::get_vStreamIndex()
{
	return vStream->index;
}
int OutMedia::get_aStreamIndex()
{
	return aStream->index;
}
