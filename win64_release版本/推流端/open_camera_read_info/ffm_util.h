#ifndef FFM_UTIL_H
#define FFM_UTIL_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/mediacodec.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include "libswscale/swscale.h"
}

#include <opencv2/highgui.hpp>		//imread, 包含界面的一些文件

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif


//获取CPU数量
int get_sysCpuNum();
void ffm_init();
int save_pic(const char *path, AVPacket *pkt);




#endif // FFM_UTIL_H