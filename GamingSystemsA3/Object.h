#ifndef OBJECT_H
#define OBJECT_H

#include "Headers.h"
#include <atlbase.h>

/*
An Object represents a model loaded from a .x file.
*/
class Object {
private:
	LPD3DXMESH pMesh; // Our mesh object in sysmem
	D3DMATERIAL9* pMeshMaterials; // Materials for our mesh
	LPDIRECT3DTEXTURE9* pMeshTextures; // Textures for our mesh
	DWORD dwNumMaterials;   // Number of mesh materials
	LPDIRECT3DDEVICE9* pDevice;//graphics device
	D3DXMATRIX worldMatrix;
	LPCWSTR filename;

public:
	Object();
	Object(LPDIRECT3DDEVICE9*, LPCWSTR);
	void setFile(LPCWSTR);
	void setDevice(LPDIRECT3DDEVICE9*);
	int InitGeometry();
	void cleanup();
	void setupMatrices(D3DXMATRIX matView);
	void drawObject();
	void translate(float, float, float);
	void rotateAboutX(float);
	void rotateAboutY(float);
	void rotateAboutZ(float);

};

#endif // !OBJECT_H
