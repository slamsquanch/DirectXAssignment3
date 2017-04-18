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
	float x;
	float y;
	float z;
	void rotateAboutX(float);
	void rotateAboutY(float);
	void rotateAboutZ(float);
	float getX();
	float getY();
	float getZ();
	void setX(float x);
	void setY(float y);
	void setZ(float z);
	float getXrotate();
	float getYrotate();
	float getZrotate();
	void setXrotate(float theta);
	void setYrotate(float theta);
	void setZrotate(float theta);
	void setXScale(float x);
	void setYScale(float y);
	void setZScale(float z);
	float getXScale();
	float getYScale();
	float getZScale();
	DWORD dwNumMaterials;   // Number of mesh materials
	D3DXMATRIX worldMatrix;
	LPD3DXMESH pMesh; // Our mesh object in sysmem
	D3DMATERIAL9* pMeshMaterials; // Materials for our mesh
	LPDIRECT3DTEXTURE9* pMeshTextures; // Textures for our mesh
	D3DXVECTOR3 _center;   //Bounding sphere center for picking.
	float _radius;   //Bounding sphere radius for picking.
	LPDIRECT3DDEVICE9* pDevice;//graphics device
	float xTheta = 0;
	float yTheta = 0;
	float zTheta = 0;

};

#endif // !OBJECT_H
