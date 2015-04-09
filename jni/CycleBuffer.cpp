/*
 * CycleBuffer.cpp
 *
 *  Created on: 2015年4月7日
 *      Author: zhao
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include<sys/time.h>
#include<sched.h>
#include "log.h"
#include "thread.h"
#include"CycleBuffer.h"
CycleBuffer::CycleBuffer(int size) {

	BufSize = size;
	Buf = new char[BufSize];
	ReadPos = 0;
	WritePos = 0;
	inCount = 0;
	isEmpty = true;
	isFull = false;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_read, NULL);
	pthread_cond_init(&cond_write, NULL);

}
CycleBuffer::~CycleBuffer() {
	delete[] Buf;
}
int CycleBuffer::write(char* buf, int count) {
	pthread_mutex_lock(&mutex);
	if (avaibleToWrite() < count) {
		ALOGD("can't write data");
		pthread_cond_signal(&cond_read);
		pthread_cond_wait(&cond_write, &mutex);
	}

	if (WritePos >= ReadPos) {
		int leftCount = BufSize - WritePos;
		if (leftCount >= count) {
			memcpy(Buf + WritePos, buf, count);
			WritePos += count;
		} else {
			memcpy(Buf + WritePos, buf, leftCount);
			memcpy(Buf, buf + leftCount, count - leftCount);
			WritePos = count - leftCount;
		}
	} else {
		memcpy(Buf + WritePos, buf, count);
		WritePos += count;
	}
	pthread_mutex_unlock(&mutex);
	return count;
}

int CycleBuffer::read(char *buf, int count) {
	pthread_mutex_lock(&mutex);
	int result;
	if (avaibleToRead() <= 0) {
		ALOGD("can't read data");
		pthread_cond_signal(&cond_write);
		pthread_cond_wait(&cond_read, &mutex);
	}
	if (avaibleToRead() >= count) {
		if (WritePos > ReadPos) {
			memcpy(buf, Buf + ReadPos, count);
			ReadPos += count;
		} else {
			int leftCount = BufSize - ReadPos;
			if (leftCount >= count) {
				memcpy(buf, Buf + ReadPos, count);
				ReadPos += count;
			} else {
				memcpy(buf, Buf + ReadPos, leftCount);
				memcpy(buf + leftCount, Buf, BufSize - leftCount);
				ReadPos = count - leftCount;
			}
		}
		result=count;
	}
	else{
		if(WritePos>ReadPos){
			int read=WritePos-ReadPos;
			memcpy(buf,Buf+ReadPos,read);
			memset(buf+read,0,count-read);
			ReadPos=WritePos;
			result=read;
		}
		else{
			int left=BufSize-ReadPos;
			int read=left+WritePos;
			memcpy(buf,Buf+ReadPos,left);
			memcpy(buf+left,Buf,WritePos);
			memset(buf+read,0,count-read);
			ReadPos=WritePos;
			result=read;
		}
	}
	pthread_mutex_unlock(&mutex);
	return result;
}

void CycleBuffer::setEmpty() {
	pthread_mutex_lock(&mutex);
	ALOGD("clear buffer");
	memset(Buf,0,BufSize);
	ReadPos = WritePos = 0;
	pthread_mutex_unlock(&mutex);
}

int CycleBuffer::avaibleToWrite() {
	if (WritePos >= ReadPos)
		return BufSize - (WritePos - ReadPos);
	else
		return ReadPos - WritePos;
}
int CycleBuffer::avaibleToRead() {
	if (WritePos >= ReadPos)
		return WritePos - ReadPos;
	else
		return WritePos + (BufSize - WritePos);
}
