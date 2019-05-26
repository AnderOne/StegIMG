#ifndef __INCLUDE_COMPRESSORDIALOG_H
#define __INCLUDE_COMPRESSORDIALOG_H

#include <QInputDialog>
#include <stegarch.hpp>

namespace Ui {
class CompressorDialog;
}

class CompressorDialog: public QDialog {
	Q_OBJECT
public:
	typedef StegArch::CompressModeFlag CompressModeFlag;
	explicit CompressorDialog(QWidget *parent = 0);
	~CompressorDialog();
	CompressModeFlag getCompressMode() const;
private slots:
	void onNone_Click();
	void onRLE_Click();
	void onLZW_Click();
private:
	Ui::CompressorDialog *ui;
	CompressModeFlag mode;
};

#endif //__INCLUDE_COMPRESSORDIALOG_H
