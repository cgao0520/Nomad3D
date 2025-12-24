// camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "camera.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CCamera::CCamera()
	{
	}
	
	void CCamera::Pitch(float angle)
	{
		CMatrix4 m;
		CMatrix4::Rotate(angle, m_vU, m);

		m_vV = m_vV * m; //m_vV.DivideW();
		m_vN = m_vN * m; //m_vN.DivideW();
	}

	void CCamera::Yaw(float angle)
	{
		CMatrix4 m;
		CMatrix4::Rotate(angle, m_vV, m);
		//CMatrix4::Rotate(0,angle,0,m);

		m_vU *= m; //m_vU.DivideW();
		m_vN *= m; //m_vN.DivideW();
	}
	
	void CCamera::Roll(float angle)
	{
		CMatrix4 m;
		CMatrix4::Rotate(angle, m_vN, m);

		m_vU *= m; //m_vU.DivideW();
		m_vV *= m; //m_vV.DivideW();
	}

	void CCamera::Walk(float units)
	{
		m_vPos += m_vN * units;
	}

	void CCamera::Strafe(float units)
	{
		m_vPos += m_vU * units;
	}
	
	void CCamera::fly(float units)
	{
		m_vPos += m_vV * units;
	}

	
	//this function generate a matrix, which let the camera from a certain world position to origin position.
	const CMatrix4& CCamera::LookAt(const CPoint4& eye, const CPoint4& look, const CVector4& up)
	{
		m_vPos = eye;
		m_vN = eye - look;
		m_vU = Cross4(up , m_vN);
		m_vN.Normalize(); 
		m_vU.Normalize();
		m_vV = Cross4(m_vN , m_vU);

		GenerateViewMatrix();

		return m_mWorld2Camera;
	}

	void CCamera::GenerateViewMatrix()
	{
		//////////////////////////////////////////////////////////////////////////
		m_mWorld2Camera.Assign(
			m_vU.x(),			m_vV.x(),			m_vN.x(),			0,
			m_vU.y(),			m_vV.y(),			m_vN.y(),			0,
			m_vU.z(),			m_vV.z(),			m_vN.z(),			0,
			-Dot4(m_vPos,m_vU),	-Dot4(m_vPos,m_vV),	-Dot4(m_vPos,m_vN),	1.0f
			);

//		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_WHITE,"World2Camera Matrix is:");
//		NM3D_DEBUG_PRINT_MATRIX(NM3D_DEBUG_COLOR_WHITE,m_mWorld2Camera);
	}

	void CCamera::GeneratePerspectiveMatrix()
	{
		float n = m_fNear;
		float f = m_fFar;
		float t = m_fTop;
		float r = m_fRight;

		m_mCamera2Perspective.Assign(
			n/r,	0,		0,			0,
			0,		n/t,	0,			0,
			0,		0,	-(f+n)/(f-n),  -1,
			0,		0,	-2*(f*n)/(f-n),	0
			);
		
//		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_WHITE,"Camera2Perspective Matrix is:");
//		NM3D_DEBUG_PRINT_MATRIX(NM3D_DEBUG_COLOR_WHITE,m_mCamera2Perspective);

	}
	
	const CMatrix4& CCamera::LookAt(const CPoint4& eye, float theta, float phi, const CVector4& up)
	{
		return LookAt(eye,CalculateLook(theta,phi),up);
	}
	
	CPoint4 CCamera::CalculateLook(float theta, float phi) const
	{
		// Not implemented yet, refer to Page 380 of Chinese edition of the book:
		// "Tricks of the 3D Game Programming Gurus-Advanced 3D Graphics and Rasterization"
		(void)theta;
		(void)phi;
		return CPoint4();
	}

	//////////////////////////////////////////////////////////////////////////
	const CMatrix4& CCamera::SetPerspective(float fFovy, float fAspect, float fNear, float fFar)
	{
		m_fFovy = fFovy;
		m_fAspect = fAspect;
		m_fNear = fNear;
		m_fFar = fFar;

		//float t = float(fNear * tan(NM3D_PI/180*fFovy/2));
		float r = float(fNear * Tan(fFovy/2));
		float t = r/fAspect;

		m_fLeft = -r;
		m_fRight = r;
		m_fTop = t;
		m_fBottom = -t;

		GeneratePerspectiveMatrix();
	
		return m_mCamera2Perspective;
	}

	CPoint4 CCamera::GetPosition() const
	{
		return m_vPos;
	}

// 	const CMatrix4& CCamera::GetLookMatrix() const
// 	{
// 		return m_mWorld2Camera;
// 	}
// 	const CMatrix4& CCamera::GetPerspectiveMatrix() const
// 	{
// 		return m_mCamera2Perspective;
// 	}

	CMatrix4 CCamera::GetCameraMatrix()
	{
		GenerateViewMatrix();
		GeneratePerspectiveMatrix();

		return m_mWorld2Camera * m_mCamera2Perspective;
	}

	CMatrix4 CCamera::GetUVNMatrix()
	{
		GenerateViewMatrix();
		return m_mWorld2Camera;
	}

	CMatrix4 CCamera::GetPerspectiveMatrix()
	{
		GeneratePerspectiveMatrix();
		return m_mCamera2Perspective;
	}

	float CCamera::GetNear() const
	{
		return m_fNear;
	}

	float CCamera::GetFar() const
	{
		return m_fFar;
	}

	float CCamera::GetLeft() const
	{
		return m_fLeft;
	}

	float CCamera::GetRight() const
	{
		return m_fRight;
	}
	float CCamera::GetTop() const
	{
		return m_fTop;
	}
	float CCamera::GetBottom() const
	{
		return m_fBottom;
	}
	float CCamera::GetFovy() const
	{
		return m_fFovy;
	}
	float CCamera::GetAspect() const
	{
		return m_fAspect;
	}
}
