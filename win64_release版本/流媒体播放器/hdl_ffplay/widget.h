#pragma once

#include <QWidget>
#include "ui_widget.h"
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QMutex>


class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget *parent = Q_NULLPTR);
	virtual void paintEvent(QPaintEvent *e);
	void paint_img(QImage img);
	~Widget();
	QMutex mutex;
private:
	Ui::Widget ui;
	QImage img;
	QPaintEvent *e;

};
#define CUR qDebug()<<__FILE__<<","<<__LINE__
