#include "tableview.hpp"
#include <QMessageBox>

TableView::TableView(QWidget *parent): QTableWidget(0, 3, parent) {
	//...
}

TableView::~TableView() {
	//...
}

StegArch::Const_ItemHand TableView::selectedItem() const {
	qint32 id = selectedRow();
	return id >= 0? item(id): nullptr;
}

StegArch::Const_ItemHand TableView::item(int row) const {
	auto ptr = dynamic_cast<ItemBase *>
	(QTableWidget::item(row, 0));
	return
	(ptr)? ptr->get(): nullptr;
}

qint32 TableView::selectedRow() const {
	if (!selectedItems().size()) {
		return -1;
	}
	return	//NOTE: This version supports single selection only!
	selectedItems().
	front()->row();
}

bool TableView::insertItem(int row, std::string key,
        StegArch::CompressModeFlag mod,
        QDataStream &inp) {

	StegArch::Const_ItemHand pt;
	auto it = item(row);
	if (it) {
		pt = ar->insertItem(it, key, mod, inp);
	}
	else {
		pt = ar->insertItem(key, mod, inp);
	}
	if (!pt) return false;

	insertRow(row);
	setItem(row, 0, new ItemName(this, pt));
	setItem(row, 1, new ItemSize(this, pt));
	setItem(row, 2, new ItemMode(this, pt));

	return true;
}

void TableView::update(StegArch *_ar) {
	this->reset(); ar = _ar;
	for (auto it: ar->items()) {
		uint n = rowCount();
		insertRow(n);
		setItem(n, 0, new ItemName(this, it));
		setItem(n, 1, new ItemSize(this, it));
		setItem(n, 2, new ItemMode(this, it));
	}
}

void TableView::removeItem(int row) {
	if (!ar) return;
	ar->removeItem(item(row));
	removeRow(row);
}

void TableView::reset() {
	while (rowCount())
	removeRow(0);
}

void TableView::ItemName::setData(int role, const QVariant &value) {

	if (!own->arch()->renameItem(it, value.toString().toStdString())) {
		QMessageBox::warning(own, "Error", "Too long name!");
		return;
	}
	ItemBase::setData(role, value);
}

QVariant TableView::ItemName::data(int role) const {

	if (role == Qt::DisplayRole ||
	    role == Qt::EditRole) return QVariant(it->name().c_str());

	return ItemBase::data(role);
}

QVariant TableView::ItemSize::data(int role) const {

	if (role == Qt::DisplayRole ||
	    role == Qt::EditRole) return QVariant(it->sizeData());

	return ItemBase::data(role);
}

QVariant TableView::ItemMode::data(int role) const {

	const static QString METH[] = {"None", "RLE", "LZW"};

	if (role == Qt::DisplayRole ||
	    role == Qt::EditRole) {
		return QVariant(
		METH[it->compressMode()]
		);
	}
	return ItemBase::data(role);
}
