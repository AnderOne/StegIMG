#ifndef __INCLUDE_STEGARCH_H
#define __INCLUDE_STEGARCH_H

#include <stegmap.hpp>
#include <QDataStream>
#include <QImage>

class StegArch {
public:
	bool encode(QDataStream &inp, std::string key = "");
	bool decode(QDataStream &out, std::string key = "");
	bool reset(const QImage &_img);
	inline const QImage &image() const { return img; }
	StegArch(const QImage &_img);
	StegArch();
private:
	QDataStream str;
	StegMap *map = nullptr;
	QImage img;
};

#endif //__INCLUDE_STEGARCH_H
