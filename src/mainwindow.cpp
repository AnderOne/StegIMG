#include "ui_mainwindow.h"
#include <mainwindow.hpp>

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
		QMessageBox::warning(this, "Error", "Failed to load image!");
		return;
	}
	if (ar) delete ar;

	ar = new StegArch(img);
	ui->buttonAddBefore->setEnabled(true);
	ui->buttonAddAfter->setEnabled(true);
	ui->buttonEncrypt->setEnabled(true);
	ui->buttonDecrypt->setEnabled(true);
	ui->imageView->update(ar->image());
	ui->tableView->update(ar);

	resetStatus();
}

void MainWindow::onEncryptClick() {

	bool flag = false;
	QString pass = QInputDialog::getText(this, "Password", "", QLineEdit::Password, "", &flag);
	if (!flag) {
		QMessageBox::information(this, "", "Operation is aborted!");
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
	if (!ar->image().save(
	fileName, end.toStdString().c_str()
	)) {
		QMessageBox::warning(
		this, "Error",
		"Failed to save file!"
		);
		return;
	}
}

void MainWindow::onDecryptClick() {

	bool flag = false;
	QString pass = QInputDialog::getText(this, "Password", "", QLineEdit::Password, "", &flag);
	if (!flag) {
		QMessageBox::information(this, "", "Operation is aborted!");
		return;
	}
	if (pass.isNull()) pass = "";

	ui->tableView->reset();
	ar->clear();
	resetStatus();

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
	ui->tableView->update(ar);
	resetStatus();
}

void MainWindow::onAddClick(bool after) {

	QString fileName = QFileDialog::getOpenFileUrl(this, "Open File", QUrl()).toLocalFile();
	if (fileName.isNull()) return;

	CompressorDialog dialog(this);
	if (!dialog.exec()) {
		QMessageBox::information(this, "", "Operation is aborted!");
		return;
	}
	StegArch::CompressModeFlag mod = dialog.getCompressMode();
	std::string key = dialog.getItemName();

	qint32 pos = ui->tableView->selectedRow();
	if (pos < 0) {
		pos = after?
		      ui->tableView->rowCount(): 0;
	}
	else {
		pos += after;
	}

	QFile file(fileName); file.open(QIODevice::ReadOnly);
	if (!file.isReadable()) {
		QMessageBox::warning(
		this, "Error", "Failed to read file!"
		);
		return;
	}
	QDataStream inp(&file);
	if (!ui->tableView->insertItem(
	    pos, key, mod, inp)) {
		QMessageBox::warning(
		this, "Error", "Too long file!"
		);
		return;
	}
	resetStatus();
}

void MainWindow::onAddBeforeClick() {
	onAddClick(false);
}

void MainWindow::onAddAfterClick() {
	onAddClick(true);
}

void MainWindow::onGetClick() {

	StegArch::Const_ItemHand it = ui->tableView->selectedItem();
	if (!it) return;

	QString fileName = QFileDialog::getSaveFileUrl(this, "Save File", QUrl()).toLocalFile();
	if (fileName.isNull()) {
		QMessageBox::information(this, "", "Operation is aborted!");
		return;
	}
	QFile file(fileName); file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	if (!it->write(out)) {
		QMessageBox::warning(
		this, "Error",
		"Failed to save file!"
		);
		return;
	}
}

void MainWindow::onSelect() {
	bool fl = !ui->tableView->selectedItems().empty();
	ui->buttonGet->setEnabled(fl);
	ui->buttonDel->setEnabled(fl);
}

void MainWindow::onDelClick() {
	qint32 id = ui->tableView->selectedRow();
	if (id < 0) return;
	ui->tableView->removeItem(id);
	resetStatus();
}

void MainWindow::resetStatus() {

	ui->hiddenDataSize->setEnabled(false); ui->imageInfo->setText("");
	if (!ar) return;

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
	ui->imageInfo->setText(
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
