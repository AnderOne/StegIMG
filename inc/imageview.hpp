#ifndef __INCLUDE_IMAGEVIEW_H
#define __INCLUDE_IMAGEVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>

class ImageView: public QGraphicsView {
	Q_OBJECT
public:
	explicit ImageView(QWidget *parent = nullptr);
	void update(const QImage &image);
	~ImageView();

protected:
	void resizeEvent(QResizeEvent *event);
private:
	QGraphicsPixmapItem *item = nullptr;
	QGraphicsScene *scene;
};

#endif //__INCLUDE_IMAGEVIEW_H
