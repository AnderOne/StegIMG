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
	struct Buffer: public QBuffer {
		template <typename ... T> Buffer(size_t max, T ... arg):
		       QBuffer(arg ...),
		       maxSize(max) {}
		qint64 writeData(const char *dat, qint64 len) override;
	private:
		size_t maxSize;
	};

	enum CompressModeFlag {
		None = 0,
		RLE = 1,
		LZW = 2
	};

	struct Item {
		CompressModeFlag compressMode() const { return mod; }
		std::string name() const { return key; }
		const char *data() const { return dat.data(); }
		char *data() { return dat.data(); }

		quint32 sizeHead() const {
			return sizeof(quint32) + sizeof(quint8) +
			       key.size() + sizeof(quint8);
		}
		quint32 sizeData() const { return dat.size(); }
		quint32 size() const {
			return sizeHead() + sizeData();
		}

		bool writeHead(QDataStream &out) const;
		bool readHead(QDataStream &inp);
		bool writeData(QDataStream &out) const;
		bool readData(QDataStream &inp);

		bool write(QDataStream &out) const;
		bool read(QDataStream &inp);

	private:
		Item(StegArch &_own, std::string _key, CompressModeFlag _mod):
		     own(_own), key(_key), mod(_mod) {}
		Item(StegArch &_own): own(_own) {}

		typedef QIODevice::OpenModeFlag OpenModeFlag;
		friend struct StegArch;

		BinStream *gener(
		    QBuffer *, OpenModeFlag
		    ) const;

		CompressModeFlag mod;
		std::string key;
		QByteArray dat;
		StegArch &own;
	};

	typedef std::shared_ptr<Item> ItemHand;

	virtual ~StegArch();
	explicit StegArch(const QImage &img, std::string key = "");
	StegArch();

	bool reset(const QImage &img, std::string key);
	bool reset(const QImage &img);
	bool reset(std::string key);

	ItemHand getItem(uint i);
	quint32 numItems() const {
		return item.size();
	}
	bool addItem(
	        uint i, std::string key, CompressModeFlag mod,
	        QDataStream &inp);
	bool addItem(
	        std::string key, CompressModeFlag mod,
	        QDataStream &inp);
	bool addItem(
	        uint i,
	        QDataStream &inp);
	bool addItem(
	        QDataStream &inp);
	void delItem(uint i);

	bool encode();
	bool decode();
	void clear();

	constexpr static quint32 sizeHead() {
		return sizeof(quint32);
	}
	const StegMap *steg() const {
		return map;
	}
	const QImage &image() const {
		return img;
	}
	quint32 capacity() const {
		return map->size();
	}
	quint32 size() const {
		return vol;
	}

private:
	std::vector<ItemHand> item;
	std::string key;
	StegMap *map = nullptr;
	QImage img;
	size_t vol = 0;
};

#endif //__INCLUDE_STEGARCH_H
