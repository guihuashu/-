#include "showvcap.h"
#include "ui_showvcap.h"

ShowVCap::ShowVCap(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowVCap)
{
    ui->setupUi(this);
    _swsNv21toRgb24 = sws_getCachedContext(_swsNv21toRgb24, \
        640, 480, AV_PIX_FMT_NV21, \
        640, 480, AV_PIX_FMT_RGB24, \
        SWS_BICUBIC, NULL, NULL, NULL);
}

ShowVCap::~ShowVCap()
{
    delete ui;
}

void ShowVCap::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawImage(0,0,_img);
}

void ShowVCap::showVideoFrame(QVideoFrame vf)
{
    AVFrame *frame;

    //qInfo()<<
    if (!FFmOpr::allocFrame(&frame, 640, 480, AV_PIX_FMT_RGB24)) {
        qWarning()<<"allocFrame err";
        return;
    }
    int ret = FFmOpr::NV21toRgb24(_swsNv21toRgb24, vf, frame);
    qWarning()<<"ret = "<< ret;
    _img = QImage(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
    update();
}

