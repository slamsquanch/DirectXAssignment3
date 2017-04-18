#include "Headers.h"

Object::Object() :pMesh(0), pMeshMaterials(0), pMeshTextures(0), dwNumMaterials(0), pDevice(0), filename(){}

/*
Constructor for an Object, stores the filename for the .x object to load.

@param newDevice - The directx device that is being used to display the objects
@param newFilename - The file path of the .x file to object to load and display
*/
Object::Object(LPDIRECT3DDEVICE9* newDevice, LPCWSTR newFilename) : pMesh(0), pMeshMaterials(0), pMeshTextures(0), dwNumMaterials(0), pDevice(newDevice), filename(newFilename) {}

void Object::setFile(LPCWSTR newFilename) {
	filename = newFilename;
}

void Object::setDevice(LPDIRECT3DDEVICE9* newDevice) {
	pDevice = newDevice;
}

/*
Loads the .x file, and the corresponding materials and textures.
*/
int Object::InitGeometry() {
	LPD3DXBUFFER pD3DXMtrlBuffer;

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM,
		*pDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &dwNumMaterials,
		&pMesh)))
	{
		TCHAR text[200];
		_stprintf_s(text, 200, TEXT("..\\%s"), filename);
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromX(text, D3DXMESH_SYSTEMMEM,
			*pDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &dwNumMaterials,
			&pMesh)))
		{
			MessageBox(NULL, TEXT("Could not find mesh"), TEXT("Object.cpp"), MB_OK);
			return E_FAIL;
		}
	}

	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	pMeshMaterials = new D3DMATERIAL9[dwNumMaterials];
	pMeshTextures = new LPDIRECT3DTEXTURE9[dwNumMaterials];

	for (DWORD i = 0; i < dwNumMaterials; i++)
	{
		// Copy the material
		pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		pMeshMaterials[i].Ambient = pMeshMaterials[i].Diffuse;

		pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFile(*pDevice,
				CA2CT(d3dxMaterials[i].pTextureFilename),
				&pMeshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const TCHAR* strPrefix = TEXT("..\\");
				const int lenPrefix = lstrlen(strPrefix);
				TCHAR strTexture[MAX_PATH];
				lstrcpyn(strTexture, strPrefix, MAX_PATH);
				lstrcpyn(strTexture + lenPrefix, CA2CT(d3dxMaterials[i].pTextureFilename), MAX_PATH - lenPrefix);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFile(*pDevice,
					strTexture,
					&pMeshTextures[i])))
				{
					MessageBox(NULL, TEXT("Could not find texture map"), TEXT("Object.cpp"), MB_OK);
				}
			}
		}
	}

	// Done with the material buffer
	pD3DXMtrlBuffer->Release();

	D3DXMatrixIdentity(&worldMatrix);

	return S_OK;
}

/*

*/
void Object::cleanup() {
	if (pMeshMaterials != NULL)
		delete[] pMeshMaterials;

	if (pMeshTextures)
	{
		for (DWORD i = 0; i < dwNumMaterials; i++)
		{
			if (pMeshTextures[i])
				pMeshTextures[i]->Release();
		}
		delete[] pMeshTextures;
	}
	if (pMesh != NULL)
		pMesh->Release();
}

void Object::setupMatrices(D3DXMATRIX matView) {

	(*pDevice)->SetTransform(D3DTS_VIEW, &matView);

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	(*pDevice)->SetTransform(D3DTS_PROJECTION, &matProj);
}

void Object::drawObject() {
	(*pDevice)->SetTransform(D3DTS_WORLD, &worldMatrix);

	// Meshes are divided into subsets, one for each material. Render them in
	// a loop
	for (DWORD i = 0; i<dwNumMaterials; i++)
	{
		// Set the material and texture for this subset
		(*pDevice)->SetMaterial(&pMeshMaterials[i]);
		(*pDevice)->SetTexture(0, pMeshTextures[i]);

		// Draw the mesh subset
		pMesh->DrawSubset(i);
	}
}

void Object::translate(float x, float y, float z) {
	D3DXMATRIX transMatrix;

	D3DXMatrixTranslation(&transMatrix, x, y, z);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &transMatrix);
}

void Object::rotateAboutX(float theta) {
	D3DXMATRIX rotMatrix;
	float x, y, z;

	x = worldMatrix._41;
	y = worldMatrix._42;
	z = worldMatrix._43;

	this->translate(-x, -y, -z);

	D3DXMatrixRotationX(&rotMatrix, theta);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotMatrix);

	this->translate(x, y, z);
}

void Object::rotateAboutY(float theta) {
	D3DXMATRIX rotMatrix;
	float x, y, z;

	x = worldMatrix._41;
	y = worldMatrix._42;
	z = worldMatrix._43;

	this->translate(-x, -y, -z);

	D3DXMatrixRotationY(&rotMatrix, theta);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotMatrix);

	this->translate(x, y, z);
}

void Object::rotateAboutZ(float theta) {
	D3DXMATRIX rotMatrix;
	float x, y, z;

	x = worldMatrix._41;
	y = worldMatrix._42;
	z = worldMatrix._43;

	this->translate(-x, -y, -z);

	D3DXMatrixRotationZ(&rotMatrix, theta);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotMatrix);

	this->translate(x, y, z);
}


float Object::getX() 
{ 
	return worldMatrix._41; 
}


float Object::getY() 
{ 
	return worldMatrix._42; 
}


float Object::getZ() 
{ 
	return worldMatrix._43; 
}


void Object::setX(float x)
{
	worldMatrix._41 = x;
}


void Object::setY(float y)
{
	worldMatrix._42 = x;
}


void Object::setZ(float z)
{
	worldMatrix._43 = x;
}



float Object::getXScale()
{ 
	return worldMatrix._11; 
}


float Object::getYScale() {
	return worldMatrix._22; 
}


float Object::getZScale() { 
	return worldMatrix._33; 
}



void Object::setXrotate(float theta) 
{
	xTheta = theta;
}


void Object::setYrotate(float theta)
{
	yTheta = theta;
}


void Object::setZrotate(float theta)
{
	zTheta = theta;
}



float Object::getXrotate()
{
	return xTheta;
}


float Object::getYrotate() {
	return yTheta;
}


float Object::getZrotate() {
	return zTheta;
}



void Object::setXScale(float x)
{
	worldMatrix._11 = x;
}


void Object::setYScale(float y)
{
	worldMatrix._22 = x;
}


void Object::setZScale(float z)
{
	worldMatrix._33 = x;
}