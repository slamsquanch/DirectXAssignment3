#pragma once

class Reflection {

	private:
		LPDIRECT3DDEVICE9* pDevice;

		D3DMATRIX *D3DMatrixReflect(D3DMATRIX, CONST D3DXPLANE ); 
		void RenderMirror();
		void setDevice(LPDIRECT3DDEVICE9*);


};