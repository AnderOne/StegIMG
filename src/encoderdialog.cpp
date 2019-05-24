#include "ui_encoderdialog.h"
#include "encoderdialog.hpp"

EncoderDialog::EncoderDialog(QWidget *parent): QDialog(parent), ui(new Ui::EncoderDialog) {
	mode = CompressModeFlag::None; ui->setupUi(this);
}

EncoderDialog::~EncoderDialog() { delete ui; }

EncoderDialog::CompressModeFlag EncoderDialog::getCompressMode() const {
	return mode;
}

QString EncoderDialog::getPassword() const {
	return ui->lineEdit_Pasword->text();
}

void EncoderDialog::onNone_Click() {
	mode = CompressModeFlag::None;
}

void EncoderDialog::onRLE_Click() {
	mode = CompressModeFlag::RLE;
}

void EncoderDialog::onLZW_Click() {
	mode = CompressModeFlag::LZW;
}
