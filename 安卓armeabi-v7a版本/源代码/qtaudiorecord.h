#ifndef QTAUDIORECORD_H
#define QTAUDIORECORD_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QIODevice>

#include <MediaEncode.h>
#include <debug.h>
#include <QThread>
#include <thread>


typedef struct AudioFrame{
    char *data;
    int dataSize;
}AudioFrame;

class QtAudioRecord : public QThread
{
    Q_OBJECT
public:
    QtAudioRecord(aEncodeArgs &aArgs,
                  QObject *parent = nullptr);

    virtual void run();
signals:

public slots:

private:
    QAudioFormat _audioFmt;
    QAudioInput *_audioInput;
    QIODevice *_audioInputIO;
    int _frameDataSize;
    //AudioFrame _audioFrame;
};

#endif // QTAUDIORECORD_H









