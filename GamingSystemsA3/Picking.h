#pragma once
#include "Headers.h"

class Picking {
	
	private:
		LPDIRECT3DDEVICE9* pDevice;  //graphics device

	public:
		Picking();
		Picking(LPDIRECT3DDEVICE9*);

};