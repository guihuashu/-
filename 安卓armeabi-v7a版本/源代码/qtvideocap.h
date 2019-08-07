#ifndef QTVIDEOCAP_H
#define QTVIDEOCAP_H

#include <QWidget>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoFrame>
#include <QCameraViewfinder>
#include <QVideoProbe>
#include <QDebug>
#include <QCameraViewfinderSettings>
#include <QAbstractVideoBuffer>
#include <list>
#include <debug.h>

class QtVideoCap : public QObject
{
    Q_OBJECT
public:
    QtVideoCap(QSize inSize, double fps, QVideoFrame::PixelFormat inFmt,
               QObject *parent = nullptr);
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
    QSize _inSize;
    double _fps;
    QVideoFrame::PixelFormat _inFmt;


};

#endif // QTVIDEOCAP_H
