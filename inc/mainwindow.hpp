#ifndef __INCLUDE_MAINWINDOW_H
#define __INCLUDE_MAINWINDOW_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QMessageBox>
#include <QMainWindow>

#include <compressordialog.hpp>
#include <tableview.hpp>
#include <stegarch.hpp>

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
	void onAddClick();
	void onGetClick();
	void onDelClick();
	void onSelect();
private:
	void resetStatus();
	Ui::MainWindow *ui;
	StegArch *ar = 0;
};

#endif //__INCLUDE_MAINWINDOW_H
