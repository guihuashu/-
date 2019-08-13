#ifndef SHOWVCAP_H
#define SHOWVCAP_H

#include <QWidget>
#include <QVideoFrame>
#include <ffmopr.h>
#include <QPainter>
namespace Ui {
class ShowVCap;
}

class ShowVCap : public QWidget
{
    Q_OBJECT

public:
    explicit ShowVCap(QWidget *parent = nullptr);
    ~ShowVCap();
    virtual void paintEvent(QPaintEvent *event);
public slots:
    void showVideoFrame(QVideoFrame frame);


private:
    Ui::ShowVCap *ui;
    SwsContext *_swsNv21toRgb24 = NULL;
    QImage _img;
};

#endif // SHOWVCAP_H
