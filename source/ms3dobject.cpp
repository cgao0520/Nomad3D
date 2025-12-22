// ms3dobject.cpp: implementation of the CMS3DObject class.
//
//////////////////////////////////////////////////////////////////////

#include "nomad3d.h"
#include "ms3dobject.h"
#include "vertex.h"
#include "fileaccess.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CMS3DObject::CMS3DObject()
	{
		
	}
	
	CMS3DObject::~CMS3DObject()
	{
		
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	bool CMS3DObject::ReadObjectFile(const char* szFilename)
	{
		unsigned char * ucpBuffer = NULL;
		unsigned char * ucpPtr = NULL;
		
		CFileAccess obj(szFilename);
		ucpBuffer = obj.GetBuffer();
		if(!ucpBuffer)
			return false;

		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_YELLOW, "%s file : ", szFilename);

		ucpPtr=ucpBuffer;

		//Check out the header, it should be 10 bytes, MS3D000000
		if(memcmp(ucpPtr, "MS3D000000", 10) != 0)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "%s is not a valid Milkshape 3D file", szFilename);
			return false;
		}
		//Check the version (should be 3 or 4)
		ucpPtr += 10;
		if(*(int *)ucpPtr != 3 && *(int *)ucpPtr != 4)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "%s is the wrong version, should be 3 or 4", szFilename);
			return false;
		}
		ucpPtr += 4;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		int i=0, j=0;

		//Read the vertices
		m_unNumVerts = *(unsigned short *)ucpPtr;
		ucpPtr += 2;
		m_pVertList = new CVertex4[m_unNumVerts];
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"Total Vertices is : %d",m_unNumVerts);
		for(i=0; i<m_unNumVerts; i++)
		{
			float* pf=(float*)(ucpPtr+1);
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"%f, %f, %f",pf[0],pf[1],pf[2]);
			m_pVertList[i].Assign((float*)(ucpPtr+1),*((char*)(ucpPtr+13)));
			ucpPtr+=15;
		}

		//Read the triangles
		m_unNumPolys = *(unsigned short *)ucpPtr;
		ucpPtr += 2;
		m_pPolyList = new CPolygon[m_unNumPolys];
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"Total Polygons is : %d",m_unNumPolys);
		for(i=0; i<m_unNumPolys; i++)
		{
			unsigned short* ps=(unsigned short*)(ucpPtr+2);
			float* pTextCoords = (float*)(ucpPtr+44);
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"%d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf",ps[0],ps[1],ps[2]
				,pTextCoords[0],pTextCoords[1],pTextCoords[2],pTextCoords[3],pTextCoords[4],pTextCoords[5]);
			//m_pPolyList[i].Assign((unsigned short*)(ucpPtr+2),(float*)(ucpPtr+8),(float*)(ucpPtr+44));
			m_pPolyList[i].AssignWithInitVertexNormal(m_pVertList,(unsigned short*)(ucpPtr+2),(float*)(ucpPtr+8),(float*)(ucpPtr+44));
			ucpPtr+=70;
		}

		//Load meshs (groups)
		m_unNumMeshes = *(unsigned short *)ucpPtr;
		ucpPtr += 2;
		m_pMeshList = new CMesh[m_unNumMeshes];
		unsigned short* pus=NULL;
		unsigned short usNumPolys=0;
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"Total Meshes is : %d",m_unNumMeshes);
		for(i=0; i<m_unNumMeshes; i++)
		{
			usNumPolys=*((unsigned short*)(ucpPtr+33));

			//////////////////////////////////////////////////////////////////////////
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"%d Mesh has %d Polygons:",i,usNumPolys);
			for(int j=0; j<usNumPolys; j++)
			{
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"%d ", *(((unsigned short*)(ucpPtr + 35))+j) );
			}
			//////////////////////////////////////////////////////////////////////////
			m_pMeshList[i].Assign(usNumPolys, /*polygon indices point*/ucpPtr+35, /*material index*/*((char*)(ucpPtr+35 + usNumPolys * 2)));
			ucpPtr+=35 + usNumPolys * 2 + 1;
		}


		//Read material information
		m_unNumMaterials = *(unsigned short *)ucpPtr;
		ucpPtr += 2;
		if(m_unNumMaterials > 0)
		{
			m_pMaterialList = new CMaterial[m_unNumMaterials];
			//Copy material information
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"Total Materials is : %d",m_unNumMaterials);
			for(i = 0; i<m_unNumMaterials; i++)
			{
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"%d Material :",i);
				float* pfA=(float*)(ucpPtr+32);
				float* pfD=(float*)(ucpPtr+48);
				float* pfS=(float*)(ucpPtr+64);
				float* pfE=(float*)(ucpPtr+80);
				float fShiness=*((float*)(ucpPtr+100));
				float fTransparency=*((float*)(ucpPtr+100));
				char* szTextureFilename=(char*)(ucpPtr+105);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Ambient : %f, %f, %f, %f ",pfA[0],pfA[1],pfA[2],pfA[3]);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Diffuse : %f, %f, %f, %f ",pfD[0],pfD[1],pfD[2],pfD[3]);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Specular : %f, %f, %f, %f ",pfS[0],pfS[1],pfS[2],pfS[3]);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Emissive : %f, %f, %f, %f ",pfE[0],pfE[1],pfE[2],pfE[3]);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Shiness : %f ",fShiness);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Transparency : %f ",fTransparency);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN,"Texture File : %s ",szTextureFilename);
				
				
				m_pMaterialList[i].Assign(
					(float*)(ucpPtr+32),
					(float*)(ucpPtr+48),
					(float*)(ucpPtr+64),
					(float*)(ucpPtr+80),
					*((float*)(ucpPtr+96)),
					*((float*)(ucpPtr+100)),
					(char*)(ucpPtr+105)
					);
				ucpPtr+=361;
			}
		}
		else
		{
			m_pMaterialList = new CMaterial[1];
			float a[4] = {0.2f, 0.2f, 0.2f, 1.0f};
			float d[4] = {0.5f, 0.5f, 0.5f, 1.0f};
			float s[4] = {0.3f, 0.3f, 0.3f, 1.0f};
			float e[4] = {0.01f, 0.01f, 0.01f, 1.0f};
			float fShininess = 1.0f;
			float fTransparency = 1.0f;
			m_pMaterialList[0].Assign(a,d,s,e,fShininess,fTransparency,NULL);

			m_unNumMaterials = 1;

			for(i=0; i<m_unNumMeshes; i++)
			{
				m_pMeshList[i].SetMaterial(0);
			}
		}

		//Read Animation information
		float fAnimationFPS = *(float*)ucpPtr; ucpPtr += 4;
		float fCurrentTime = *(float*)ucpPtr; ucpPtr += 4;
		int nTotalFrames = *(int*)ucpPtr; ucpPtr += 4;
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED,"fAnimationFPS : %f",fAnimationFPS);
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED,"fCurrentTime : %f",fCurrentTime);
		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED,"nTotalFrames : %d",nTotalFrames);

		//Read Joint information
		m_unNumJoints = *(unsigned short *)ucpPtr;
		ucpPtr += 2;
		if(m_unNumJoints > 0)
		{
			//Allocate memory
			m_pJointList = new CJoint[m_unNumJoints];
			typedef char Name[32];
			Name* pName = new Name[m_unNumJoints];
			Name* pParent = new Name[m_unNumJoints];
			//Read in joint info
			CMatrix4 matLocal;
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_BLUE,"Total Joints is : %d",m_unNumJoints);
			for(i = 0; i < m_unNumJoints; i++)
			{
				memcpy(pName+i, ucpPtr+1, 32);
				memcpy(pParent+i, ucpPtr+33, 32);
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED,"No.%d Joint, Name: %s, Parent: %s",i,pName+i,pParent+i);
				unsigned short usNumRotFrames = *(unsigned short*)(ucpPtr + 89);
				unsigned short usNumTransFrames = *(unsigned short*)(ucpPtr + 91);
				CKeyFrame* pRotKeyFrameData = (CKeyFrame*)(ucpPtr + 93);
				int nRotKeyFrameSize = sizeof(CKeyFrame) * usNumRotFrames;
				CKeyFrame* pTransKeyFrameData = (CKeyFrame*)(ucpPtr + 93 + nRotKeyFrameSize);
				int nTransKeyFrameSize = sizeof(CKeyFrame) * usNumTransFrames;
				
				CalcLocalMatrix((float*)(ucpPtr+65), (float*)(ucpPtr+77), matLocal);
				
				m_pJointList[i].Assign(NM3D_JOINT_NO_PARENT, &matLocal, usNumRotFrames, \
					usNumTransFrames,pRotKeyFrameData, pTransKeyFrameData);
				
				ucpPtr += 93 + nRotKeyFrameSize + nTransKeyFrameSize;
			}
			
			//Find the parent joint array indices
			for(i = 0; i < m_unNumJoints; i++)
			{
				if(pParent[i][0] != '\0')//If the bone has a parent
				{
					//Compare names of theparent bone of x with the names of all bones
					for(j = 0; j < m_unNumJoints; j++)
					{
						//A match has been found
						if(strcmp(pName[j], pParent[i]) == 0)
						{
							m_pJointList[i].m_nParent = j;
							break;
						}
					}
				}
				else//The bone has no parent
				{
					m_pJointList[i].m_nParent = NM3D_JOINT_NO_PARENT;
				}
			}
			
			for(i = 0; i < m_unNumJoints; i++)
			{
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_WHITE,"No.%d Joint, Name: %s, Parent: %s--->%d",i,pName+i,pParent+i,m_pJointList[i].m_nParent);
			}

			delete[] pName;
			delete[] pParent;
		}


		if(m_unNumJoints > 0)
			SetupSkeleton();
		else
		{
			m_pVertListTran = new CVertex4[m_unNumVerts];
			for(i=0; i<m_unNumVerts; i++)
				m_pVertListTran[i] = m_pVertList[i];
		}

		ComputeRadius();

		SetPolygonMaterialAndVertList();

		return true;
	}

	void CMS3DObject::CalcLocalMatrix(const float* fRotation, const float* fPosition, CMatrix4& mat)
	{
		CMatrix4 matRot;
		CMatrix4 matTrans;
		CMatrix4::Rotate(R2D(fRotation[0]), R2D(fRotation[1]), R2D(fRotation[2]), matRot);
		CMatrix4::Translate(fPosition[0], fPosition[1], fPosition[2], matTrans);
		mat = matRot;
		mat *= matTrans;
#if 1
 		int j;

  		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Translation: %lf, %lf, %lf",fPosition[0],fPosition[1],fPosition[2]);
  		NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Rotation: %lf, %lf, %lf",fRotation[0],fRotation[1],fRotation[2]);

		static int i=0;
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN, "No.%d Local:",i);
			for(j=0; j<4; j++)
			{
				float * pp = mat[j];
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "%lf, %lf, %lf, %lf", pp[0], pp[1], pp[2], pp[3] );
			}
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN, "\n\n");
		i++;
#endif
	}

	bool CMS3DObject::SetupSkeleton()
	{
		int i=0;
	
		if(m_unNumJoints <= 0)
			return false;		

		for(i = 0; i < m_unNumJoints; i++)
		{
			if(m_pJointList[i].m_nParent != NM3D_JOINT_NO_PARENT)
				m_pJointList[i].m_matAbs = m_pJointList[i].m_matLocal * m_pJointList[m_pJointList[i].m_nParent].m_matAbs;
			else
				m_pJointList[i].m_matAbs = m_pJointList[i].m_matLocal;

			m_pJointList[i].m_matFinal = m_pJointList[i].m_matAbs;

			//m_pJointList[i].m_matFinal.Inverse(m_pJointList[i].m_matFinal);
/*
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN, "No.%d Joint's Final Matrix:",i);
			for(int jj=0; jj<4; jj++)
			{
				float * pp = m_pJointList[i].m_matFinal[jj];
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "%lf, %lf, %lf, %lf", pp[0], pp[1], pp[2], pp[3] );
			}
			*/
		}

		CMatrix4* pMat = NULL;
		for(i = 0; i < m_unNumVerts; i++)
		{
			int nBoneID = m_pVertList[i].GetBone();
			if(nBoneID == NM3D_JOINT_NO_PARENT)
				continue;
			
			pMat = & (m_pJointList[m_pVertList[i].GetBone()].m_matFinal);

			CMatrix4 im;
			im.Inverse(*pMat);
			m_pVertList[i] *= im;
		
  			//pMat->InverseTranslateVec(m_pVertList[i].Get());
  			//pMat->InverseRotateVec(m_pVertList[i].Get());

			m_pVertList[i][3]=1;
			
		}

/*
		//Transform the Vertice's normals
		for(i = 0; i < m_unNumPolys; i++)
		{
			CPolygon * pTri = &m_pPolyList[i];
			
			for(int z = 0; z < 3; z++)
			{
				CVertex4 * pVert = &m_pVertListTran[pTri->GetVertIndices()[z]];
				
				//if it is not attached to a bone, don't do any transforms
				if(pVert->GetBone() == NM3D_JOINT_NO_PARENT)
					continue;
				
				CJoint * pJoint = &m_pJointList[pVert->GetBone()];
				
				pJoint->m_matFinal.InverseRotateVec(pTri->GetNormals(z)->Get());
				
			}
		}
*/

#if 0 //Set the MS3D Object to the first frame(Skeleton Frame)!
		CMatrix4 matRot;
		CMatrix4 matTrans;
		CMatrix4 matFinal;
		CJoint* pJoint = NULL;

		for(i=0; i<m_unNumJoints; i++)
		{
			pJoint = &(m_pJointList[i]);

			//NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_WHITE, "No.%d Joint(RotFrames,TranFrames): %d, %d",i,pJoint->m_usNumRotFrames,pJoint->m_usNumTransFrames);
			float* pRot = pJoint->m_pRotKeyFrames[0].m_fParam;
			float* pTran = pJoint->m_pTransKeyFrames[0].m_fParam;
			
			CMatrix4::Rotate(R2D(pRot[0]), R2D(pRot[1]), R2D(pRot[2]), matRot);
			CMatrix4::Translate(pTran[0], pTran[1], pTran[2], matTrans);
			
			matFinal = matRot;
			matFinal *= matTrans;
			
			matFinal *= pJoint->m_matLocal;
			
			if(pJoint->m_nParent == NM3D_JOINT_NO_PARENT)
				pJoint->m_matFinal = matFinal;
			else
				pJoint->m_matFinal = matFinal * m_pJointList[pJoint->m_nParent].m_matFinal;

			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_GREEN, "AAAAAAAAAAAAAA No.%d Joint's Final Matrix:",i);
			for(int jj=0; jj<4; jj++)
			{
				float * pp = pJoint->m_matFinal[jj];
				NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "%lf, %lf, %lf, %lf", pp[0], pp[1], pp[2], pp[3] );
			}
		}

		if(m_unNumJoints>0)
		{
			for(i = 0; i<m_unNumVerts; i++)
			{
				int nBoneID = m_pVertList[i].GetBone();

				if(nBoneID == NM3D_JOINT_NO_PARENT)
					continue;
				
				pJoint = &(m_pJointList[nBoneID]);
				
				m_pVertList[i] *= pJoint->m_matFinal; 
				m_pVertList[i][3]=1;
			}
		}

#endif

		m_pVertListTran = new CVertex4[m_unNumVerts];
		//memcpy(m_pVertListTran, m_pVertList, sizeof(m_pVertList[0])*m_unNumVerts);
		for(i=0; i<m_unNumVerts; i++)
			m_pVertListTran[i] = m_pVertList[i];
		

		return true;
	}

	void CMS3DObject::SetPolygonMaterialAndVertList()
	{
		// 1. Set Polygon Material
		int i=0,j=0;
		CMesh* pMesh = NULL;
		CMaterial* pMaterial = NULL;
		for(i=0; i<m_unNumMeshes; i++)
		{
			pMesh = &(m_pMeshList[i]);
			pMaterial = &(m_pMaterialList[pMesh->m_cMaterial]);
			for(j=0; j<pMesh->m_usNumPolys; j++)
			{
				m_pPolyList[pMesh->m_pusIndices[j]].m_pMaterial = pMaterial;
			}
		}

		// 2. Set Polygon Vertices Point
		for(i=0; i<m_unNumPolys; i++)
		{
			m_pPolyList[i].m_pVertList = m_pVertListTran;//m_pVertList;
		}
	}

}

