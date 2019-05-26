#ifndef __INCLUDE_STEGARCH_H
#define __INCLUDE_STEGARCH_H

#include <binstream.hpp>
#include <rlestream.hpp>
#include <lzwstream.hpp>
#include <stegmap.hpp>
#include <memory>
#include <QDataStream>
#include <QBuffer>
#include <QImage>

class StegArch {

public:
	enum CompressModeFlag { None = 0, RLE = 1, LZW = 2 };

	struct Item {
		explicit Item(quint32 maxSize, CompressModeFlag mod);
		inline CompressModeFlag compressMode() const { return mod; }
		inline quint32 capacity() const { return vol; }
		inline quint32 size() const { return dat.size(); }
		inline char *data() { return dat.data(); }
		bool write(QDataStream &out) const;
		bool read(QDataStream &inp);
	private:
		typedef QIODevice::OpenModeFlag OpenModeFlag;
		BinStream *gener(QBuffer *, OpenModeFlag);
		CompressModeFlag mod;
		QByteArray dat;
		quint32 vol;
	};

	typedef std::shared_ptr<Item> ItemPointer;

	virtual ~StegArch();
	explicit StegArch(const QImage &img, std::string key = "");
	StegArch();

	bool reset(const QImage &img, std::string key);
	bool reset(const QImage &img);
	bool reset(std::string key);

	ItemPointer getItem(uint i);
	inline quint32 numItems() const { return item.size(); }
	bool addItem(uint i, QDataStream &inp, CompressModeFlag mod);
	bool addItem(QDataStream &inp, CompressModeFlag mod);
	void delItem(uint i);

	bool encode();
	bool decode();
	void clear();

	constexpr static quint32 sizeOfItemHeader() {
		return sizeof(quint32) + 1;
	}
	constexpr static quint32 sizeOfHeader() {
		return sizeof(quint32);
	}
	inline const StegMap *steg() const {
		return map;
	}
	inline const QImage &image() const {
		return img;
	}
	inline quint32 capacity() const {
		return map->size();
	}
	inline quint32 size() const {
		return vol;
	}

private:
	bool check(quint32 len) {
		return sizeOfItemHeader() +
		       sizeOfHeader() +
		       vol + len <=
		       capacity();
	}

	std::vector<ItemPointer> item;
	std::string key;
	StegMap *map = nullptr;
	QImage img;
	size_t vol = 0;
};

#endif //__INCLUDE_STEGARCH_H
