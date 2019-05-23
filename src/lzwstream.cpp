#include "lzwstream.hpp"
#include <algorithm>
#include <climits>
#include <cstdlib>

LZWStream::LZWStream(QIODevice *_dev, QIODevice::OpenMode mode): BinStream(_dev) { open(mode); }

LZWStream::LZWStream(QIODevice *_dev): BinStream(_dev) {}

bool LZWStream::open(QIODevice::OpenMode mode) {

	if (!BinStream::open(mode)) return false;

	if (mode & QIODevice::WriteOnly) {
		EncoderDict.resize(DICT_SIZE);
		for (int i = 0; i < ALPH_SIZE; ++ i) EncoderDict[i].del();
		top = &EncoderDict[ALPH_SIZE];
		cur = nullptr;
	}
	if (mode & QIODevice::ReadOnly) {
		DecoderBuff.resize(DICT_SIZE);
		DecoderDict.resize(DICT_SIZE);
		for (int i = 0; i < ALPH_SIZE; ++ i) {
			DecoderDict[i].first = i;
			DecoderDict[i].last = -1;
			DecoderDict[i].sym = i;
		}
	}
	first = true;
	count = 0;
	max = ALPH_SIZE - 1;
	dim = CHAR_BIT;
	buf = 0;
	bit = 0;
	return true;
}

qint64 LZWStream::bytesAvailable() const { return count + BinStream::bytesAvailable(); }

qint64 LZWStream::bytesToWrite() const { return count + BinStream::bytesToWrite(); }

qint64 LZWStream::writeData(const char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (!put(data[i])) return -1;
	}
	return maxSize;
}

qint64 LZWStream::readData(char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (this->atEnd()) return i;
		if (!get((uchar *) &data[i])) {
			return -1;
		}
	}
	return maxSize;
}

bool LZWStream::putBits(uint c) {

	for (int i = 0; i < dim; ++ i) {
		if (bit == CHAR_BIT) {
			if (!dev->putChar(buf)) return false;
			bit = 0; buf = 0;
		}
		if (1 & c) {
			buf |= 1 << bit;
		}
		c >>= 1;
		++ bit;
	}
	return true;
}

bool LZWStream::getBits(uint *c) {
	*c = 0;
	for (int i = 0; i < dim; ++ i) {
		if (!bit && !dev->getChar((char *) &buf))
			return false;
		if (1 & buf) *c |= 1 << i;
		if ((++ bit) == CHAR_BIT)
			bit = 0;
		buf >>= 1;
	}
	return true;
}

bool LZWStream::put(uchar c) {

	if (!cur) { cur = EncoderDict.data() + c; return true; }

	EncoderNode *nxt = cur->get(c);
	if (nxt) { cur = nxt; return true; }

	//Выводим битовый код строки:
	if (!putBits(cur - EncoderDict.data())) {
		return false;
	}
	//Меняем разрядность кода:
	if ((++ max) == (1U << dim)) dim += 1;
	//Проверяем переполнение:
	if (max == DICT_SIZE) {
		for (int i = 0; i < ALPH_SIZE; ++ i) {
			EncoderDict[i].del();
		}
		top = &EncoderDict[ALPH_SIZE];
		cur = &EncoderDict[c];
		first = true;
		max = ALPH_SIZE - 1;
		dim = CHAR_BIT;
		return true;
	}
	//Добавляем строку:
	cur->set(c, top);
	cur = EncoderDict.data() + c;
	top->del();
	top ++;
	return true;
}

bool LZWStream::get(uchar *ch) {

	if (count) { *ch = DecoderBuff[-- count]; return true; }

	if (first) {
		if (!getBits(&ind)) return false;
		first = false;
		*ch = ind;
		return true;
	}
	uint lst = ind;
	//Меняем разрядность кода:
	if ((++ max) == (1U << dim)) dim += 1;
	//Проверяем переполнение:
	if (max == DICT_SIZE) {
		max = ALPH_SIZE - 1;
		dim = CHAR_BIT;
		if (!getBits(&ind)) return false;
		*ch = ind;
		return true;
	}
	//Читаем новый код:
	if (!getBits(&ind) || (ind > max)) {
		return false;
	}
	//Добавляем строку:
	uint nxt = (ind == max)? lst: ind;
	DecoderDict[max].first =
	       DecoderDict[lst].first;
	DecoderDict[max].last =
	       lst;
	DecoderDict[max].sym =
	       DecoderDict[nxt].first;
	//Заполняем буфер:
	for (long i = ind; i != -1; i = DecoderDict[i].last) {
		DecoderBuff[count ++] =
		DecoderDict[i].sym;
	}
	*ch = DecoderBuff[-- count];
	return true;
}

bool LZWStream::flush() {

	if (cur != nullptr) {
		if (!putBits(cur - EncoderDict.data())) {
			return false;
		}
		cur = nullptr;
	}
	if (bit) {
		if (!dev->putChar(buf)) {
			return false;
		}
		bit = 0;
	}
	return
	BinStream::flush();
}

bool LZWStream::atEnd() const {
	return
	BinStream::atEnd() &&
	!count;
}
