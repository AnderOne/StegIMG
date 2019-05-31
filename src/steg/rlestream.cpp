#include <rlestream.hpp>
#include <algorithm>
#include <climits>
#include <cstdlib>

RLEStream::RLEStream(QIODevice *_dev, QIODevice::OpenMode mode): BinStream(_dev) { open(mode); }

RLEStream::RLEStream(QIODevice *_dev): BinStream(_dev) {}

bool RLEStream::open(QIODevice::OpenMode mode) {

	if (!BinStream::open(mode)) return false;
	first = true;
	count = 0;
	last = -1;
	return true;
}

qint64 RLEStream::bytesAvailable() const { return count + BinStream::bytesAvailable(); }

qint64 RLEStream::bytesToWrite() const { return count + BinStream::bytesToWrite(); }

qint64 RLEStream::writeData(const char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (!put(data[i])) return -1;
	}
	return maxSize;
}

qint64 RLEStream::readData(char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (this->atEnd()) return i;
		if (!get((uchar *) &data[i])) {
			return -1;
		}
	}
	return maxSize;
}

bool RLEStream::put(uchar c) {

	if (last == -1) { count = 1; last = c; return true; }

	if (last != c || (count == UCHAR_MAX + 2)) {
		if (!flush()) return false;
		count = 0; last = c;
	}
	++ count;
	return true;
}

bool RLEStream::get(uchar *ch) {

	if (count) { *ch = last; -- count; return true; }

	if (!dev->getChar((char *) ch)) return false;

	if (last != *ch) {
		last = *ch; first = true; return true;
	}
	if (!first) { first = true; return true; }
	static uchar c;
	if (!dev->getChar((char*) &c)) {
		return false;
	}
	first = false;
	count = c;

	return true;
}

bool RLEStream::flush() {

	if (!count) return BinStream::flush();
	if (count > 1) {
		if (!dev->putChar(last) ||
		    !dev->putChar(last) ||
		    !dev->putChar(count - 2)) return false;
	}
	else {
		if (!dev->putChar(last)) return false;
	}
	count = 0;
	return BinStream::flush();
}

bool RLEStream::atEnd() const {
	return
	BinStream::atEnd() &&
	!count;
}
