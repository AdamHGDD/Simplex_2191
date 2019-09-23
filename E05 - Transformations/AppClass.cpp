#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_WHITE);

	for (size_t i = 0; i < 46; i++)
	{
		meshes.push_back(new MyMesh());
		meshes[i]->GenerateCube(1.0f, C_WHITE);

		positions.push_back(vector3(0,0,0));
	}
	//m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();

	//matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	//static float value = 0.0f;
	//matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	//value += 0.01f;

	//matrix4 m4Model = m4Translate * m4Scale;
	float static fposition = 0.0f;
	fposition += .05f;

	matrix4 m4Model = glm::translate(IDENTITY_M4, vector3(fposition, 0, 0));


#pragma region Hard Coded Pixels
	// Column One
	positions[0] = vector3(fposition, 1, 0);
	positions[1] = vector3(fposition, 2, 0);
	positions[2] = vector3(fposition, 3, 0);

	// Column Two
	positions[3] = vector3(fposition + 1, 3, 0);
	positions[4] = vector3(fposition + 1, 4, 0);

	// Column Three
	positions[5] = vector3(fposition + 2, 1, 0);
	positions[6] = vector3(fposition + 2, 2, 0);
	positions[7] = vector3(fposition + 2, 3, 0);
	positions[8] = vector3(fposition + 2, 4, 0);
	positions[9] = vector3(fposition + 2, 5, 0);
	positions[10] = vector3(fposition + 2, 7, 0);

	// Column Four
	positions[11] = vector3(fposition + 3, 0, 0);
	positions[12] = vector3(fposition + 3, 2, 0);
	positions[13] = vector3(fposition + 3, 3, 0);
	positions[14] = vector3(fposition + 3, 5, 0);
	positions[15] = vector3(fposition + 3, 6, 0);

	// Column Five
	positions[16] = vector3(fposition + 4, 0, 0);
	positions[17] = vector3(fposition + 4, 2, 0);
	positions[18] = vector3(fposition + 4, 3, 0);
	positions[19] = vector3(fposition + 4, 4, 0);
	positions[20] = vector3(fposition + 4, 5, 0);

	//Column Six
	positions[21] = vector3(fposition + 5, 2, 0);
	positions[22] = vector3(fposition + 5, 3, 0);
	positions[23] = vector3(fposition + 5, 4, 0);
	positions[24] = vector3(fposition + 5, 5, 0);

	// Column Seven
	positions[25] = vector3(fposition + 6, 0, 0);
	positions[26] = vector3(fposition + 6, 2, 0);
	positions[27] = vector3(fposition + 6, 3, 0);
	positions[28] = vector3(fposition + 6, 4, 0);
	positions[29] = vector3(fposition + 6, 5, 0);

	// Column Eight
	positions[30] = vector3(fposition + 7, 0, 0);
	positions[31] = vector3(fposition + 7, 2, 0);
	positions[32] = vector3(fposition + 7, 3, 0);
	positions[33] = vector3(fposition + 7, 5, 0);
	positions[34] = vector3(fposition + 7, 6, 0);

	// Column Nine
	positions[35] = vector3(fposition + 8, 1, 0);
	positions[36] = vector3(fposition + 8, 2, 0);
	positions[37] = vector3(fposition + 8, 3, 0);
	positions[38] = vector3(fposition + 8, 4, 0);
	positions[39] = vector3(fposition + 8, 5, 0);
	positions[40] = vector3(fposition + 8, 7, 0);

	// Column Ten
	positions[41] = vector3(fposition + 9, 3, 0);
	positions[42] = vector3(fposition + 9, 4, 0);

	// Column Eleven
	positions[43] = vector3(fposition + 10, 1, 0);
	positions[44] = vector3(fposition + 10, 2, 0);
	positions[45] = vector3(fposition + 10, 3, 0);
#pragma endregion


	//m_pMesh->Render(m4Projection, m4View, m4Model);

	for (size_t i = 0; i < 46; i++)
	{
		m4Model = glm::translate(IDENTITY_M4, positions[i]);

		meshes[i]->Render(m4Projection, m4View, m4Model);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}