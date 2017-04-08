#ifndef FRAMETRACKER_H
#define FRAMETRACKER_H

#include "Headers.h"

/*
The FrameTracker class keeps track of how many frames have been rendered since
startReset() was called. After one second has passed the frame count can be used
as the frames per second.
*/
class FrameTracker {
private:
	int frameCount;
	LARGE_INTEGER startTime;
	LARGE_INTEGER frequency;

public:
	int initTracker();
	void startReset();
	void incCount();
	bool secondPassed();
	int getFPS();
	LARGE_INTEGER getFrequency();
};

#endif // !FRAMETRACKER_H
