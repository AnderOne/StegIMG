#include "stegmap.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

StegMap::StegMap(uint32_t *map, size_t num, std::string key):
         QIODevice(), buffMap(map), buffInd(nullptr), sizeMap(num), sizeInd(num * 2), sizeDat(num / 2), cur(0) {
	buffInd = new qint32[sizeInd];
	StegMap::reset(key);
}

StegMap::~StegMap() { delete[] buffInd; }

qint64 StegMap::writeData(const char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) { if (StegMap::atEnd()) return i; this->put(data[i]); }
	return maxSize;
}

qint64 StegMap::readData(char *data, qint64 maxSize) {

	for (int i = 0; i < maxSize; ++ i) { if (StegMap::atEnd()) return i; data[i] = get(); }
	return maxSize;
}

int StegMap::put(uint8_t c) {

	if (StegMap::atEnd()) return EOF;
	for (int i = 6; i >= 0; i -= 2) {
		int p = buffInd[cur ++];
		int s = 8 * (p % 4);
		p /= 4;
		buffMap[p] = (buffMap[p] & (~ (0x03 << s))) | (((c >> i) & 0x03) << s);
	}
	return c;
}

int StegMap::get() {

	if (StegMap::atEnd()) return EOF;
	int c = 0;
	for (int i = 0; i < 4; ++ i) {
		int p = buffInd[cur ++];
		int s = 8 * (p % 4);
		p /= 4;
		c = (c << 2) | ((buffMap[p] >> s) & 0x03);
	}
	return c;
}

bool StegMap::reset(std::string key) {

	static const int MAGIC = ((1 << 17) - 1);	//NOTE: Magic prime number!

	//We use only R and B components:
	for (int i = 0; i < sizeMap; ++ i) buffInd[2 * i + 1] = (buffInd[2 * i] = 4 * i) + 2;
	if (isOpen() && !StegMap::reset()) return false;

	if (!key.size()) return true;
	int h = 0;
	for (int i = 0; i < key.size(); ++ i) h = std::abs(h + (uint8_t) key[i]) % MAGIC;
	for (int i = 0; i < sizeInd; ++ i) {
		h = std::abs(h * (uint8_t) key[i % key.size()] + MAGIC) % sizeInd;
		std::swap(buffInd[h], buffInd[i]);
	}
	return true;
}

void StegMap::setNoise() {
	srand(time(nullptr));
	for (int i = 1; i < sizeMap; i += 2) buffMap[i] ^= rand() % 4;
}

bool StegMap::open(QIODevice::OpenMode mode) {
	return QIODevice::open(mode) && StegMap::reset();
}

void StegMap::close() {
	cur = 0;
	QIODevice::close();
}

bool StegMap::waitForBytesWritten(int msecs) {
	return QIODevice::waitForBytesWritten(msecs);
}

bool StegMap::waitForReadyRead(int msecs) {
	return QIODevice::waitForReadyRead(msecs);
}

qint64 StegMap::bytesAvailable() const {
	return QIODevice::bytesAvailable();
}

qint64 StegMap::bytesToWrite() const {
	return QIODevice::bytesToWrite();
}

bool StegMap::isSequential() const {
	return false;
}

bool StegMap::canReadLine() const {
	return !atEnd();
}

bool StegMap::atEnd() const { return (cur > sizeInd - 4); }

qint64 StegMap::size() const { return sizeDat; }

qint64 StegMap::pos() const { return cur / 4; }

bool StegMap::seek(qint64 pos) {
	QIODevice::seek(pos);
	if (pos > sizeInd - 4) return false;
	cur = pos * 4;
	return true;
}

bool StegMap::reset() {
	QIODevice::reset();
	cur = 0;
	return true;
}
