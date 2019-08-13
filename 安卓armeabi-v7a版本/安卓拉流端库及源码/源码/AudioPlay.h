#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <Decode.h>

class AudioPlay
{
public:
	AudioPlay();
	virtual ~AudioPlay();
	bool open();
	void writePcm(Pcm *pcm);

public:
    int sampleRate = 8000;
	int sampleSize = 16;
	int channels = 2;


private:
	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	QAudioFormat fmt;
};


#endif // AUDIOPLAY_H
