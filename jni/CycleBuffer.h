/*
 * CycleBuffer.h
 *
 *  Created on: 2015年4月7日
 *      Author: zhao
 */

#ifndef CYCLEBUFFER_H_
#define CYCLEBUFFER_H_
class CycleBuffer {
public:

	CycleBuffer(int size = 1024 * 1024);
	pthread_cond_t cond_read;
	pthread_cond_t cond_write;
	 pthread_mutex_t mutex;
	virtual ~CycleBuffer();
	int write(char* buf, int count);
	int read(char* buf, int count);
	int avaibleToRead();
	int avaibleToWrite();
	void setEmpty();

private:
	//空标示符
	bool isEmpty;
	//满标识符
	bool isFull;
	//缓冲区指针
	char * Buf;
	//缓冲区大小
	int BufSize;
	// 已写数据大小
	int inCount;
	//始位置
	int ReadPos;
	//可写位置
	int WritePos;

};

#endif /* CYCLEBUFFER_H_ */
