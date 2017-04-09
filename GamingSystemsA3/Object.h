#ifndef OBJECT_H
#define OBJECT_H

#include "Headers.h"
#include <atlbase.h>

//Defines a ray.
struct Ray
{
	D3DXVECTOR3 _origin;
	D3DXVECTOR3 _direction;
};



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
	D3DXMATRIX worldMatrix;
	D3DXVECTOR3 _center;
	float _radius;

};

#endif // !OBJECT_H
