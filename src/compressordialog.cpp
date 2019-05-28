#include "ui_compressordialog.h"
#include "compressordialog.hpp"

CompressorDialog::CompressorDialog(QWidget *parent):
	QDialog(parent), ui(new Ui::CompressorDialog) {
	mode = CompressModeFlag::None;
	ui->setupUi(this);
}

CompressorDialog::~CompressorDialog() { delete ui; }

CompressorDialog::CompressModeFlag
CompressorDialog::getCompressMode() const {
	return mode;
}

std::string CompressorDialog::getItemName() const {
	return ui->lineEdit->text().toStdString();
}

void CompressorDialog::onNone_Click() {
	mode = CompressModeFlag::None;
}

void CompressorDialog::onRLE_Click() {
	mode = CompressModeFlag::RLE;
}

void CompressorDialog::onLZW_Click() {
	mode = CompressModeFlag::LZW;
}
