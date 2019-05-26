#include "stegarch.hpp"

StegArch::~StegArch() { delete map; }

StegArch::StegArch(const QImage &_img, std::string _key): StegArch() {
	reset(_img, _key);
}

StegArch::StegArch() {}

bool StegArch::reset(const QImage &_img, std::string _key) {
	key = _key; return reset(_img);
}

bool StegArch::reset(const QImage &_img) {
	item.clear();
	img = _img.convertToFormat(QImage::Format_ARGB32);
	if (map) delete map;
	map = new StegMap(
		(QRgb *) img.bits(), img.byteCount() / 4, key
	);
	vol = 0;
	return map->open(QIODevice::ReadWrite);
}

bool StegArch::reset(std::string _key) {
	if (!map) return false;
	key = _key;
	return map->reset(_key);
}

StegArch::ItemPointer StegArch::getItem(uint i) {
	if (i >= item.size())
		return nullptr;
	return item[i];
}

bool StegArch::addItem(uint i, QDataStream &inp, CompressModeFlag mod) {
	quint32 len = sizeOfItemHeader() + sizeOfHeader() + vol;
	if (len > capacity()) return false;
	ItemPointer it(new Item(capacity() - len, mod));
	if (!it) return false;
	item.emplace(item.begin() + i, it);
	if (!it->read(inp)) {
		return false;
	}
	vol += sizeOfItemHeader() +
	       it->size();
	return true;
}

bool StegArch::addItem(QDataStream &inp, CompressModeFlag mod) {
	return addItem(
	item.size(), inp, mod
	);
}

void StegArch::delItem(uint i) {
	if (i >= item.size())
		return;
	item.erase(
	item.begin() + i
	);
}

BinStream *StegArch::Item::gener(QBuffer *buf, OpenModeFlag flg) {

	switch (mod) {
	case None:
		return new BinStream(buf, flg);
	case RLE:
		return new RLEStream(buf, flg);
	case LZW:
		return new LZWStream(buf, flg);
	}

	return nullptr;
}

StegArch::Item::Item(quint32 _maxSize, CompressModeFlag _mod):
	vol(_maxSize), mod(_mod) {
	dat.resize(vol);
}

#define BUFSIZE (1024)

bool StegArch::Item::write(QDataStream &out) const {

	constexpr QDataStream::Status err =
	          QDataStream::WriteFailed;
	constexpr OpenModeFlag flg =
	          QIODevice::ReadOnly;

	std::unique_ptr<BinStream> bin;
	QBuffer dev(&dat);
	dev.open(flg);

	bin.reset(gener(&dev, flg)); if (!bin) return false;

	char buf[BUFSIZE];

	while (!bin->atEnd()) {
		int n = bin->read(buf, BUFSIZE);
		if (n < 0) return false;
		out.writeRawData(buf, n);
		if (out.status() == err) {
			return false;
		}
	}
	return true;
}

bool StegArch::Item::read(QDataStream &inp) {

	constexpr QDataStream::Status err =
	          QDataStream::ReadCorruptData;
	constexpr OpenModeFlag flg =
	          QIODevice::WriteOnly;

	std::unique_ptr<BinStream> bin;
	QBuffer dev(&dat);
	dev.open(flg);
	dat.resize(0);

	bin.reset(gener(&dev, flg)); if (!bin) return false;

	char buf[BUFSIZE];

	for (int len = capacity(); len && !inp.atEnd(); ) {
		int n = std::min(len, BUFSIZE);
		n = inp.readRawData(buf, n);
		if (inp.status() == err) {
			return false;
		}
		if (bin->write(buf, n) != n) {
			return false;
		}
		len -= n;
	}
	if (!bin->flush()) {
		return false;
	}
	return true;
}

bool StegArch::encode() {

	if (!map) return false;

	map->setNoise();	//Добавляем случайный шум к изображению!

	quint32 num = numItems(); QDataStream str(map); str << num;

	for (auto &it: item) {
		quint8 mod = it->compressMode(); num = it->size();
		str << num; str << mod;
		if (str.writeRawData(it->data(), num) != num) {
			return false;
		}
	}
	if (str.status() == QDataStream::WriteFailed) {
		return false;
	}
	return true;
}

bool StegArch::decode() {

	if (!map) return false;

	std::vector<ItemPointer> temp; item.clear(); vol = 0;
	QDataStream str(map); map->reset();
	quint32 num; str >> num;

	for (int i = 0; i < num; ++ i) {
		quint32 len; str >> len;
		quint8 mod; str >> mod;
		if (!check(len)) return false;
		ItemPointer it(
		    new Item(len, mod)
		);
		temp.push_back(it);
		if (str.readRawData(it->data(), len) != len) {
		    return false;
		}
		vol += sizeOfItemHeader() + len;
	}
	if (str.status() ==
	    QDataStream::ReadCorruptData) {
	    return false;
	}
	item = std::move(temp);
	return true;
}

void StegArch::clear() {
	item.clear();
}
