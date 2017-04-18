#ifndef CAMERA_H
#define CAMERA_H

#include "Headers.h"
#include "Vertex.h"

class Camera
{
public:
	enum CameraType { LANDOBJECT, AIRCRAFT };
	Camera();
	Camera(CameraType cameraType);
	~Camera();
	void strafe(float units); // left/right
	void fly(float units); // up/down
	void walk(float units); // forward/backward
	void pitch(float angle); // rotate on right vector
	void yaw(float angle); // rotate on up vector
	void roll(float angle); // rotate on look vector
	void getViewMatrix(D3DXMATRIX* V);
	void setCameraType(CameraType cameraType);
	D3DXVECTOR3* getPosition();
	void setPosition(D3DXVECTOR3* pos);
	void getRight(D3DXVECTOR3* right);
	void getUp(D3DXVECTOR3* up);
	D3DXVECTOR3* getLook();
	D3DXMATRIX* GetViewMatrix();
	D3DXMATRIX* GetProjectionMatrix();
	D3DXVECTOR3* GetPosition();
private:
	CameraType _cameraType;
	D3DXVECTOR3 _right;
	D3DXVECTOR3 _up;
	D3DXVECTOR3 _look;
	D3DXVECTOR3 _pos;
	D3DXMATRIX _view;
	D3DXMATRIX _projection;

};

#endif // !CAMERA_H
