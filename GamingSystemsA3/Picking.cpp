#include "Headers.h"




/*
Constructor for a Picking.

@param newDevice - The directx device that is being used to calculate the object's projection rays. 
*/
Picking::Picking(LPDIRECT3DDEVICE9* pdevice){
	pDevice = pdevice;
}
