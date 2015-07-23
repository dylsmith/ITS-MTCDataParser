#ifndef DEADLOCKLOCK_H
#define DEADLOCKLOCK_H

#include <thread>
#include <mutex>
#include <list>
#include "DataClasses.h"

using namespace std;

class DeadlockLock
{
	mutex m;
	mutex  modifyingLock;
public:
	bool demandUnlock = false;
	bool beingChecked = false;
	list<int> waitingList;

	//put TID in queue, wait until TID is head of queue then lock
	inline void waitLock(int tid)
	{
		modifyingLock.lock();
		waitingList.push_back(tid);
		modifyingLock.unlock();
		while (waitingList.front() != tid);
		m.lock();
	}

	//remove self from queue, unlock
	inline void unlock(int tid)
	{
		modifyingLock.lock();		
		waitingList.remove(tid);
		modifyingLock.unlock();
		m.unlock();
	}

	//locks as soon as possible and invisibly
	void immediateLock(Trip* t2);
};


#endif