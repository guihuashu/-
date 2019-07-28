#include "Demux.h"
using namespace std;

double calc_AVRational(AVRational Ration)
{
	return Ration.den == 0 ? 0 : (double)Ration.num / (double)Ration.den;
}

Demux::Demux(char *inUrl)
{
	this->inUrl = inUrl;

	//av_register_all();	//初始化封装库
	avformat_network_init();	//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	//avcodec_register_all();	//注册解码器
	
	


	//解封装上下文
	if (avformat_open_input(&inFmtCtx, inUrl, 0, &opts)) { // 0表示自动选择解封器
		cout << "open " << inUrl << " failed! :" << endl;
		exit(0);
	}
	av_dict_set(&opts, "fflags", "nobuffer", 0);
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	av_dict_set(&opts, "flags", "low_delay", 0);
	//av_dict_set(&opts, "max_delay", "0", 0);
	av_dict_set(&opts, "strict", "experimental", 0);

	//av_dict_set(&opts, "max_delay", "0", 0);
	// 获取流信息 , 对于流媒体这一步必不可少
	if (avformat_find_stream_info(inFmtCtx, 0)) {
		cout << "ERR: avformat_find_stream_info" << endl;
		exit(0);
	}
	av_dump_format(inFmtCtx, 0, path, 0);	//打印视频流详细信息


	//获取流信息
	for (int i = 0; i < inFmtCtx->nb_streams; i++)
	{
		AVStream *avs = inFmtCtx->streams[i];
		cout << "codec_id = " << avs->codecpar->codec_id << endl;
		cout << "format = " << avs->codecpar->format << endl;
		
		if (avs->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)	//音频 AVMEDIA_TYPE_AUDIO
		{
			as = avs;
			asIndex = i;
			aCodecParam = avs->codecpar;
			cout << i << "音频信息" << endl;
			cout << "sample_rate = " << as->codecpar->sample_rate << endl;
			cout << "channels = " << as->codecpar->channels << endl;	//AVSampleFormat;
			cout << "frame_size = " << as->codecpar->frame_size << endl;	// 单通道样本数 
			//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size
		}
		else if (avs->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)	//视频 AVMEDIA_TYPE_VIDEO
		{
			vs = avs;
			vsIndex = i;
			vCodecParam = avs->codecpar;
			cout << i << "视频信息" << endl;
			cout << "width=" << vs->codecpar->width << endl;
			cout << "height=" << vs->codecpar->height << endl;
			//帧率 fps 分数转换
			cout << "video fps = " << calc_AVRational(vs->avg_frame_rate) << endl;
		}
	}

}


Demux::~Demux()
{
}
