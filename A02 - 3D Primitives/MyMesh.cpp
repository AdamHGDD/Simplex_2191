#define _USE_MATH_DEFINES

#include "MyMesh.h"
#include <math.h>
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Cone Replacement Code
	for (size_t i = 0; i < a_nSubdivisions; i++)
	{
		// Angle
		float baseAngle = (float)2 * M_PI / a_nSubdivisions;

		// Points on circle
		glm::vec3 middle(0, 0, -.5f * a_fHeight);
		glm::vec3 point1(cos(baseAngle * i) * a_fRadius, sin(baseAngle * i) * a_fRadius, -.5f * a_fHeight);
		glm::vec3 point2(cos(baseAngle * (i + 1)) * a_fRadius, sin(baseAngle * (i + 1)) * a_fRadius, -.5f * a_fHeight);
		AddTri(middle, point2, point1);

		// Connect to the top
		glm::vec3 top(0, 0, .5* a_fHeight);
		AddTri(top,point1,point2);
	}

	// -------------------------------
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Cylinder Replacement Code
	for (size_t i = 0; i < a_nSubdivisions; i++)
	{
		// Angle
		float baseAngle = (float)2 * M_PI / a_nSubdivisions;

		// Points on circle 1
		glm::vec3 c1_middle(0, 0, -.5f * a_fHeight);
		glm::vec3 c1_point1(cos(baseAngle * i) * a_fRadius, sin(baseAngle * i) * a_fRadius, -.5f * a_fHeight);
		glm::vec3 c1_point2(cos(baseAngle * (i + 1)) * a_fRadius, sin(baseAngle * (i + 1)) * a_fRadius, -.5f * a_fHeight);
		AddTri(c1_middle, c1_point2, c1_point1);

		// Points on circle 2
		glm::vec3 c2_middle(0, 0, .5f * a_fHeight);
		glm::vec3 c2_point1(cos(baseAngle * i) * a_fRadius, sin(baseAngle * i) * a_fRadius, .5f * a_fHeight);
		glm::vec3 c2_point2(cos(baseAngle * (i + 1)) * a_fRadius, sin(baseAngle * (i + 1)) * a_fRadius, .5f * a_fHeight);
		AddTri(c2_middle, c2_point1, c2_point2);

		// Create Quad Between them
		AddQuad(c1_point2, c2_point2, c1_point1, c2_point1);
		//AddTri(c1_point2, c2_point2, c2_point1);
		//AddTri(c1_point2, c2_point1, c1_point1);
	}

	// -------------------------------
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Tube Replacement Code
	for (size_t i = 0; i < a_nSubdivisions; i++)
	{
		// Angle
		float baseAngle = (float)2 * M_PI / a_nSubdivisions;

		// Points on outer Circle 1
		glm::vec3 oc1_middle(0, 0, -.5f * a_fHeight);
		glm::vec3 oc1_point1(cos(baseAngle * i) * a_fOuterRadius, sin(baseAngle * i) * a_fOuterRadius, -.5f * a_fHeight);
		glm::vec3 oc1_point2(cos(baseAngle * (i + 1)) * a_fOuterRadius, sin(baseAngle * (i + 1)) * a_fOuterRadius, -.5f * a_fHeight);

		// Points on outer circle 2
		glm::vec3 oc2_middle(0, 0, .5f * a_fHeight);
		glm::vec3 oc2_point1(cos(baseAngle * i) * a_fOuterRadius, sin(baseAngle * i) * a_fOuterRadius, .5f * a_fHeight);
		glm::vec3 oc2_point2(cos(baseAngle * (i + 1)) * a_fOuterRadius, sin(baseAngle * (i + 1)) * a_fOuterRadius, .5f * a_fHeight);

		// Create quad on the outside
		AddQuad(oc1_point2, oc2_point2, oc1_point1, oc2_point1);

		// Points on inner Circle 1
		glm::vec3 ic1_middle(0, 0, -.5f * a_fHeight);
		glm::vec3 ic1_point1(cos(baseAngle * i) * a_fInnerRadius, sin(baseAngle * i) * a_fInnerRadius, -.5f * a_fHeight);
		glm::vec3 ic1_point2(cos(baseAngle * (i + 1)) * a_fInnerRadius, sin(baseAngle * (i + 1)) * a_fInnerRadius, -.5f * a_fHeight);

		// Points on inner circle 2
		glm::vec3 ic2_middle(0, 0, .5f * a_fHeight);
		glm::vec3 ic2_point1(cos(baseAngle * i) * a_fInnerRadius, sin(baseAngle * i) * a_fInnerRadius, .5f * a_fHeight);
		glm::vec3 ic2_point2(cos(baseAngle * (i + 1)) * a_fInnerRadius, sin(baseAngle * (i + 1)) * a_fInnerRadius, .5f * a_fHeight);

		// Create quad on the inside
		AddQuad(ic1_point2, ic1_point1, ic2_point2, ic2_point1);

		// Now Fill in the middle
		AddQuad(oc1_point1, ic1_point1, oc1_point2, ic1_point2); // Top
		AddQuad(oc2_point1, oc2_point2, ic2_point1, ic2_point2); // Bottom
	}

	// -------------------------------
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Sphere Replacement Code


	// I decided to make the sphere the way that is better for lighting engines
	// So I made a cube normally then broke it into subdivisions and then rounded it


	// Base 8 points of a cube
	// p = positive n = negative
	// I redid these points on my own to make it easier for me to process what each point was
	glm::vec3 ppp(a_fRadius, a_fRadius, a_fRadius);
	glm::vec3 ppn(a_fRadius, a_fRadius, -a_fRadius);
	glm::vec3 pnp(a_fRadius, -a_fRadius, a_fRadius);
	glm::vec3 pnn(a_fRadius, -a_fRadius, -a_fRadius);
	glm::vec3 npp(-a_fRadius, a_fRadius, a_fRadius);
	glm::vec3 npn(-a_fRadius, a_fRadius, -a_fRadius);
	glm::vec3 nnp(-a_fRadius, -a_fRadius, a_fRadius);
	glm::vec3 nnn(-a_fRadius, -a_fRadius, -a_fRadius);



	// Adding a face the way you would in a cube but every face is a 2x2 series of faces
#pragma region Loops for faces

	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(pnp, pnn, ppp, ppn); Original face on a cube
			// X and Zs for mini quad
			float y1 = pnp.y + ((ppp.y - pnp.y) * i / a_nSubdivisions);
			float y2 = pnp.y + ((ppp.y - pnp.y) * (i + 1) / a_nSubdivisions);
			float z1 = pnp.z + ((pnn.z - pnp.z) * j / a_nSubdivisions);
			float z2 = pnp.z + ((pnn.z - pnp.z) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botrig(pnp.x, y1, z1);
			glm::vec3 botlef(pnp.x, y2, z1);
			glm::vec3 toprig(pnp.x, y1, z2);
			glm::vec3 toplef(pnp.x, y2, z2);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}



	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(pnn,nnn,ppn,npn); Original face on a cube
			// X and Zs for mini quad
			float x1 = pnn.x + ((nnn.x - pnn.x) * i / a_nSubdivisions);
			float x2 = pnn.x + ((nnn.x - pnn.x) * (i + 1) / a_nSubdivisions);
			float y1 = pnn.y + ((ppn.y - pnn.y) * j / a_nSubdivisions);
			float y2 = pnn.y + ((ppn.y - pnn.y) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botrig(x1, y1, pnn.z);
			glm::vec3 botlef(x1, y2, pnn.z);
			glm::vec3 toprig(x2, y1, pnn.z);
			glm::vec3 toplef(x2, y2, pnn.z);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}



	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(nnn,nnp,npn,npp); Original face on a cube
			// X and Zs for mini quad
			float y1 = nnn.y + ((npn.y - nnn.y) * i / a_nSubdivisions);
			float y2 = nnn.y + ((npn.y - nnn.y) * (i + 1) / a_nSubdivisions);
			float z1 = nnn.z + ((nnp.z - nnn.z) * j / a_nSubdivisions);
			float z2 = nnn.z + ((nnp.z - nnn.z) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botrig(nnn.x, y1, z1);
			glm::vec3 botlef(nnn.x, y2, z1);
			glm::vec3 toprig(nnn.x, y1, z2);
			glm::vec3 toplef(nnn.x, y2, z2);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}



	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(nnp,pnp,npp,ppp); Original face on a cube
			// X and Zs for mini quad
			float x1 = nnp.x + ((pnp.x - nnp.x) * i / a_nSubdivisions);
			float x2 = nnp.x + ((pnp.x - nnp.x) * (i + 1) / a_nSubdivisions);
			float y1 = nnp.y + ((npp.y - nnp.y) * j / a_nSubdivisions);
			float y2 = nnp.y + ((npp.y - nnp.y) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botrig(x1, y1, nnp.z);
			glm::vec3 botlef(x1, y2, nnp.z);
			glm::vec3 toprig(x2, y1, nnp.z);
			glm::vec3 toplef(x2, y2, nnp.z);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}



	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(npn,npp,ppn,ppp); Original face on a cube
			// X and Zs for mini quad
			float x1 = npn.x + ((ppn.x - npn.x) * i / a_nSubdivisions);
			float x2 = npn.x + ((ppn.x - npn.x) * (i + 1) / a_nSubdivisions);
			float z1 = npn.z + ((npp.z - npn.z) * j / a_nSubdivisions);
			float z2 = npn.z + ((npp.z - npn.z) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botlef(x1, npn.y, z1);
			glm::vec3 botrig(x1, npn.y, z2);
			glm::vec3 toplef(x2, npn.y, z1);
			glm::vec3 toprig(x2, npn.y, z2);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}



	for (size_t j = 0; j < a_nSubdivisions; j++)
	{
		for (size_t i = 0; i < a_nSubdivisions; i++)
		{
			//AddQuad(pnn,pnp,nnn,nnp); Original face on a cube
			// X and Zs for mini quad
			float x1 = pnn.x + ((nnn.x - pnn.x) * i / a_nSubdivisions);
			float x2 = pnn.x + ((nnn.x - pnn.x) * (i + 1) / a_nSubdivisions);
			float z1 = pnn.z + ((pnp.z - pnn.z) * j / a_nSubdivisions);
			float z2 = pnn.z + ((pnp.z - pnn.z) * (j + 1) / a_nSubdivisions);

			// Points for mini quad
			glm::vec3 botlef(x1, pnn.y, z1);
			glm::vec3 botrig(x1, pnn.y, z2);
			glm::vec3 toplef(x2, pnn.y, z1);
			glm::vec3 toprig(x2, pnn.y, z2);

			// Rounding quad
			float dist_bl = sqrt(botlef.x * botlef.x + botlef.y * botlef.y + botlef.z * botlef.z);
			float dist_br = sqrt(botrig.x * botrig.x + botrig.y * botrig.y + botrig.z * botrig.z);
			float dist_tl = sqrt(toplef.x * toplef.x + toplef.y * toplef.y + toplef.z * toplef.z);
			float dist_te = sqrt(toprig.x * toprig.x + toprig.y * toprig.y + toprig.z * toprig.z);

			botlef = botlef * a_fRadius / dist_bl;
			botrig = botrig * a_fRadius / dist_br;
			toplef = toplef * a_fRadius / dist_tl;
			toprig = toprig * a_fRadius / dist_te;


			// Adding quad
			AddQuad(botlef, botrig, toplef, toprig);
		}
	}
#pragma endregion

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}