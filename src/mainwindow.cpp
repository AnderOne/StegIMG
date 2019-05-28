#include "ui_mainwindow.h"
#include "mainwindow.hpp"

#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QColor>
#include <iostream>
#include <set>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

void MainWindow::onInputImageClick() {

	QString fileName = QFileDialog::getOpenFileUrl(
	                   this, "Open BitMap", QUrl(), "Image Files (*.png *.bmp)").toLocalFile();
	if (fileName.isNull()) {
		return;
	}
	QImage img;
	if (!img.load(fileName)) {
		QMessageBox::warning(this, "Error", "Invalid image format!");
		return;
	}
	if (ar) delete ar;
	ui->inputImageView->update((ar = new StegArch(img))->image());
	ui->buttonEncrypt->setEnabled(true);
	ui->buttonDecrypt->setEnabled(true);
	ui->buttonAddItem->setEnabled(true);

	resetStatus();
}

void MainWindow::onEncryptClick() {

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
	if (!ar->encode()) {
		QMessageBox::warning(this, "Error", "Failed to encrypt!");
		return;
	}
	resetStatus();

	QString fileName = QFileDialog::getSaveFileUrl(
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

	bool flag = false;
	QString pass = QInputDialog::getText(this, "Password", "", QLineEdit::Password, "", &flag);
	if (!flag) {
		QMessageBox::information(this, "", "Operation was aborted!");
		return;
	}
	if (pass.isNull()) {
		pass = "";
	}
	ar->clear();

	if (!ar->reset(pass.toStdString())) {
		QMessageBox::warning(this, "Error", "Failed to apply key!");
		resetStatus();
		return;
	}
	if (!ar->decode()) {
		QMessageBox::warning(this, "Error", "Failed to decrypt!");
		resetStatus();
		return;
	}
	resetStatus();
}

void MainWindow::onAddClick() {

	QString fileName = QFileDialog::getOpenFileUrl(this, "Open File", QUrl()).toLocalFile();
	if (fileName.isNull()) return;

	CompressorDialog dialog(this);
	if (!dialog.exec()) {
		QMessageBox::information(this, "", "Operation was aborted!");
		return;
	}
	StegArch::CompressModeFlag mod = dialog.getCompressMode();

	QFile file(fileName); file.open(QIODevice::ReadOnly);
	QDataStream inp(&file);
	if (!ar->addItem("TEST!", mod, inp)) {
		QMessageBox::warning(this, "Error", "Too long file!");
		return;
	}
	resetStatus();
}

void MainWindow::onGetClick() {

	std::set<int> ind; for (auto it: ui->tableWidget->selectedItems()) ind.insert(it->row());
	if (ind.size() != 1) {
		return;
	}
	QString fileName = QFileDialog::getSaveFileUrl(this, "Save File", QUrl()).toLocalFile();
	if (fileName.isNull()) {
		return;
	}
	QFile file(fileName); file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	auto it = ar->getItem(*ind.begin());
	if (!it->write(out)) {
		QMessageBox::warning(
		this, "Error",
		"File save failed!"
		);
		return;
	}
}

void MainWindow::onDelClick() {
	std::set<int> ind;
	for (auto it: ui->tableWidget->selectedItems()) ind.insert(it->row());
	for (auto i: ind) {
		ar->delItem(i);
	}
	resetStatus();
}

void MainWindow::onSelect() {
	bool fl = !ui->tableWidget->selectedItems().empty();
	ui->buttonGetItem->
	    setEnabled(fl);
	ui->buttonDelItem->
	    setEnabled(fl);
}

void MainWindow::resetStatus() {

	ui->hiddenDataSize->setEnabled(false); ui->inputImageInfo->setText("");
	while (ui->tableWidget->rowCount()) ui->tableWidget->removeRow(0);
	if (!ar) return;

	const static QString METH[] = {"None", "RLE", "LZW"};

	for (int i = 0; i < ar->numItems(); ++ i) {
		auto it = ar->getItem(i);
		ui->tableWidget->insertRow(i);
		ui->tableWidget->setItem(
		i, 0, new QTableWidgetItem(QString(it->name().c_str()))
		);
		ui->tableWidget->setItem(
		i, 1, new QTableWidgetItem(QString::number(it->size()))
		);
		ui->tableWidget->setItem(
		i, 2, new QTableWidgetItem(METH[it->compressMode()])
		);
	}

	//Calculation of steganographic capacity:
	uint32_t size;
	QString info = QString::number(size = ar->capacity()) +
	             " bytes";
	double mb = (double) size / (1024 * 1024);
	double kb = (double) size / (1024);
	if (mb > 1.0)
		info = QString::number(mb, 'f', 2) +
		     " MB (" + info + ")";
	else
	if (kb > 1.0) {
		info = QString::number(kb, 'f', 2) +
		     " KB (" + info + ")";
	}
	ui->inputImageInfo->setText(
	"Capacity: " + info
	);

	//Change for hidden data size:
	size_t vol = ar->size();
	ui->hiddenDataSize->setEnabled(vol);
	ui->hiddenDataSize->setValue(
	100 * (double) vol / size
	);
}

MainWindow::~MainWindow() {
	delete ar;
	delete ui;
}
