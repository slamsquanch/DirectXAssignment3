#include "Headers.h"

/*
Initializes the FrameTracker's frequency to be the systems timing frequency.

@return - Returns an int to be used as an HRESULT in the FAILED() macro.
		  Fails if the System does not support high frequency timing.
*/
int FrameTracker::initTracker() {
	QueryPerformanceFrequency(&frequency);

	if (frequency.QuadPart == 0) {
		SetError(TEXT("System does not support high frequency timing"));
		return E_FAIL;
	}

	return S_OK;
}

/*
Starts or resets the FrameTracker. To be used after a second has passed.
*/
void FrameTracker::startReset() {
	QueryPerformanceCounter(&startTime);
	frameCount = 0;
}

/*
Increments the amount of frames rendered since the FrameTracker was started/reset.
*/
void FrameTracker::incCount() {
	frameCount++;
}

/*
Checks whether or not a second has passed since the FrameTracker was started/reset.

@return - Returns true if a second has passed.
*/
bool FrameTracker::secondPassed() {
	LARGE_INTEGER curTime;
	QueryPerformanceCounter(&curTime);

	return (curTime.QuadPart - startTime.QuadPart >= frequency.QuadPart);
}

/*
Gets the amount of frames rendered since the FrameTracker was started/reset.

@return  - The frames rendered in the last second.
*/
int FrameTracker::getFPS() {
	return frameCount;
}

LARGE_INTEGER FrameTracker::getFrequency() {
	return frequency;
}
