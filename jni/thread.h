/*
 * thread.h
 *
 *  Created on: 2015年4月7日
 *      Author: zhao
 */

#ifndef THREAD_H_
#define THREAD_H_
#include <pthread.h>
class Thread {
public:
	Thread();
	virtual ~Thread();
	int start();
	int join();
	int detach();
	pthread_t self();
	virtual void* run() = 0;
private:
	pthread_t m_tid;
	int m_running;
	int m_detached;
};

#endif /* THREAD_H_ */
