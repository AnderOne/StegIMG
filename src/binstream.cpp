#include "binstream.hpp"
#include <algorithm>
#include <climits>
#include <cstdlib>

BinStream::BinStream(QIODevice *_dev, QIODevice::OpenMode mode): QIODevice(), dev(_dev) { open(mode); }

BinStream::BinStream(QIODevice *_dev): QIODevice(), dev(_dev) {}

BinStream::~BinStream() { close(); }

bool BinStream::open(QIODevice::OpenMode mode) {

	if (((mode & QIODevice::WriteOnly) && (mode & QIODevice::ReadOnly)) ||
	     (mode & QIODevice::Append)) {
		return false;
	}
	return QIODevice::open(mode | QIODevice::Unbuffered);
}

void BinStream::close() {

	if (openMode() & QIODevice::WriteOnly) {
		flush();
	}
	QIODevice::close();
}

qint64 BinStream::bytesAvailable() const { return QIODevice::bytesAvailable(); }

qint64 BinStream::bytesToWrite() const { return QIODevice::bytesToWrite(); }

bool BinStream::isSequential() const { return true; }

bool BinStream::canReadLine() const { return false; }

qint64 BinStream::writeData(const char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (!dev->putChar(data[i])) return -1;
	}
	return maxSize;
}

qint64 BinStream::readData(char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) {
		if (this->atEnd()) return i;
		if (!dev->getChar(&data[i])) {
			return -1;
		}
	}
	return maxSize;
}

bool BinStream::flush() { return true; }

qint64 BinStream::size() const { return bytesAvailable(); }

qint64 BinStream::pos() const { return 0; }

bool BinStream::atEnd() const {
	return dev->atEnd();
}

bool BinStream::seek(qint64 pos) {
	return false;
}

bool BinStream::reset() {
	return false;
}
