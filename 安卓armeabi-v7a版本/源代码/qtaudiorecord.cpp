#include "qtaudiorecord.h"




QtAudioRecord::QtAudioRecord(aEncodeArgs &aArgs, QObject *parent)
    : QObject(parent),
      _frameDataSize(aArgs.frameDateSize)
{
    if (aArgs.sample_fmt !=  AV_SAMPLE_FMT_S16){    //< signed 16 bits
        qWarning()<<"ERR: audio record only support AV_SAMPLE_FMT_S16";
        exit(0);
    }
    _audioFmt.setSampleRate(aArgs.sample_rate);
    _audioFmt.setChannelCount(aArgs.channels);
    _audioFmt.setSampleSize(16);
    _audioFmt.setSampleType(QAudioFormat::SignedInt);
    _audioFmt.setByteOrder(QAudioFormat::LittleEndian);
    _audioFmt.setCodec("audio/pcm");

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    // 测试默认设备是否支持采集的格式, 如果不支持就退出
    if (!info.isFormatSupported(_audioFmt)) {
        qWarning()<<"ERR: The default audio device does unsupport this fmt of seted recording format";
        exit(0);
    }
    _audioInput = new QAudioInput(_audioFmt, this);
    _audioInputIO = _audioInput->start();
    if (!_audioInputIO) {
        qWarning()<<"ERR: _audioInputIO is NULL";
        exit(0);
    }
    if (!_audioInputIO->open(QIODevice::ReadWrite))  {
        qWarning()<<"ERR: open _audioInputIO";
       exit(0);
    }
}

void QtAudioRecord::run()
{
    AudioFrame *audioFrame= new AudioFrame();
    audioFrame->data = new char[_frameDataSize];

    while(1) {
        _frameDataSize = new
    }
}














