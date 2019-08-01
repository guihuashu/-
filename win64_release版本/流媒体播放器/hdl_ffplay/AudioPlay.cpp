#include "AudioPlay.h"



AudioPlay::AudioPlay()
{
}
bool AudioPlay::open()
{
	fmt.setSampleRate(sampleRate);
	fmt.setChannelCount(channels);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	// S16
	fmt.setSampleSize(sampleSize);
	fmt.setSampleType(QAudioFormat::UnSignedInt);

	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	if (!info.isFormatSupported(fmt))			// 检查input是否支持这种输入格式
		return false;

	output = new QAudioOutput(fmt);
	io = output->start(); //开始播放
	return true;
}

void AudioPlay::writePcm(Pcm *pcm)
{
	if (!pcm || !pcm->size)
		return;
	qint64 all = (qint64)pcm->size;
	int ret;
	qint64 len = 0;
	while (len < all) {
		ret  = io->write((char *)(pcm->data + len), all - len);
		len += ret;
	}
	free_Pcm(pcm);
}

AudioPlay::~AudioPlay()
{
}
