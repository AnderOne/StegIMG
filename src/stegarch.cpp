#include "stegarch.hpp"
#include <memory>

StegArch::~StegArch() { delete map; }

StegArch::StegArch(const QImage &_img, std::string _key) { reset(_img, _key); }

StegArch::StegArch() {}

bool StegArch::reset(const QImage &_img, std::string _key) {
	key = _key; return reset(_img);
}

bool StegArch::reset(const QImage &_img) {
	//Переводим в полноцветное 32-битное изображение:
	img = _img.convertToFormat(QImage::Format::Format_ARGB32);
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
	vol = 0;
	return map->reset(_key);
}

#define BUF_SIZE (quint32(1024))

BinStream *StegArch::gener(CompressModeFlag _mod, OpenModeFlag _flg) {

	switch (_mod) {
	case None:
		return new BinStream(map, _flg);
	case RLE:
		return new RLEStream(map, _flg);
	case LZW:
		return new LZWStream(map, _flg);
	}

	return nullptr;
}

bool StegArch::encode(QDataStream &inp, CompressModeFlag mod) {

	if (!map) return false;

	map->setNoise();	//Добавляем случайный шум к изображению!

	//Записываем заголовок:
	QDataStream str(map); str << (quint8) mod;
	quint32 len = 0, com = 0;
	qint64 pos = map->pos();
	str << len; if (mod != None) str << com;
	qint64 top = map->pos();

	//Записываем данные:
	std::unique_ptr<BinStream> bin;
	bin.reset(gener(mod, QIODevice::WriteOnly));
	if (!bin) return false;
	str.setDevice(bin.get());
	char buf[BUF_SIZE];
	while (!inp.atEnd()) {
		int n = inp.readRawData(buf, BUF_SIZE);
		str.writeRawData(buf, n);
		if (str.status() != QDataStream::Ok) {
			return false;
		}
		len += n;
	}
	if (!bin->flush()) {
		return false;
	}
	vol = map->pos();

	//Обновляем заголовок:
	com = map->pos() - top;
	map->seek(pos);
	str.setDevice(map);
	str << len;
	if (mod != None) {
		str << com;
	}
	return true;
}

bool StegArch::decode(QDataStream &out) {

	if (!map) return false;

	//Считываем заголовок:
	QDataStream str(map); quint8 tmp; str >> tmp;
	CompressModeFlag mod = CompressModeFlag(tmp);
	quint32 len = 0, com = 0;
	str >> len; if (mod != None) str >> com;
	
	//Считываем данные:
	std::unique_ptr<BinStream> bin;
	bin.reset(gener(mod, QIODevice::ReadOnly));
	if (!bin) return false;
	str.setDevice(bin.get());
	char buf[BUF_SIZE];
	while (true) {
		size_t m = std::min(len, BUF_SIZE);
		int n = str.readRawData(buf, m);
		if (n <= 0) break;
		out.writeRawData(buf, n);
		len -= n;
	}
	return !len;
}
