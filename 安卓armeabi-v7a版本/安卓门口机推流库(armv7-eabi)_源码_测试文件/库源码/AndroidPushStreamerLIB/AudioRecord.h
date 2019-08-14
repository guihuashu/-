#ifndef AUDIORECORD
#define AUDIORECORD

#include <QAudioFormat>
#include <QAudioInput>
#include <QIODevice>
#include <QThread>
#include <QMutex>
#include <iostream>
#include <QDebug>
#include <list>
using namespace std;

typedef enum RecordArgType {
	 SAMPLERATE, 	// 采样率
	 CHANNELS,			// 音频通道
	 SAMPLE_BYTE16,		// 一个样本所用字节数
	 VOLUME
}RecordArgType;


class AudioRecord
{
public:
	AudioRecord();
	~AudioRecord();

	void set_arg(RecordArgType type);
	bool init_record();
	void get_audio(char *buf, const int size);
	int get_sampleRate();
	int get_channels();
	int get_sampleByte();

private:
	//QMutex record;		// 决定是否采集
	QAudioFormat fmt;
	QAudioInput *input;
	QIODevice *io;


	int sampleRate = 44100;		// 采样率
	int channels = 2;			// 音频通道
	int sampleByte = 2;			// 一个样本所用字节数
	int inputVolume = 100;
	//int record_bufsize = 4096;
};


#endif	// AUDIORECORD
