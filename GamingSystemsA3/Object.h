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
	int x = worldMatrix._41;
	int y = worldMatrix._42;
	int z = worldMatrix._43;
	DWORD dwNumMaterials;   // Number of mesh materials
	D3DXMATRIX worldMatrix;
	LPD3DXMESH pMesh; // Our mesh object in sysmem
	D3DMATERIAL9* pMeshMaterials; // Materials for our mesh
	LPDIRECT3DTEXTURE9* pMeshTextures; // Textures for our mesh
	D3DXVECTOR3 _center;   //Bounding sphere center for picking.
	float _radius;   //Bounding sphere radius for picking.

};

#endif // !OBJECT_H
