#ifndef BUFFER_STREAM_H
#define BUFFER_STREAM_H

#include <memory>
#include <Stream.h>

#include "types.h"

class BufferStream : public Stream {
	public:
		BufferStream(uint32_t bufferLength);
		int available();
		int read();
		int peek();
		size_t write(uint8_t b);
		virtual bool isWritable() {
			return false;
		}

		void rewind() {
			seekTo(0);
		}

		void seekTo(uint32_t position) {
			bufferPosition = position;
		}

		inline uint8_t * getBuffer() {
			return buffer.get();
		}
		inline uint32_t size() {
			return bufferLength;
		}
		bool writeBuffer(uint8_t * data, uint32_t length, uint32_t offset);
		void writeBufferByte(uint8_t data, uint32_t offset);
		bool incrementBufferByte(uint32_t offset, int8_t by);
	protected:
		std::unique_ptr<uint8_t[]> buffer;
		uint32_t bufferLength;
		uint32_t bufferPosition;
};

class WritableBufferStream : public BufferStream {
	public:
		WritableBufferStream(uint32_t bufferLength) : BufferStream(bufferLength), bufferWritePosition(0) {};
		size_t write(uint8_t b);
		bool isWritable() override {
			return true;
		};

		void rewindWrite() {
			bufferWritePosition = 0;
		}

	private:
		uint32_t bufferWritePosition;

};


#endif // BUFFER_STREAM_H
