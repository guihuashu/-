#ifndef _VideoEncode_H
#define _VideoEncode_H

//#include <AudioRecord.h>
#include <mutex>
#include <iostream>
#include <QDebug>
#include <QMutex>
#include <debug.h>
#include <string>


extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}


// 视频编码器参数
typedef struct vEncodeArgs {
	int thread_count;					// 用于编码的线程数
	int64_t bit_rate; 					// 码率, 越大视频越清晰
    int inWidth;						// 输出图像的宽度
    int inHeight;						// 输出图像的高度
    int outWidth;						// 输出图像的宽度
	int outHeight;						// 输出图像的高度
	int gop_size;						// 关键帧周期
	int max_b_frames;					// 最大b帧数, 有b帧就会有延迟
    int fps;
    enum AVPixelFormat out_pixFmt;	// 输出帧的像素格式
    enum AVPixelFormat in_pixFmt;	// 输出帧的像素格式
    std::string CRF;                // 压缩率, 范围 0-51: 0是编码毫无丢失信息, 23 is 默认, 51 是最差的情况。相对合理的区间是18-28.
}vEncodeArgs;

// 音频编码器参数
typedef struct aEncodeArgs {
    int sample_rate;                    // 音频采样率
    int channels;                       // 音频采样通道数
    int frameDateSize;                  // 一帧音频数据的字节数
    enum AVSampleFormat sample_fmt;     // 采样格式
    enum AVSampleFormat resample_fmt;   // 重采样格式
    int thread_count;					// 用于编码的线程数
    int64_t bit_rate; 					// 码率, 越大声音越清晰
}aEncodeArgs;

class MediaEncode : QObject
{
    Q_OBJECT
public:
    MediaEncode(vEncodeArgs &vArgs, aEncodeArgs &aArgs);
	~MediaEncode();

	// 视频相关函数
    //bool init();
    bool init_vEncodeCtx();

    //void set_vCodeArg(enum vEncodeArgType type, int64_t val);
    //bool init_vEncode();
    //bool init_aEncode();
    //bool init_avEncode();
    //AVFrame *brg24ToYuv420p(cv::Mat *inMat);
    //AVFrame *fmtS16_to_fmtFltp(char *inData);

    //bool aEncode(AVFrame *inFrame, AVPacket *aPkt);
    bool vEncode(AVFrame *inFrame, AVPacket *vPkt);


public:
	// 视频相关变量
    AVCodecContext *_vEncodeCtx = NULL;
	AVCodecContext *aEncodeCtx = NULL;
	AVFrame *pcm = NULL;
    vEncodeArgs _vArgs;
    aEncodeArgs _aArgs;
    QMutex vMutex;
    //SwrContext *aSwr = NULL;

//	// 音频相关变量
//	int sampleRate = 44100;	 // 采样率 44100
//	int channels = 2;
//	int aBit_rate = 20 * 1024 * 8;	// 音频编解码码率
//	int nb_samples = 1024;	// 一个通道一帧的样本数量
//	int aFrameBufSize = 1024 * 2 * 2;	// 两个通道, 每个样本4096字节(参考AudioRecord.h)

    long long _sysPts;			// 系统时间戳, 微妙
    AVRational _sysTimebase;		// 系统时间基数, 1/1000000, 微妙

    //unsigned int _aListSize = 3;
   // unsigned int _vListSize = 10;
    //std::list<AVPacket *> _aPktList;
    //std::list<AVPacket *> _vPktList;
    //std::mutex _aList_mutex;
    //std::mutex _vList_mutex;

	// 获取包装好的音频包
    //AVPacket *gain_aPkt();
    //AVPacket *gain_vPkt();

    //

	//void make_aPkt();
	//void make_vPkt();
private:
};  // MediaEncode


#endif		// _VideoEncode_H
