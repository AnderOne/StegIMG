#ifndef __INCLUDE_LZWSTREAM_H
#define __INCLUDE_LZWSTREAM_H

#include <binstream.hpp>

#include <cstdint>
#include <climits>
#include <string>
#include <vector>

#define DICT_SIZE (10000)
#define ALPH_SIZE (256)
#define MASK_BITS (64)

struct LZWStream: public BinStream {

protected:
	qint64 writeData(const char *data, qint64 maxSize) override;
	qint64 readData(char *data, qint64 maxSize) override;

public:
	LZWStream(QIODevice *_dev, QIODevice::OpenMode mode);
	LZWStream(QIODevice *_dev);
	bool open(QIODevice::OpenMode mode) override;
	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;
	bool atEnd() const override;
	bool flush() override;

private:

	struct EncoderNode {

		inline void set(int c, EncoderNode *p) {
			CHILD[c] = p; mask[c / MASK_BITS] |= 1ULL << (c % MASK_BITS);
		}
		inline EncoderNode *get(int c) {
			if ((mask[c / MASK_BITS] >> (c % MASK_BITS)) & 1ULL) {
				return CHILD[c];
			}
			return nullptr;
		}
		inline void del() {
			for (int i = 0; i < 4; ++ i) mask[i] = 0;
		}
		EncoderNode *CHILD[ALPH_SIZE];
		uint64_t mask[4];
	};

	struct DecoderNode {
		uchar first, sym;
		long last;
	};

	bool getBits(uint *c);
	bool putBits(uint c);
	bool get(uchar *c);
	bool put(uchar c);

	std::vector<EncoderNode> EncoderDict;
	std::vector<DecoderNode> DecoderDict;
	std::vector<uchar> DecoderBuff;
	uint64_t count;
	EncoderNode *top = nullptr;
	EncoderNode *cur = nullptr;
	bool first = false;
	uint64_t max;
	uint ind;
	int dim;
	uchar buf;
	int bit;
};

#endif //__INCLUDE_LZWSTREAM_H
