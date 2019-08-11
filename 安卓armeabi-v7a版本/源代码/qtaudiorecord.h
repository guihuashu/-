#ifndef QTAUDIORECORD_H
#define QTAUDIORECORD_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QIODevice>
#include <QThread>
#include <QMetaType>

#include <MediaEncode.h>
#include <debug.h>


typedef struct AudioFrame{
    char *data;
    int dataSize;
    int nb_samples;
}AudioFrame;

class QtAudioRecord : public QThread
{
    Q_OBJECT
public:
    QtAudioRecord(aEncodeArgs &aArgs,
                  QThread *parent = nullptr);
    virtual void run();
    void dumpAudioFmt(QAudioFormat &audioFmt);
signals:
    void newAudioFrame(AudioFrame &audioFrame);

private:
    QAudioFormat _audioFmt;
    QAudioInput *_audioInput;
    QIODevice *_audioInputIO;
    int _frameDataSize;
    int _nb_samples;
};

#endif // QTAUDIORECORD_H









