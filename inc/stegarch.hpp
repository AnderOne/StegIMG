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

		struct Head {
			Head(std::string _key, CompressModeFlag _mod, quint32 _vol): key(_key), mod(_mod), vol(_vol) {}
			CompressModeFlag compressMode() const { return mod; }
			std::string name() const { return key; }
			quint32 capacity() const { return vol; }
			quint32 size() const {
				return sizeof(quint32) +
				       sizeof(quint8) + key.size() +
				       sizeof(quint8);
			}
		private:
			friend struct Item;
			CompressModeFlag mod;
			std::string key;
			quint32 vol;
		};

		Item(std::string key, CompressModeFlag mod, quint32 vol): inf{key, mod, vol} { dat.resize(vol); }

		CompressModeFlag compressMode() const { return inf.compressMode(); }
		quint32 capacity() const { return inf.capacity(); }
		std::string name() const { return inf.name(); }

		quint32 sizeHead() const { return inf.size(); }
		quint32 sizeData() const { return dat.size(); }
		quint32 size() const { return sizeHead() + sizeData(); }

		const Head &head() const { return inf; }
		const char *data() const { return dat.data(); }
		char *data() { return dat.data(); }

		bool write(QDataStream &out) const;
		bool read(QDataStream &inp);
	private:
		typedef QIODevice::OpenModeFlag
		OpenModeFlag;

		BinStream *gener(
		    QBuffer *, OpenModeFlag
		    ) const;
		Item::Head inf;
		QByteArray dat;
	};

	typedef std::shared_ptr<Item>
	ItemPointer;

	virtual ~StegArch();
	explicit StegArch(const QImage &img, std::string key = "");
	StegArch();

	bool reset(const QImage &img, std::string key);
	bool reset(const QImage &img);
	bool reset(std::string key);

	ItemPointer getItem(uint i);
	quint32 numItems() const {
		return item.size();
	}
	bool addItem(
	        uint i, std::string key, CompressModeFlag mod,
	        QDataStream &inp);
	bool addItem(
	        std::string key, CompressModeFlag mod,
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
	ItemPointer newItem(
	    std::string key, CompressModeFlag mod,
	    quint32 vol
	    );

	std::vector<ItemPointer> item;
	std::string key;
	StegMap *map = nullptr;
	QImage img;
	size_t vol = 0;
};

#endif //__INCLUDE_STEGARCH_H
