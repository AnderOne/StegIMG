#ifndef __INCLUDE_STEGMAP_H
#define __INCLUDE_STEGMAP_H

#include <QIODevice>

#include <cstdint>
#include <string>

struct StegMap: public QIODevice {
protected:
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 readLineData(char *data, qint64 maxSize) override;
	qint64 readData(char *data, qint64 maxSize) override;
public:
	virtual bool reset(std::string key);
	virtual void setNoise();
	//...
	StegMap(uint32_t *map, size_t num, std::string key = "");
	~StegMap();
	bool open(QIODevice::OpenMode mode) override;
	void close() override;
	bool waitForBytesWritten(int msecs) override;
	bool waitForReadyRead(int msecs) override;
	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;
	bool isSequential() const override;
	bool canReadLine() const override;
	bool atEnd() const override;
	qint64 size() const override;
	qint64 pos() const override;
	bool seek(qint64 pos) override;
	bool reset() override;
	//...
private:
	int put(uint8_t c);
	int get();
	uint32_t *buffMap;
	qint32 *buffInd;
	qint32 sizeMap;
	qint32 sizeInd;
	qint32 sizeDat;
	qint64 cur;
};

#endif //__INCLUDE_STEGMAP_H
