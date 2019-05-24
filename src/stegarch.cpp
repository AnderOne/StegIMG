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
	return map->open(QIODevice::ReadWrite);
}

bool StegArch::reset(std::string _key) {
	if (!map) return false;
	key = _key;
	return map->reset(_key);
}

#define BUF_SIZE (quint32(1024))

bool StegArch::encode(QDataStream &inp, CompressModeFlag mode) {

	if (!map) return false;

	map->setNoise();	//Добавляем случайный шум к изображению!

	//Записываем заголовок:
	QDataStream str(map);
	quint32 len = 0;
	str << len;
	//Записываем данные:
	std::unique_ptr<BinStream> bin;
	switch (mode) {
	case None:
		bin.reset(new BinStream(map, QIODevice::WriteOnly));
		break;
	case RLE:
		bin.reset(new RLEStream(map, QIODevice::WriteOnly));
		break;
	case LZW:
		bin.reset(new LZWStream(map, QIODevice::WriteOnly));
		break;
	default:
		return false;
	}
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
	str.setDevice(map);
	//Обновляем заголовок:
	map->reset();
	str << len;
	return true;
}

bool StegArch::decode(QDataStream &out, CompressModeFlag mode) {

	if (!map) return false;

	//Считываем заголовок:
	QDataStream str(map);
	quint32 len = 0;
	str >> len;
	//Считываем данные:
	std::unique_ptr<BinStream> bin;
	switch (mode) {
	case None:
		bin.reset(new BinStream(map, QIODevice::ReadOnly));
		break;
	case RLE:
		bin.reset(new RLEStream(map, QIODevice::ReadOnly));
		break;
	case LZW:
		bin.reset(new LZWStream(map, QIODevice::ReadOnly));
		break;
	default:
		return false;
	}
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
