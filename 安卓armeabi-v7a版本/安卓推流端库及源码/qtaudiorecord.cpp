#include "qtaudiorecord.h"



void QtAudioRecord::dumpAudioFmt(QAudioFormat &audioFmt)
{
    qInfo()<<"sampleRate: "<<audioFmt.sampleRate();
    qInfo()<<"channelCount: "<<audioFmt.channelCount();
    qInfo()<<"sampleSize: "<<audioFmt.sampleSize();
    qInfo()<<"sampleType: "<<audioFmt.sampleType();
    qInfo()<<"byteOrder: "<<audioFmt.byteOrder();
    qInfo()<<"codec: "<<audioFmt.codec();
}

QtAudioRecord::QtAudioRecord(aEncodeArgs &aArgs,  QThread *parent):
        QThread(parent),
        _frameDataSize(aArgs.frameDateSize),
        _nb_samples(aArgs.nb_samples)   // 单通道样本数量
{
    if (aArgs.sample_fmt !=  AV_SAMPLE_FMT_S16){    //< signed 16 bits
        qWarning()<<"ERR: audio record only support AV_SAMPLE_FMT_S16";
        getchar();
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
        _audioFmt = info.nearestFormat(_audioFmt);
        qWarning()<<"ERR: The default audio device does unsupport this fmt of seted recording format";
        qInfo()<<"recent nearestFormat:";
        dumpAudioFmt(_audioFmt);
        getchar();
    }
    qInfo()<<"set audio record fmt:";
    dumpAudioFmt(_audioFmt);
    _audioInput = new QAudioInput(_audioFmt, this);
    _audioInputIO = _audioInput->start();
    if (!_audioInputIO) {
        qWarning()<<"ERR: _audioInputIO is NULL";
        getchar();
    }
    if (!_audioInputIO->open(QIODevice::ReadWrite))  {
        qWarning()<<"ERR: open _audioInputIO";
        getchar();
    }
    // 类型(AudioFrame &)在信号槽函数中使用, 应该注册这种类型, 否则系统不能识别这种类型
    qRegisterMetaType<AudioFrame>("AudioFrame &");
}

void QtAudioRecord::run()
{
    while(1) {
        AudioFrame audioFrame;
        audioFrame.dataSize = _frameDataSize;
        audioFrame.nb_samples = _nb_samples;
        audioFrame.data = new char[_frameDataSize];

        qint64 len = 0;
        qint64 size = (int)_frameDataSize;
        while (len < size){
            len += _audioInputIO->read(audioFrame.data + len, size - len);
        }
        emit newAudioFrame(audioFrame); // 帧中的数据需要手动释放
    }
}
















