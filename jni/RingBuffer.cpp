/*
 * RingBuffer.cpp
 *
 *  Created on: 2015年4月10日
 *      Author: zhao
 */
#define LOG_TAG "OpenSLActivity"
#define DGB 1
#include"RingBuffer.h"
#include "log.h"

	RingBuffer::RingBuffer(size_t size){
		ALOGD("pzhao-->create RingBuffer");
					mLength = size;
					mRingBufferPtr = ringbuffer_create(mLength);
				}
	RingBuffer::~RingBuffer(){
	ALOGD("pzhao-->delete RingBuffer");
					if(mRingBufferPtr != NULL)
						ringbuffer_free(mRingBufferPtr);
				}

	size_t RingBuffer::length(){
					return mLength;
				}

	size_t RingBuffer::getReadSpace(){
					return ringbuffer_read_space(mRingBufferPtr);
				}
	size_t RingBuffer::getWriteSpace(){
					return ringbuffer_write_space(mRingBufferPtr);
				}

	size_t RingBuffer::Read( char *dest, size_t cnt){
	//	ALOGD("pzhao-->read RingBuffer");
		return ringbuffer_read (mRingBufferPtr , dest, cnt);
	}

	size_t RingBuffer::Write(char *src,size_t cnt){
	//	ALOGD("pzhao-->write RingBuffer");
		return ringbuffer_write (mRingBufferPtr, src, cnt);
	}

	size_t RingBuffer::get_readPtr(){
		return get_read_ptr(mRingBufferPtr);
	}
	size_t RingBuffer::get_writePtr(){
			return get_write_ptr(mRingBufferPtr);
		}
	void RingBuffer::Reset(){
		ringbuffer_reset (mRingBufferPtr);
	}


