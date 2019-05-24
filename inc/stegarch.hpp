#ifndef __INCLUDE_STEGARCH_H
#define __INCLUDE_STEGARCH_H

#include <binstream.hpp>
#include <rlestream.hpp>
#include <lzwstream.hpp>
#include <stegmap.hpp>
#include <QDataStream>
#include <QImage>

class StegArch {

public:
	enum CompressModeFlag { None = 0, RLE = 1, LZW = 2 };

	virtual ~StegArch();
	explicit StegArch(const QImage &_img, std::string _key = "");
	StegArch();

	bool encode(QDataStream &_inp, CompressModeFlag _mod = None);
	bool decode(QDataStream &_out, CompressModeFlag _mod = None);

	bool reset(const QImage &_img, std::string _key);
	bool reset(const QImage &_img);
	bool reset(std::string _key);

	inline const StegMap *steg() const { return map; }
	inline const QImage &image() const { return img; }

private:
	std::string key;
	StegMap *map = nullptr;
	QImage img;
};

#endif //__INCLUDE_STEGARCH_H
