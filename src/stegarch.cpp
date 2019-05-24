#include "stegarch.hpp"

#define BUF_SIZE (quint32(1024))

bool StegArch::encode(QDataStream &inp, std::string key) {

	if (!map) return false;
	map->setNoise();	//Добавляем случайный шум к изображению!
	map->reset(key);
	//Записываем заголовок:
	QDataStream str(map);
	quint32 len = 0;
	str << len;
	//Записываем данные:
	BinStream bin(map, QIODevice::WriteOnly);
	str.setDevice(&bin);
	char buf[BUF_SIZE];
	while (!inp.atEnd()) {
		int n = inp.readRawData(buf, BUF_SIZE);
		str.writeRawData(buf, n);
		if (str.status() != QDataStream::Ok) {
			return false;
		}
		len += n;
	}
	if (!bin.flush()) {
		return false;
	}
	str.setDevice(map);
	//Обновляем заголовок:
	map->reset();
	str << len;
	return true;
}

bool StegArch::decode(QDataStream &out, std::string key) {

	if (!map) return false;
	map->reset(key);
	//Считываем заголовок:
	QDataStream str(map);
	quint32 len = 0;
	str >> len;
	//Считываем данные:
	BinStream bin(map, QIODevice::ReadOnly);
	str.setDevice(&bin);
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

bool StegArch::reset(const QImage &_img) {
	//Переводим в полноцветное 32-битное изображение:
	img = _img.convertToFormat(QImage::Format::Format_ARGB32);
	if (map) delete map;
	map = new StegMap(
		(QRgb *) img.bits(), img.byteCount() / 4
	);
	return map->open(QIODevice::ReadWrite);
}

StegArch::StegArch(const QImage &_img) {
	reset(_img);
}

StegArch::StegArch() {}
