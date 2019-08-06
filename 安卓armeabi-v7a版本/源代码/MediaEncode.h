#ifndef _VideoEncode_H
#define _VideoEncode_H

#include <AudioRecord.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <list>
#include <qtvideocap.h>
#include <QThreadPool>
#include <QRunnable>
#include <ffmopr.h>

typedef enum vEncodeArgType {
	THREAD_COUNT = 0, BIT_RATE, INWIDTH, FPS, OUT_PIXFMT,
	INHEIGHT, OUTWIDTH, OUTHEIGHT,GOP_SIZE, MAX_B_FRAMES 
}vEncodeArgType;




typedef struct VideoCodeArgs {
	// 编码器参数
	int thread_count;					// 用于编码的线程数
	int64_t bit_rate; 					// 码率, 越大视频越清晰
	int outWidth;						// 输出图像的宽度
	int outHeight;						// 输出图像的高度
	int gop_size;						// 关键帧周期
	int max_b_frames;					// 最大b帧数, 有b帧就会有延迟
    AVPixelFormat out_pixFmt;	// 输出帧的像素格式
}VideoCodeArgs;


class MediaEncode : QObject
{
    Q_OBJECT
public:
	MediaEncode();
	~MediaEncode();

	// 视频相关函数
	void set_vCodeArg(enum vEncodeArgType type, int64_t val);
    bool init_vEncode(QtVideoCap *cap);
	bool init_aEncode();
	bool init_avEncode();
    //AVFrame *brg24ToYuv420p(cv::Mat *inMat);
	AVFrame *fmtS16_to_fmtFltp(char *inData);

	bool aEncode(AVFrame *inFrame, AVPacket *aPkt);
	bool vEncode(AVFrame *inFrame, AVPacket *aPkt);
    void pktListPopFont(std::list<AVPacket *> *list);
    int Nv21toYuv420p(QVideoFrame &frame, AVFrame *yuv420p);
    int ARGB32toYuv420p(QVideoFrame &frame, AVFrame *yuv420p);

    void vPktPushFront(AVPacket *pkt);

public slots:
    void newVideoFrame(const QVideoFrame &frame);

public:
	// 视频相关变量
    AVCodecContext *_vEncodeCtx = NULL;
	AVCodecContext *aEncodeCtx = NULL;
	AVFrame *pcm = NULL;
    VideoCodeArgs _vArgs;
	SwrContext *aSwr = NULL;

	// 音频相关变量
	int sampleRate = 44100;	 // 采样率 44100
	int channels = 2;
	int aBit_rate = 20 * 1024 * 8;	// 音频编解码码率
	int nb_samples = 1024;	// 一个通道一帧的样本数量
	int aFrameBufSize = 1024 * 2 * 2;	// 两个通道, 每个样本4096字节(参考AudioRecord.h)

    long long _sysPts;			// 系统时间戳, 微妙
    AVRational _sysTimebase;		// 系统时间基数, 1/1000000, 微妙
    unsigned int _aListSize = 3;
    unsigned int _vListSize = 10;
    std::list<AVPacket *> _aPktList;
    std::list<AVPacket *> _vPktList;
    std::mutex _aList_mutex;
    std::mutex _vList_mutex;

	// 获取包装好的音频包
	AVPacket *gain_aPkt();
	AVPacket *gain_vPkt();

    //

	//void make_aPkt();
	//void make_vPkt();
private:
    bool init_vEncodeCtx(QtVideoCap *cap);
    QThreadPool *gpool;
};  // MediaEncode

class vTask : QRunnable
{
public:
    QRunnable(QVideoFrame curFrame, MediaEncode *This);
    virtual void run();
private:
    QVideoFrame _curFrame;
    MediaEncode *_This;
};

#endif		// _VideoEncode_H
