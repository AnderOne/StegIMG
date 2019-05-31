#ifndef __INCLUDE_RLESTREAM_H
#define __INCLUDE_RLESTREAM_H

#include <binstream.hpp>

#include <cstdint>
#include <climits>
#include <string>
#include <vector>

struct RLEStream: public BinStream {
protected:
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 readData(char *data, qint64 maxSize) override;
public:
	RLEStream(QIODevice *_dev, QIODevice::OpenMode mode);
	RLEStream(QIODevice *_dev);
	bool open(QIODevice::OpenMode mode) override;
	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;
	bool atEnd() const override;
	bool flush() override;
private:
	bool get(uchar *c);
	bool put(uchar c);
	bool first;
	int count;
	int last;
};

#endif //__INCLUDE_RLESTREAM_H
