#ifndef QTVIDEOCAP_H
#define QTVIDEOCAP_H

#include <QWidget>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoFrame>
#include <QCameraViewfinder>
#include <QThread>
#include <QPainter>
#include <QTimer>
#include <QVideoProbe>
#include <QDebug>
#include <QPixmap>
#include <QThreadPool>
#include <QCameraViewfinderSettings>
#include <QAbstractVideoBuffer>
#include <list>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}
#define CUR qInfo()<<__FILE__<<","<<__LINE__

class QtVideoCap : public QObject
{
    Q_OBJECT
public:
    QtVideoCap(QObject *parent = nullptr);
    void init();

public slots:
    // 采集到新帧数据
    //void newVideoFrame(const QVideoFrame &frame);
    void stateChanged(QCamera::State state);
//signals:


public :
    QCamera *_camera;
    QVideoProbe *_probe;
    QCameraInfo useCameraInfo;
    QCameraViewfinder *viewfinder;  // 相机取景器. 必须设置
    QSize inSize = {640,480};
    qreal fps = 30;

#ifdef ANDROID
    QVideoFrame::PixelFormat Fmt = QVideoFrame::Format_NV21;
    AVPixelFormat inFmt = AV_PIX_FMT_NV21;
#elif WIN64
    QVideoFrame::PixelFormat Fmt = QVideoFrame::Format_ARGB32;
#endif


};

#endif // QTVIDEOCAP_H
