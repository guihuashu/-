#ifndef FFM_H
#define FFM_H
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/mediacodec.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include "libswscale/swscale.h"
}
#include <ffm_util.h>

void start_push_flow();

#endif