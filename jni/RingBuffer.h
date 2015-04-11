/*
 * RingBuffer.h
 *
 *  Created on: 2015年4月10日
 *      Author: zhao
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *buf;
    size_t len;
}
ringbuffer_data_t ;

typedef struct {
    char	*buf;
    volatile size_t write_ptr;
    volatile size_t read_ptr;
    size_t	size;
    size_t	size_mask;
}ringbuffer_t ;

class RingBuffer{
private:
	ringbuffer_t *mRingBufferPtr;
	size_t mLength;
	ringbuffer_t *ringbuffer_create(size_t sz) {
		size_t power_of_two;
		ringbuffer_t *rb;

		if ((rb = (ringbuffer_t *) malloc(sizeof(ringbuffer_t))) == NULL) {
			return NULL;
		}

		for (power_of_two = 1; 1 << power_of_two < sz; power_of_two++)
			;

		rb->size = 1 << power_of_two;
		rb->size_mask = rb->size;
		rb->size_mask -= 1;
		rb->write_ptr = 0;
		rb->read_ptr = 0;
		if ((rb->buf = (char *) malloc(rb->size)) == NULL) {
			free(rb);
			return NULL;
		}

		return rb;
	}

	void ringbuffer_free(ringbuffer_t * rb) {
		free(rb->buf);
		free(rb);
	}

	void ringbuffer_reset(ringbuffer_t * rb) {
		rb->read_ptr = 0;
		rb->write_ptr = 0;
		memset(rb->buf, 0, rb->size);
	}

	size_t ringbuffer_read_space(const ringbuffer_t * rb) {
		size_t w, r;

		w = rb->write_ptr;
		r = rb->read_ptr;

		if (w > r) {
			return w - r;
		} else {
			return (w - r + rb->size) & rb->size_mask;
		}
	}

	size_t ringbuffer_write_space(const ringbuffer_t * rb) {
		size_t w, r;

		w = rb->write_ptr;
		r = rb->read_ptr;

		if (w > r) {
			return ((r - w + rb->size) & rb->size_mask) - 1;
		} else if (w < r) {
			return (r - w) - 1;
		} else {
			return rb->size - 1;
		}
	}

	size_t ringbuffer_read(ringbuffer_t * rb, char *dest, size_t cnt) {
		size_t free_cnt;
		size_t cnt2;
		size_t to_read;
		size_t n1, n2;

		if ((free_cnt = ringbuffer_read_space(rb)) == 0) {
			return 0;
		}

		to_read = cnt > free_cnt ? free_cnt : cnt;

		cnt2 = rb->read_ptr + to_read;

		if (cnt2 > rb->size) {
			n1 = rb->size - rb->read_ptr;
			n2 = cnt2 & rb->size_mask;
		} else {
			n1 = to_read;
			n2 = 0;
		}

		memcpy(dest, &(rb->buf[rb->read_ptr]), n1);
		rb->read_ptr = (rb->read_ptr + n1) & rb->size_mask;

		if (n2) {
			memcpy(dest + n1, &(rb->buf[rb->read_ptr]), n2);
			rb->read_ptr = (rb->read_ptr + n2) & rb->size_mask;
		}

		return to_read;
	}
	size_t ringbuffer_write(ringbuffer_t * rb, const char *src, size_t cnt) {
		size_t free_cnt;
		size_t cnt2;
		size_t to_write;
		size_t n1, n2;

		if ((free_cnt = ringbuffer_write_space(rb)) == 0) {
			return 0;
		}

		to_write = cnt > free_cnt ? free_cnt : cnt;

		cnt2 = rb->write_ptr + to_write;

		if (cnt2 > rb->size) {
			n1 = rb->size - rb->write_ptr;
			n2 = cnt2 & rb->size_mask;
		} else {
			n1 = to_write;
			n2 = 0;
		}

		memcpy(&(rb->buf[rb->write_ptr]), src, n1);
		rb->write_ptr = (rb->write_ptr + n1) & rb->size_mask;

		if (n2) {
			memcpy(&(rb->buf[rb->write_ptr]), src + n1, n2);
			rb->write_ptr = (rb->write_ptr + n2) & rb->size_mask;
		}

		return to_write;
	}

	size_t get_read_ptr(ringbuffer_t * rb){
		return rb->read_ptr;
	}
	size_t get_write_ptr(ringbuffer_t * rb){
			return rb->write_ptr;
		}

public:
	RingBuffer(size_t size);
	~RingBuffer();
	size_t length();
	size_t getReadSpace();
	size_t getWriteSpace();
	size_t get_readPtr();
	size_t get_writePtr();
	size_t Read( char *dest, size_t cnt);
	size_t Write(char *src,size_t cnt);
	void Reset();
};

#ifdef __cplusplus
}
#endif


#endif /* RINGBUFFER_H_ */
