
#include "AudioRecord.h"

AudioRecord::AudioRecord()
{	
}

AudioRecord::~AudioRecord()
{
	delete input;
}


bool AudioRecord::init_record()
{
	/*1. 创建声卡输入 */
	fmt.setSampleRate(sampleRate);
	fmt.setChannelCount(channels);
	fmt.setSampleSize(sampleByte * 8);	// 样本字节数
	//  Qt音频的编码方式, 将采集到AV_SAMPLE_FMT_S16格式的音频数据
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian); // 音频数据为小端
	fmt.setSampleType(QAudioFormat::UnSignedInt); // 样本类型

	// 获取系统默认的音频设备的信息
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();

	// 检查我们设置的音频采样格式是否能设置在默认音频设备上
	if (!info.isFormatSupported(fmt))
	{
		cout << "Audio format not support!" << endl;
		exit(0);
		return false;
		//fmt = info.nearestFormat(fmt);
	}
	
	this->input = new QAudioInput(fmt);	// 创建音频输入接口
	input->setVolume(inputVolume);
	//input->setBufferSize(6000);
	this->io = input->start();	//开始录制音频
	//input->suspend();
	return true;
}

void AudioRecord::get_audio(char *buf, const int readSize)
{
	//input->resume();
	//io->readAll();
	//CUR;
	int size;
	if (!buf || (!readSize) || !input)
		return;
	//memset(buf, 0, readSize);
	for (;;)
	{
		//一次读取一帧音频
		if (input->bytesReady() < readSize)
		{
			QThread::usleep(50);
			//QThread::msleep(10);
			continue;
		}
		size = 0;
		while (size != readSize)
		{
			int len = io->read(buf + size, readSize - size);
			if (len < 0)
				break;
			size += len;
		}
		if (size != readSize)
			continue;
		//input->suspend();
		return;
	}
}
int AudioRecord::get_sampleRate()
{
	return this->sampleRate;
}
int AudioRecord::get_channels()
{
	return this->channels;
}
int AudioRecord::get_sampleByte()
{
	return this->sampleByte;
}
