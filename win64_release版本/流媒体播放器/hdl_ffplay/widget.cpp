#include "widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->resize(640, 480);
}

Widget::~Widget()
{

}
void Widget::paintEvent(QPaintEvent *e)
{
	qDebug() << "paintEvent";
	//mutex.lock();
	QPainter painter(this);
	//painter.drawPixmap(0,0, this->width(), this->height(), QPixmap::fromImage(this->img));
	
	painter.drawImage(0, 0, img);
	//mutex.unlock();
}


void Widget::paint_img(QImage img)
{
	qDebug() << "painterImage";
	this->img = img;
	this->update();
}
