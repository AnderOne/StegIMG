#include "stegarch.hpp"

StegArch::~StegArch() { delete map; }

StegArch::StegArch(const QImage &_img, std::string _key) {
	reset(_img, _key);
	dict[nullptr] = item.end();
}

bool StegArch::reset(const QImage &_img, std::string _key) {
	key = _key;
	return
	reset(_img);
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

bool StegArch::insertItem(const Const_ItemHand &pos, std::string key, CompressModeFlag mod, QDataStream &inp) {

	ItemHand itr(new Item(*this, key, mod));
	if (!itr) return false;
	quint32 len = sizeHead() + size() +
	        itr->sizeHead();
	if (len > capacity() ||
	    !itr->read(inp)) {
		return false;
	}
	dict[itr.get()] = item.insert(
	dict[pos.get()], itr
	);
	vol += itr->size();
	return true;
}

bool StegArch::insertItem(std::string key, CompressModeFlag mod, QDataStream &inp) {
	return insertItem(nullptr, key, mod, inp);
}

bool StegArch::insertItem(const Const_ItemHand &pos, QDataStream &inp) {

	ItemHand itr(new Item(*this));
	if (!itr || !itr->readHead(inp) ||
	    !itr->readData(inp)) {
		return false;
	}
	dict[itr.get()] = item.insert(
	dict[pos.get()], itr
	);
	vol += itr->size();
	return true;
}

bool StegArch::insertItem(QDataStream &inp) {
	return insertItem(nullptr, inp);
}

void StegArch::removeItem(const Const_ItemHand &itr) {
	if (itr == nullptr) return;
	item.erase(dict[itr.get()]);
	dict.erase(itr.get());
	vol -= itr->size();
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

bool StegArch::Item::writeData(QDataStream &out) const {
	return out.writeRawData(data(), sizeData()) == sizeData();
}

bool StegArch::Item::writeHead(QDataStream &out) const {
	out << (quint8) key.size();
	if (out.writeRawData(
	    key.c_str(), key.size()) != key.size()) {
		return false;
	}
	out << (quint8) compressMode();
	out << sizeData();
	return out.status() !=
	QDataStream::
	WriteFailed;
}

bool StegArch::Item::readData(QDataStream &inp) {
	return inp.readRawData(
	data(), sizeData()
	) == sizeData();
}

bool StegArch::Item::readHead(QDataStream &inp) {

	quint32 vol; quint8 len; inp >> len;
	char buf[len];
	if (inp.readRawData(buf, len) != len) return false;
	key = std::string(buf, len);
	inp >> len;
	mod = CompressModeFlag(len);
	inp >> vol;
	if (own.sizeHead() + own.size() +
	    sizeHead() + vol >
	    own.capacity()) {
	    return false;
	}
	dat.resize(vol);
	return inp.status() !=
	QDataStream::
	ReadCorruptData;
}

#define BUFSIZE (1024)

bool StegArch::Item::write(QDataStream &out) const {

	constexpr QDataStream::Status err = QDataStream::WriteFailed;
	constexpr OpenModeFlag flg = QIODevice::ReadOnly;

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

	constexpr QDataStream::Status err = QDataStream::ReadCorruptData;
	constexpr OpenModeFlag flg = QIODevice::WriteOnly;

	std::unique_ptr<BinStream> bin;
	quint32 len = own.capacity() - (
	              own.sizeHead() + own.size() + sizeHead()
	              );
	StegArch::Buffer dev(len, &dat); dev.open(flg);
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

	quint32 num = item.size(); QDataStream str(map); str << num;

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
	QDataStream str(map);
	map->reset();
	quint32 num;
	str >> num;
	clear();
	for (int i = 0; i < num; ++ i) {
	    if (!insertItem(str)) { clear(); return false; }
	}
	if (str.status() ==
	    QDataStream::ReadCorruptData) {
	    clear();
	    return false;
	}
	return true;
}

void StegArch::clear() {
	item.clear();
	vol = 0;
}
