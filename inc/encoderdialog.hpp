#ifndef __INCLUDE_ENCODERDIALOG_H
#define __INCLUDE_ENCODERDIALOG_H

#include <QInputDialog>
#include <stegarch.hpp>

namespace Ui {
class EncoderDialog;
}

class EncoderDialog: public QDialog {
	Q_OBJECT
public:
	typedef StegArch::CompressModeFlag CompressModeFlag;
	explicit EncoderDialog(QWidget *parent = 0);
	~EncoderDialog();
	CompressModeFlag getCompressMode() const;
	QString getPassword() const;
private slots:
	void onNone_Click();
	void onRLE_Click();
	void onLZW_Click();
private:
	Ui::EncoderDialog *ui;
	CompressModeFlag
	mode;
};

#endif //__INCLUDE_ENCODERDIALOG_H
