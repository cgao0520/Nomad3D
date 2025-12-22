// camera.h: interface for the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CAMERA_H_A20874F7_AA39_46A1_B406_D27AF48502E8
#define CAMERA_H_A20874F7_AA39_46A1_B406_D27AF48502E8

#include "vector4.h"
#include "matrix4.h"

namespace Nomad3D
{
	class CCamera
	{
	public:
		CCamera();
		//virtual ~CCamera();
		//CCamera(const CCamera& cam);
		//CCamera& operator = (const CCamera& cam);
	public:
		void Pitch(float angle); //roate around U
		void Yaw(float angle); //roate around V
		void Roll(float angle); //roate around N
		void Walk(float units); // N
		void Strafe(float units); // U
		void fly(float units); // V
		const CMatrix4& LookAt(const CPoint4& eye, const CPoint4& look, const CVector4& up);//look is the target which camera is looking
		const CMatrix4& LookAt(const CPoint4& eye, float theta, float phi, const CVector4& up);
		const CMatrix4& SetPerspective(float fFovy, float fAspect, float fNear, float fFar);
		CPoint4 GetPosition() const;
// 		const CMatrix4& GetLookMatrix() const;
// 		const CMatrix4& GetPerspectiveMatrix() const;
		CMatrix4 GetCameraMatrix();
		CMatrix4 GetUVNMatrix();
		CMatrix4 GetPerspectiveMatrix();
		float GetNear() const;
		float GetFar() const;
		float GetLeft() const;
		float GetRight() const;
		float GetTop() const;
		float GetBottom() const;
		float GetFovy() const;
		float GetAspect() const;
	protected:
		CPoint4 CalculateLook(float theta, float phi) const;
		void GenerateViewMatrix();
		void GeneratePerspectiveMatrix();
	protected:
		CPoint4		m_vPos;//the position of this camera in the world coordinate
		CVector4	m_vU,m_vV,m_vN;//three vectors of u,v,n
		CMatrix4	m_mWorld2Camera;
		//////////////////////////////////////////////////////////////////////////
		float		m_fFovy;//use degree, it is a angle between the top face and bottom face in the frustum
		float		m_fAspect;// width/height ratio(aspect ratio)
		float		m_fNear;//the near face's z coordinate
		float		m_fFar;//the far face's z coordinate
		CMatrix4	m_mCamera2Perspective;
		//////////////////////////////////////////////////////////////////////////
		float		m_fLeft;
		float		m_fRight;
		float		m_fTop;
		float		m_fBottom;
	};
}

#endif //CAMERA_H_A20874F7_AA39_46A1_B406_D27AF48502E8
