#ifndef _VideoEncode_H
#define _VideoEncode_H

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavdevice/avdevice.h>
	#include <libavcodec/avcodec.h>
	#include <libavcodec/mediacodec.h>
	#include <libavutil/avutil.h>
	#include <libavutil/dict.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/time.h>

}
#include <ffm_videoTst.h>
#include <opencv2/highgui.hpp>		//imread, 包含界面的一些文件
#include <ffm_util.h>
#include <list>
#include <AudioRecord.h>
#include <VideoCap.h>
#include <thread>
#include <mutex>
using namespace std;

typedef enum vEncodeArgType {
	THREAD_COUNT = 0, BIT_RATE, INWIDTH, FPS, OUT_PIXFMT,
	INHEIGHT, OUTWIDTH, OUTHEIGHT,GOP_SIZE, MAX_B_FRAMES 
}vEncodeArgType;


typedef struct VideoCodeArgs {
	// 编码器参数
	int thread_count;					// 用于编码的线程数
	int64_t bit_rate; 					// 码率, 越大视频越清晰
	int inWidth;						// 输入图像的宽度
	int inHeight;						// 输出图像的高度
	int outWidth;						// 输出图像的宽度
	int outHeight;						// 输出图像的高度
	int fps;
	int gop_size;						// 关键帧周期
	int max_b_frames;					// 最大b帧数, 有b帧就会有延迟
	AVPixelFormat in_pixFmt;	// 输入帧的像素格式
	AVPixelFormat out_pixFmt;	// 输出帧的像素格式
}VideoCodeArgs;


class MediaEncode
{
public:
	MediaEncode();
	~MediaEncode();

	// 视频相关函数
	void set_vCodeArg(enum vEncodeArgType type, int64_t val);
	bool init_vEncode();
	bool init_aEncode();
	bool init_avEncode();
	AVFrame *brg24ToYuv420p(cv::Mat *inMat);
	AVFrame *fmtS16_to_fmtFltp(char *inData);

	bool aEncode(AVFrame *inFrame, AVPacket *aPkt);
	bool vEncode(AVFrame *inFrame, AVPacket *aPkt);

public:
	// 视频相关变量
	AVCodecContext *vEncodeCtx = NULL;
	AVCodecContext *aEncodeCtx = NULL;
	AVFrame *yuv420p;
	AVFrame *pcm = NULL;
	VideoCodeArgs vArgs;
	SwsContext *vSwsCtx = NULL;
	SwrContext *aSwr = NULL;

	//int vPts = 0;
	//int aPts = 0;
	// 音频相关变量
	int sampleRate = 44100;	 // 采样率 44100
	int channels = 2;
	int aBit_rate = 20 * 1024 * 8;	// 音频编解码码率
	int nb_samples = 1024;	// 一个通道一帧的样本数量
	int aFrameBufSize = 1024 * 2 * 2;	// 两个通道, 每个样本4096字节(参考AudioRecord.h)

	long long sysPts;			// 系统时间戳, 微妙
	AVRational sysTimebase;		// 系统时间基数, 1/1000000
	int aListSize = 3;
	int vListSize = 3;
	list<AVPacket *> aPktList;
	list<AVPacket *> vPktList;
	mutex aList_mutex;
	mutex vList_mutex;

	// 获取包装好的音频包
	AVPacket *gain_aPkt();
	AVPacket *gain_vPkt();

	//void make_aPkt();
	//void make_vPkt();
};

#endif		// _VideoEncode_H
