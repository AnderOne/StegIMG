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

	//Calculation of steganographic capacity:
	uint32_t size;
	QString info = QString::number(size = ar->steg()->size()) + " bytes";
	double mb = (double) size / (1024 * 1024);
	double kb = (double) size / (1024);
	if (mb > 1.0)
		info = QString::number(mb, 'f', 2) + " MB (" + info + ")";
	else
	if (kb > 1.0) {
		info = QString::number(kb, 'f', 2) + " KB (" + info + ")";
	}
	ui->inputImageInfo->setText(
	"Capacity: " + info
	);
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

	if (!ar->reset(pass.toStdString())) {
		QMessageBox::warning(this, "Error", "Failed to apply key!");
		return;
	}

	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream input(&file);
	if (!ar->encode(input)) {
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
	//Check for extension:
	QString name = fileName.section('/', -1).toLower();
	QString end = name.section('.', -1);
	if (((end != "png") && (end != "bmp")) ||
	     (end == name))
	     fileName += "." + (end = "bmp");
	ar->image().save(
	fileName,
	end.toStdString().c_str()
	);
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
	if (!ar->reset(pass.toStdString())) {
		QMessageBox::warning(this, "Error", "Failed to apply key!");
		return;
	}

	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	if (!ar->decode(out)) {
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
