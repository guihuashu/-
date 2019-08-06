#include "qtvideocap.h"


QtVideoCap::QtVideoCap(QObject *parent) : QObject(parent)
{
}

void QtVideoCap::init() // 必须在主线程中调用
{
    CUR;
    /* 初始化相机 */
    useCameraInfo = QCameraInfo::defaultCamera();   // 使用默认的相机
    _camera = new QCamera(useCameraInfo);
    viewfinder = new QCameraViewfinder();
    viewfinder->close();    // 这个窗口在安卓中是不支持显示的
    _camera->setViewfinder(viewfinder);
    _probe = new QVideoProbe();
    _probe->setSource(_camera);
    // 在newVideoFrame函数中捕获视频帧,  当相机成功加载后,在stateChanged函数中设置相机
    //connect(_probe, &QVideoProbe::videoFrameProbed, this, &QtVideoCap::newVideoFrame);
    connect(_camera, &QCamera::stateChanged, this, &QtVideoCap::stateChanged);
    _camera->setCaptureMode(QCamera::CaptureVideo);
    _camera->start();
}


//void QtVideoCap::newVideoFrame(const QVideoFrame &frame)
//{
//    AVFrame *yuv420p;
//    int ret;

//    if (!frame.isValid()) {
//        qWarning()<<"newFrame isValid";
//        return;
//    }
//    emit newYuv420p(*yuv420p);
//}

void QtVideoCap::stateChanged(QCamera::State state)
{
    // 相机处于激活状态
    if (QCamera::ActiveState != state)
        return;

    QList<QCameraViewfinderSettings> list = _camera->supportedViewfinderSettings();
    for (int i=0; i < list.size(); i++)
    {
        QCameraViewfinderSettings set = list.at(i);
        // 取30fps 640x480
        if (set.resolution() == inSize \
           &&  (set.maximumFrameRate()>=fps) \
           &&  (set.maximumFrameRate()<(fps +10))
           &&  (set.pixelFormat() == Fmt))
        {
            _camera->setViewfinderSettings(set);
            qWarning()<<set.minimumFrameRate();
            qWarning()<<set.maximumFrameRate();
            qWarning()<<set.resolution().width()<<"x"<<set.resolution().height();
            return;
        }
    }
}

