#include "stegarch.hpp"

#define BUFF_SIZE (1024)

char buff[BUFF_SIZE];

bool StegArch::encode(QDataStream &inp, std::string key) {

	if (!map) return false;
	map->setNoise();	//Добавляем случайный шум к изображению!
	map->reset(key);
	//Записываем заголовок:
	QDataStream str(map);
	uint32_t len = 0;
	str << len;
	//Записываем данные:
	BinStream bin(map, QIODevice::WriteOnly);
	str.setDevice(&bin);
	while (!inp.atEnd()) {
		size_t n = inp.readRawData(buff, BUFF_SIZE);
		str.writeRawData(buff, n);
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
	uint32_t len = 0;
	str >> len;
	//Считываем данные:
	BinStream bin(map, QIODevice::ReadOnly);
	str.setDevice(&bin);
	while (true) {
		size_t m = std::min(len, (uint32_t) BUFF_SIZE);
		size_t n = str.readRawData(buff, m);
		if (!n) break;
		out.writeRawData(buff, n);
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
