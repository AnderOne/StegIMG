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

bool StegArch::addItem(uint i, std::string key, CompressModeFlag mod, QDataStream &inp) {
	Item::Head head(key, mod, 0); quint32 len = sizeHead() + size() + head.size();
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

StegArch::ItemPointer StegArch::newItem(const StegArch::Item::Head &head) {

	if (sizeHead() + size() + head.size() + head.capacity() <= capacity()) {
		return ItemPointer(new Item(head));
	}
	return nullptr;
}

StegArch::ItemPointer StegArch::getItem(uint i) {
	if (i >= item.size())
		return nullptr;
	return item[i];
}

void StegArch::delItem(uint i) {
	if (i >= item.size()) return;
	vol -= item[i]->size();
	item.erase(
	item.begin() + i
	);
}

BinStream *StegArch::Item::gener(QBuffer *buf, OpenModeFlag flg) const {

	switch (compressMode()) {
	case None: return new BinStream(buf, flg);
	case RLE: return new RLEStream(buf, flg);
	case LZW: return new LZWStream(buf, flg);
	}

	return nullptr;
}

qint64 StegArch::Buffer::writeData(const char *dat, qint64 len) {

	if (buffer().size() + len > maxSize) {
		return -1;
	}
	return QBuffer::writeData(dat, len);
}

bool StegArch::Item::Head::write(QDataStream &out) const {
	out << (quint8) key.size();
	if (out.writeRawData(key.c_str(), key.size()) != key.size()) {
		return false;
	}
	out << (quint8) mod;
	out << vol;
	return out.status() != QDataStream::WriteFailed;
}

bool StegArch::Item::Head::read(QDataStream &inp) {

	quint8 len; inp >> len; char buf[len];
	if (inp.readRawData(buf, len) != len) {
		return false;
	}
	key = std::string(buf, len);
	inp >> len;
	mod = CompressModeFlag(len);
	inp >> vol;
	return inp.status() !=
	QDataStream::
	ReadCorruptData;
}

bool StegArch::Item::writeData(QDataStream &out) const {
	return out.writeRawData(data(), sizeData()) == sizeData();
}

bool StegArch::Item::readData(QDataStream &inp) {
	return inp.readRawData(data(), sizeData()) == sizeData();
}

bool StegArch::Item::writeHead(QDataStream &out) const {
	return inf.write(out);
}
bool StegArch::Item::readHead(QDataStream &inp) {
	return inf.read(inp);
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
	inf.vol = sizeData();
	return true;
}

bool StegArch::encode() {

	if (!map) return false;

	map->setNoise();	//Добавляем случайный шум к изображению!

	quint32 num = numItems(); QDataStream str(map); str << num;

	for (auto &it: item) {
		if (!it->writeHead(str) || !it->writeData(str)) {
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
		Item::Head h; if (!h.read(str)) return false;
		vol = v;
		ItemPointer it = newItem(h);
		vol = 0;
		if (!it) return false;
		temp.push_back(it);
		if (!it->readData(str)) {
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
