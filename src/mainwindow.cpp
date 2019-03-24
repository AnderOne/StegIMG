#include "ui_mainwindow.h"
#include "mainwindow.hpp"

#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <iostream>
#include <QColor>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

void MainWindow::onInputImageClick() {

	QString fileName = QFileDialog::getOpenFileUrl(this, "Open BitMap", QUrl(), "Image Files (*.png *.bmp)").toLocalFile();
	if (fileName.isNull()) return;
	QImage img;
	if (!img.load(fileName)) {
		QMessageBox::warning(this, "Error", "Invalid image format!");
		return;
	}
	if (ar) delete ar;
	ui->inputImageView->update((ar = new StegArch(img))->image());
	ui->buttonEncrypt->setEnabled(true);
	ui->buttonDecrypt->setEnabled(true);
}

void MainWindow::onEncryptClick() {

	QString fileName = QFileDialog::getOpenFileUrl(this, "Open File", QUrl()).toLocalFile();
	if (fileName.isNull()) return;

	bool flag = false;
	QString pass = QInputDialog::getText(this, "Password", "", QLineEdit::Password, "", &flag);
	if (!flag) {
		QMessageBox::information(this, "", "Operation was aborted!");
		return;
	}
	if (pass.isNull()) {
		pass = "";
	}

	QFile file(fileName); file.open(QIODevice::ReadOnly);
	QDataStream input(&file);
	if (!ar->encode(input, pass.toStdString())) {
		QMessageBox::warning(this, "Error", "Too long file!");
		return;
	}

	fileName = QFileDialog::getSaveFileUrl(
		this, "Save File", QUrl(), "Image Files (*.png *.bmp)"
	).toLocalFile();
	if (fileName.isNull()) {
		QMessageBox::information(
		this, "", "Operation was aborted!"
		);
		return;
	}
	ar->image().save(fileName, "png");
}

void MainWindow::onDecryptClick() {

	QString fileName = QFileDialog::getSaveFileUrl(this, "Save File", QUrl()).toLocalFile();
	if (fileName.isNull()) return;

	bool flag = false;
	QString pass = QInputDialog::getText(this, "Password", "", QLineEdit::Password, "", &flag);
	if (!flag) {
		QMessageBox::information(this, "", "Operation was aborted!");
		return;
	}
	if (pass.isNull()) {
		pass = "";
	}

	QFile file(fileName); file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	if (!ar->decode(out, pass.toStdString())) {
		file.remove();
		QMessageBox::warning(this, "Error", "Invalid data!");
		return;
	}
	file.close();
}

MainWindow::~MainWindow() {
	delete ar;
	delete ui;
}
