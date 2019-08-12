#include "qtvideocap.h"


QtVideoCap::QtVideoCap(QSize inSize, double fps, QVideoFrame::PixelFormat inFmt,
                       QObject *parent)
{
    _inSize = inSize;
    _fps = fps;
    _inFmt = inFmt;

    if (_inFmt != QVideoFrame::Format_NV21) {
        qWarning()<<"cannot unsupport cap fmt";
        getchar();
    }
    init();
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
    CUR;
    // 在newVideoFrame函数中捕获视频帧,  当相机成功加载后,在stateChanged函数中设置相机
    //connect(_probe, &QVideoProbe::videoFrameProbed, this, &QtVideoCap::newVideoFrame);
    connect(_camera, &QCamera::stateChanged, this, &QtVideoCap::stateChanged);

    _camera->setCaptureMode(QCamera::CaptureVideo);   
    _camera->start();
}


void QtVideoCap::stateChanged(QCamera::State state)
{
    // 相机处于激活状态
    if (QCamera::ActiveState != state)
        return;

    QList<QCameraViewfinderSettings> list = _camera->supportedViewfinderSettings();
    for (int i=0; i < list.size(); i++)
    {
        QCameraViewfinderSettings set = list.at(i);
        qInfo()<<"------------support ----------------";
        qInfo()<<set.pixelFormat();
        qInfo()<<"minimumFrameRate: "<<set.minimumFrameRate();
        qInfo()<<"maximumFrameRate: "<<set.maximumFrameRate();
        qInfo()<<"resolution: "<<set.resolution().width()<<"x"<<set.resolution().height();
        if (set.resolution() == _inSize \
           &&  (set.maximumFrameRate()>=_fps) \
           &&  (set.maximumFrameRate()<(_fps +10)) \
           &&  (set.pixelFormat() == _inFmt))
        {
            _camera->setViewfinderSettings(set);
            qInfo()<<"--------- seted -------------------";
            qInfo()<<"minimumFrameRate: "<<set.minimumFrameRate();
            qInfo()<<"maximumFrameRate: "<<set.maximumFrameRate();
            qInfo()<<"resolution: "<<set.resolution().width()<<"x"<<set.resolution().height();
            return;
        }
    }
    CUR;
    qWarning()<<"unsupport video set, please Please select the correct format ";
    getchar();
}

