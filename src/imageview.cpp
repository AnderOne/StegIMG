#include <imageview.hpp>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QWheelEvent>

ImageView::ImageView(QWidget *parent): QGraphicsView(parent) {
	this->setScene(scene = new QGraphicsScene(this));
}

void ImageView::resizeEvent(QResizeEvent *event) {
	if (item) fitInView(item, Qt::KeepAspectRatio);
}

void ImageView::update(const QImage &image) {

	item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	scene->clear();
	scene->setSceneRect(0, 0, image.width(), image.height());
	scene->addItem(item);
	fitInView(item, Qt::KeepAspectRatio);
}

ImageView::~ImageView() {
	delete scene;
}
