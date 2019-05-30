#ifndef __INCLUDE_TABLEVIEW_H
#define __INCLUDE_TABLEVIEW_H

#include <QAbstractTableModel>
#include <QTableWidget>
#include <stegarch.hpp>

class TableView: public QTableWidget {
	Q_OBJECT

protected:
	typedef QTableWidgetItem Item;

	struct ItemBase:
	public Item {
		ItemBase(TableView *_own, StegArch::Const_ItemHand _it):
		QTableWidgetItem(_own),
		own(_own),
		it(_it) {
			setFlags(flags() & ~Qt::ItemIsEditable);
		}
		const StegArch::Const_ItemHand &get() const {
			return it;
		}
	protected:
		StegArch::Const_ItemHand it;
		TableView *own;
	};

	struct ItemName:
	public ItemBase {

		ItemName(TableView *_own, StegArch::Const_ItemHand _it):
		ItemBase(_own, _it) {
			setFlags(flags() | Qt::ItemIsEditable);
		}

		QVariant data(int role) const override;
		void setData(
		int role, const QVariant &value
		) override;
	};

	struct ItemSize:
	public ItemBase {

		ItemSize(TableView *_own, StegArch::Const_ItemHand _it):
		ItemBase(_own, _it) {}

		QVariant data(int role) const override;
	};

	struct ItemMode:
	public ItemBase {
	
		ItemMode(TableView *_own, StegArch::Const_ItemHand _it):
		ItemBase(_own, _it) {}

		QVariant data(int role) const override;
	};

public:
	const StegArch *arch() const { return ar; }

	StegArch *arch() { return ar; }

	StegArch::Const_ItemHand selectedItem() const;

	StegArch::Const_ItemHand item(int row) const;

	qint32 selectedRow() const;

	bool insertItem(int row, std::string key,
	         StegArch::CompressModeFlag mod,
	         QDataStream &inp);

	void removeItem(int row);

	TableView(QWidget *parent = nullptr);

	void update(StegArch *ar);
	void reset();

	~TableView();

private:
	StegArch *ar = 0;
};

#endif //__INCLUDE_TABLEVIEW_H
