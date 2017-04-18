#ifndef UTIL_H
#define UTIL_H

#include "Headers.h"

void SetError(TCHAR*, ...);


//
// Colors
//
const D3DXCOLOR      WHITE(D3DCOLOR_XRGB(255, 255, 255));
const D3DXCOLOR      BLACK(D3DCOLOR_XRGB(0, 0, 0));
const D3DXCOLOR        RED(D3DCOLOR_XRGB(255, 0, 0));
const D3DXCOLOR      GREEN(D3DCOLOR_XRGB(0, 255, 0));
const D3DXCOLOR       BLUE(D3DCOLOR_XRGB(0, 0, 255));
const D3DXCOLOR     YELLOW(D3DCOLOR_XRGB(255, 255, 0));
const D3DXCOLOR       CYAN(D3DCOLOR_XRGB(0, 255, 255));
const D3DXCOLOR    MAGENTA(D3DCOLOR_XRGB(255, 0, 255));

struct BoundingBox
{
	BoundingBox();

	bool isPointInside(D3DXVECTOR3& p);

	D3DXVECTOR3 _min;
	D3DXVECTOR3 _max;
};


struct BoundingSphere
{
	BoundingSphere();
	D3DXVECTOR3 _center;
	float _radius;
};


class Util {

	public:
		Util();


};



//
// Cleanup
//
template<class T> void Release(T t)
{
	if (t)
	{
		t->Release();
		t = 0;
	}
}


#endif // !UTIL_H
