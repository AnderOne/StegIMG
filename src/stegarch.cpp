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

StegArch::ItemPointer StegArch::newItem(std::string key, CompressModeFlag mod, quint32 vol) {

	if (sizeOfItemHeader() +
	    sizeOfHeader() +
	    key.size() + 1 +
	    size() + vol <=
	    capacity()) return ItemPointer(new Item(key, mod, vol));

	return nullptr;
}

StegArch::ItemPointer StegArch::getItem(uint i) {
	if (i >= item.size())
		return nullptr;
	return item[i];
}

bool StegArch::addItem(uint i, std::string key, CompressModeFlag mod, QDataStream &inp) {
	quint32 len = sizeOfItemHeader() + sizeOfHeader() + key.size() + 1 + vol;
	if (len > capacity()) return false;
	ItemPointer it(new Item(key, mod, capacity() - len));
	if (!it) return false;
	if (!it->read(inp)) {
		return false;
	}
	item.emplace(item.begin() + i, it);
	vol += it->size();
	return true;
}

bool StegArch::addItem(std::string key, CompressModeFlag mod, QDataStream &inp) {
	return addItem(item.size(), key, mod, inp);
}

void StegArch::delItem(uint i) {
	if (i >= item.size()) return;
	vol -= item[i]->size();
	item.erase(
	item.begin() + i
	);
}

qint64 StegArch::Buffer::writeData(const char *dat, qint64 len) {

	if (buffer().size() + len > maxSize) {
		return -1;
	}
	return QBuffer::writeData(dat, len);
}

BinStream *StegArch::Item::gener(QBuffer *buf, OpenModeFlag flg) const {

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

StegArch::Item::Item(std::string _key, CompressModeFlag _mod, quint32 _vol):
	key(_key), mod(_mod), vol(_vol) {
	dat.resize(vol);
}

#define BUFSIZE (1024)

bool StegArch::Item::write(QDataStream &out) const {

	constexpr QDataStream::Status err =
	          QDataStream::WriteFailed;
	constexpr OpenModeFlag flg =
	          QIODevice::ReadOnly;

	std::unique_ptr<BinStream> bin;
	StegArch::Buffer dev(dat.size(), &dat); dev.open(flg);

	bin.reset(gener(&dev, flg)); if (!bin) return false;

	char buf[BUFSIZE];

	while (!bin->atEnd()) {
		int n;
		if ((n = bin->read(buf, BUFSIZE)) < 0) {
			return false;
		}
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
	StegArch::Buffer dev(capacity(), &dat); dev.open(flg);
	dat.resize(0);

	bin.reset(gener(&dev, flg)); if (!bin) return false;

	char buf[BUFSIZE];

	while (!inp.atEnd()) {
		int n = inp.readRawData(buf, BUFSIZE);
		if (inp.status() == err) {
			return false;
		}
		if (bin->write(buf, n) != n) {
			return false;
		}
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
		quint8 len = it->name().size(); str << len;
		char *s = it->name().c_str();
		str.writeRawData(s, len);
		quint8 mod = it->compressMode();
		num = it->sizeData();
		str << mod << num;
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
	quint32 v = 0;
	quint32 num; str >> num;

	for (int i = 0; i < num; ++ i) {
		quint8 n; str >> n; char buf[n];
		if (str.readRawData(buf, n) != n) return false;
		std::string s(buf, n);
		quint8 m; str >> m; quint32 len; str >> len;
		vol = v;
		auto it = newItem(s, m, len);
		vol = 0;
		if (!it) return false;
		temp.push_back(it);
		if (str.readRawData(it->data(), len) != len) {
		    return false;
		}
		v += it->size();
	}
	if (str.status() ==
	    QDataStream::ReadCorruptData) {
	    return false;
	}
	item = std::move(temp);
	vol = v;
	return true;
}

void StegArch::clear() {
	item.clear();
}
