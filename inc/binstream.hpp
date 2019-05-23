#ifndef __INCLUDE_BINSTREAM_H
#define __INCLUDE_BINSTREAM_H

#include <QIODevice>

#include <cstdint>
#include <climits>
#include <string>
#include <vector>

struct BinStream: public QIODevice {
protected:
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 readData(char *data, qint64 maxSize) override;
public:
	BinStream(QIODevice *_dev, QIODevice::OpenMode mode);
	BinStream(QIODevice *_dev);
	~BinStream();
	bool open(QIODevice::OpenMode mode) override;
	void close() override;
	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;
	bool isSequential() const override;
	bool canReadLine() const override;
	bool atEnd() const override;
	qint64 size() const override;
	qint64 pos() const override;
	bool seek(qint64 pos) override;
	bool reset() override;
	virtual bool flush();
private:
	QIODevice *dev;
};

#endif //__INCLUDE_BINSTREAM_H
