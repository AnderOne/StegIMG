#ifndef __INCLUDE_MAINWINDOW_H
#define __INCLUDE_MAINWINDOW_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QMainWindow>

#include "stegarch.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
private slots:
	void onInputImageClick();
	void onEncryptClick();
	void onDecryptClick();
private:
	Ui::MainWindow *ui;
	StegArch *ar = 0;
};

#endif //__INCLUDE_MAINWINDOW_H
