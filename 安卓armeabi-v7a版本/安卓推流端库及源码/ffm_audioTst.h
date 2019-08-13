#ifndef FFM_AUDIO_H
#define FFM_AUDIO_H

#include <QAudioInput>
#include <iostream>
#include <AudioRecord.h>
#include <MediaEncode.h>
#include <OutMedia.h>

extern "C"
{
	#include <libswresample/swresample.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/dict.h>
}



int audio_push();
void my_push();

#endif //FFM_AUDIO_H
