#include "stdafx.h"

#include "DataClasses.h"
#include "DeadlockLock.h"

using namespace std;
struct Trip;

//locks as soon as possible and invisibly
inline void DeadlockLock::immediateLock(Trip* t2)
{
	while (!m.try_lock())
	{
		if (t2->lock.beingChecked)
		{
			if (ConflictResolutionLock.try_lock())
			{
				t2->lock.demandUnlock = true;
				t2->lock.m.lock();
				t2->lock.demandUnlock = false;
				ConflictResolutionLock.unlock();
			}
			else
			{
				if (demandUnlock)
				{
					m.unlock();
					while (demandUnlock);
					m.lock();
				}
			}
		}
	}
}
