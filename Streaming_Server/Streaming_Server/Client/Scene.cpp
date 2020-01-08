#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
}


CScene::~CScene()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	srand(unsigned int(time(NULL)));

	//전체 오브젝트 리스트 생성
	m_nObjects = 3062;
	m_ppObjects = new CGameObject*[m_nObjects];
	int ObjectCount = 0;

	//전체 매쉬 리스트 생성
	m_nMeshs = 12;
	m_ppMeshs = new CMesh*[m_nMeshs];

	//플레이어 생성
	float PlaneWidth = 20.0f;
	float PlaneHeight = 20.0f;
	float PlaneDepth = 4.0f;
	m_ppMeshs[0] = new CAirplaneMeshDiffused(m_Vertex, PlaneWidth, PlaneHeight, PlaneDepth, XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
	m_ppMeshs[0]->SetOOBB(XMFLOAT3(0.0f, 0.25f * PlaneHeight, -PlaneDepth / 2), XMFLOAT3(PlaneWidth / 2, 0.4f*PlaneHeight, PlaneDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[0]->m_nStartInstance = 0;
	m_ppMeshs[0]->m_nInstance = 1;

	m_pPlayer = new CAirplanePlayer();
	m_pPlayer->Move(XMFLOAT3( float(m_nWidth / 2), 0.0f, 0.0f), false);
	m_pPlayer->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pPlayer->SetMesh(m_ppMeshs[0]);
	m_pPlayer->UpdateOOBB();
	m_ppObjects[ObjectCount++] = m_pPlayer;

	//레일
	float RailWidth = 20.0f;
	float RailHeight = 1.0f;
	float RailDepth = 2.0f;

	m_nFirst = 0;
	m_nDest = 0;
	m_nEnd = 4;

	m_ppMeshs[1] = new CCubeMeshDiffused(m_Vertex, m_Index, NULL, RailWidth, RailHeight, RailDepth);
	m_ppMeshs[1]->SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(10.0f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[1]->m_nStartInstance = 1;
	m_ppMeshs[1]->m_nInstance = 5;

	m_nRails = 50;
	m_ppRails = new CGameObject*[m_nRails];
	for (int i = 0; i < m_nRails; i++) {
		m_ppRails[i] = new CGameObject();
		m_ppRails[i]->SetPosition( float(m_nWidth / 2), 0.0f, float(i * 6));
		m_ppRails[i]->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		m_ppRails[i]->SetMesh(m_ppMeshs[1]);
		m_ppRails[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppRails[i];
	}

	//땅
	m_pTerrain = new CHeightMapTerrain(L"heightmap.raw", 513, 513, XMFLOAT3(10.0f, 4.0f, 10.0f));
	m_pTerrain->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	m_ppObjects[ObjectCount++] = m_pTerrain;
	m_pTerrain->CreateMesh(&m_ppMeshs[2], m_Vertex, m_Index, 257, 257, XMFLOAT4(0.0f, 0.2f, 0.0f, 0.0f), 51);

	//플레이어 컨텍스트 설정
	m_pPlayer->SetPlayerUpdatedContext(m_pTerrain);
	m_pPlayer->SetCameraUpdatedContext(m_pTerrain);

	//땅 크기
	m_nWidth = int(m_pTerrain->GetWidth());
	m_nHeight = int(m_pTerrain->GetLength());

	//나무
	float TreeWidth = 12.0f;
	float TreeHeight = 12.0f;
	float TreeDepth = 12.0f;

	m_ppMeshs[6] = new CTreeMesh(m_Vertex, m_Index, NULL, TreeWidth, TreeHeight, TreeDepth);
	m_ppMeshs[6]->SetOOBB(XMFLOAT3(0.0f, TreeHeight / 2, 0.0f), XMFLOAT3(TreeWidth/2, TreeHeight/2, TreeDepth/2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[6]->m_nStartInstance = 52;
	m_ppMeshs[6]->m_nInstance = 1000;

	m_nTrees = 2000;
	m_ppTrees = new CGameObject*[m_nTrees];

	for (int i = 0; i < 1000; i++) {
		m_ppTrees[i] = new CGameObject();

		//지형의 높이와 일치
		float fWidth = float(rand() % m_nWidth);
		float fDepth = float(rand() % m_nHeight);
		float fHeight = m_pTerrain->GetHeight(fWidth, fDepth);
		m_ppTrees[i]->SetPosition(fWidth, fHeight, fDepth);

		//지형의 법선과 일치
		XMFLOAT3 xmfNorm = m_pTerrain->GetNormal(fWidth, fDepth);
		m_ppTrees[i]->MatchUp(xmfNorm);

		XMFLOAT4 Color{ 0.2f, 0.0f, 0.0f, 0.0f };
		m_ppTrees[i]->SetColor(Color);
		m_ppTrees[i]->SetMesh(m_ppMeshs[6]);
		m_ppTrees[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppTrees[i];
	}
	
	TreeWidth = 12.0f;
	TreeHeight = 36.0f;
	TreeDepth = 12.0f;

	m_ppMeshs[7] = new CTreeMesh(m_Vertex, m_Index, m_ppMeshs[6], TreeWidth, TreeHeight, TreeDepth);
	m_ppMeshs[7]->SetOOBB(XMFLOAT3(0.0f, TreeHeight / 2, 0.0f), XMFLOAT3(TreeWidth / 2, TreeHeight / 2, TreeDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[7]->m_nStartInstance = 1052;
	m_ppMeshs[7]->m_nInstance = 700;

	for (int i = 1000; i < 1700; i++) {
		m_ppTrees[i] = new CGameObject();

		float fWidth = float(rand() % m_nWidth);
		float fDepth = float(rand() % m_nHeight);
		float fHeight = m_pTerrain->GetHeight(fWidth, fDepth);
		m_ppTrees[i]->SetPosition(fWidth, fHeight, fDepth);

		XMFLOAT3 xmfNorm = m_pTerrain->GetNormal(fWidth, fDepth);
		m_ppTrees[i]->MatchUp(xmfNorm);

		XMFLOAT4 Color{ 0.1f, -0.1f, 0.0f, 0.0f };
		m_ppTrees[i]->SetColor(Color);
		m_ppTrees[i]->SetMesh(m_ppMeshs[7]);
		m_ppTrees[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppTrees[i];
	}

	TreeWidth = 36.0f;
	TreeHeight = 108.0f;
	TreeDepth = 36.0f;

	m_ppMeshs[8] = new CTreeMesh(m_Vertex, m_Index, m_ppMeshs[6], TreeWidth, TreeHeight, TreeDepth);
	m_ppMeshs[8]->SetOOBB(XMFLOAT3(0.0f, TreeHeight / 2, 0.0f), XMFLOAT3(TreeWidth / 2, TreeHeight / 2, TreeDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[8]->m_nStartInstance = 1752;
	m_ppMeshs[8]->m_nInstance = 250;

	for (int i = 1700; i < 1950; i++) {
		m_ppTrees[i] = new CGameObject();

		float fWidth = float(rand() % m_nWidth);
		float fDepth = float(rand() % m_nHeight);
		float fHeight = m_pTerrain->GetHeight(fWidth, fDepth);
		m_ppTrees[i]->SetPosition(fWidth, fHeight, fDepth);

		XMFLOAT3 xmfNorm = m_pTerrain->GetNormal(fWidth, fDepth);
		m_ppTrees[i]->MatchUp(xmfNorm);

		XMFLOAT4 Color{ 0.0f, 0.05f, 0.0f, 0.0f };
		m_ppTrees[i]->SetColor(Color);
		m_ppTrees[i]->SetMesh(m_ppMeshs[8]);
		m_ppTrees[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppTrees[i];
	}

	TreeWidth = 108.0f;
	TreeHeight = 300.0f;
	TreeDepth = 108.0f;

	m_ppMeshs[9] = new CTreeMesh(m_Vertex, m_Index, m_ppMeshs[6], TreeWidth, TreeHeight, TreeDepth);
	m_ppMeshs[9]->SetOOBB(XMFLOAT3(0.0f, TreeHeight / 2, 0.0f), XMFLOAT3(TreeWidth / 2, TreeHeight / 2, TreeDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[9]->m_nStartInstance = 2002;
	m_ppMeshs[9]->m_nInstance = 50;

	for (int i = 1950; i < m_nTrees; i++) {
		m_ppTrees[i] = new CGameObject();

		float fWidth = float(rand() % m_nWidth);
		float fDepth = float(rand() % m_nHeight);
		float fHeight = m_pTerrain->GetHeight(fWidth, fDepth);
		m_ppTrees[i]->SetPosition(fWidth, fHeight, fDepth);

		XMFLOAT3 xmfNorm = m_pTerrain->GetNormal(fWidth, fDepth);
		m_ppTrees[i]->MatchUp(xmfNorm);

		XMFLOAT4 Color{ 0.0f, 0.0f, 0.0f, 0.0f };
		m_ppTrees[i]->SetColor(Color);
		m_ppTrees[i]->SetMesh(m_ppMeshs[9]);
		m_ppTrees[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppTrees[i];
	}

	//새
	float BirdWidth = 15.0f;
	float BirdHeight = 5.0f;
	float BirdDepth = 5.0f;

	m_ppMeshs[10] = new CCubeMeshDiffused(m_Vertex, m_Index, m_ppMeshs[1], BirdWidth, BirdHeight, BirdDepth);
	m_ppMeshs[10]->SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(BirdWidth / 2, BirdHeight / 2, BirdDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_ppMeshs[10]->m_nStartInstance = 2052;
	m_ppMeshs[10]->m_nInstance = 1000;

	m_nBirds = 1000;
	m_ppBirds = new CBirdObject*[m_nBirds];

	float MaxHeight = m_pTerrain->GetMaxHeight();
	for (int i = 0; i < m_nBirds; i++) {
		m_ppBirds[i] = new CBirdObject();
		m_ppBirds[i]->SetPosition(float(rand() % m_nWidth), float(rand() % int(MaxHeight)), float((rand()) % m_nHeight));
		m_ppBirds[i]->SetBound(float(m_nWidth), float(m_nHeight), 0.0f, MaxHeight+ 200.0f);
		m_ppBirds[i]->SetColor(RANDOM_COLOR);
		m_ppBirds[i]->SetMesh(m_ppMeshs[10]);
		m_ppBirds[i]->UpdateOOBB();
		m_ppObjects[ObjectCount++] = m_ppBirds[i];
	}

	//기둥
	m_ppMeshs[11] = new CCubeMeshTop(m_Vertex, m_Index, m_ppMeshs[1]);
	m_ppMeshs[11]->m_nInstance = 2;
	m_ppMeshs[11]->m_nStartInstance = 3052;

	m_nPillars = m_nRails / 10 * 2;	//10칸마다 좌우로 2개씩 배치한다.
	m_ppPillars = new CGameObject*[m_nPillars];

	for (int i = 0; i < m_nPillars; i++) {
		m_ppPillars[i] = new CGameObject();
		if(i&1)
			m_ppPillars[i]->SetColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f));
		else
			m_ppPillars[i]->SetColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f));
		m_ppObjects[ObjectCount++] = m_ppPillars[i];
	}

	//플레이어와 레일 초기상태 설정
	Renew();

	//쉐이더 생성
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	m_nShaders = 2;
	m_ppShaders = new CShader*[m_nShaders];

	m_ppShaders[0] = new CInstancingShader(m_ppObjects, m_nObjects, m_ppMeshs, m_nMeshs);
	m_ppShaders[0]->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_ppShaders[0]->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ppShaders[1] = new CUIShader();
	m_ppShaders[1]->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	//정점 버퍼 생성
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_Vertex.data(), sizeof(CDiffusedVertex) * m_Vertex.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	//정점 버퍼 뷰
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(CDiffusedVertex);
	m_d3dVertexBufferView.SizeInBytes = sizeof(CDiffusedVertex) * m_Vertex.size();

	//인덱스 버퍼 생성
	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_Index.data(), sizeof(UINT) * m_Index.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_Index.size();

	//벡터 초기화
	m_Vertex.clear();
	m_Index.clear();
}

void CScene::ReleaseObjects() {
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppObjects) {
		for (int i = 0; i < m_nObjects; i++)
			delete m_ppObjects[i];
		delete[] m_ppRails;
		delete[] m_ppTrees;
		delete[] m_ppBirds;
		delete[] m_ppObjects;
	}

	if (m_ppShaders) {
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			delete m_ppShaders[i];
		}
		delete[] m_ppShaders;
	}
}
void CScene::ReleaseUploadBuffers() {
	//정점 업로드용 버퍼 삭제
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	//인덱스 업로드용 버퍼 삭제
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}

ID3D12RootSignature *CScene::GetGraphicsRootSignature() {
	return m_pd3dGraphicsRootSignature;
}
ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice) {
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;
	D3D12_ROOT_PARAMETER pd3dRootParameters[1];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 16;
	pd3dRootParameters[0].Constants.ShaderRegister = 0;
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));

	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;
	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
	return(pd3dGraphicsRootSignature);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam)
{
	return false;
}
bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,	LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		if (lParam & 0x40000000) return false;
		switch (wParam)
		{
		case 'W':
			--m_Pitch;
			break;
		case 'S':
			++m_Pitch;
			break;
		case 'A':
			--m_Yaw;
			break;
		case 'D':
			++m_Yaw;
			break;
		case 'Q':
			++m_Roll;
			break;
		case 'E':
			--m_Roll;
			break;
		}
		break;

	case WM_KEYUP:
		switch (wParam)
		{
		case 'W':
			++m_Pitch;
			break;
		case 'S':
			--m_Pitch;
			break;
		case 'A':
			++m_Yaw;
			break;
		case 'D':
			--m_Yaw;
			break;
		case 'Q':
			--m_Roll;
			break;
		case 'E':
			++m_Roll;
			break;
		case '1':
			m_pPlayer->GetCamera()->SetOffset(XMFLOAT3(0.0f, 20.0f, +50.0f));
			break;
		case '2':
			m_pPlayer->GetCamera()->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			break;
		case '3':
			m_pPlayer->GetCamera()->SetOffset(XMFLOAT3(0.0f, 40.0f, -100.0f));
			break;
		}
		break;

	default:
		break;
	}
	return false;
}
bool CScene::ProcessInput()
{
	return false;
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	//Player가 다음 레일로 매 프레임 이동한다.
	float fDistance = fTimeElapsed * m_pPlayer->GetSpd();
	XMFLOAT3 Dest = Vector3::Subtract(m_ppRails[m_nDest]->GetPosition(), m_pPlayer->GetPosition());
	float fLength = Vector3::Length(Dest);
	fDistance = (fDistance > fLength) ? fLength : fDistance;
	XMFLOAT3 Go = Vector3::ScalarProduct(Vector3::Normalize(Dest), fDistance);
	m_pPlayer->Move(Go, false);

	//플레이어가 맵 밖으로 나가면 초기상태로 변환
	if (mapOut())
		Renew();

	//플레이어의 OOBB 업데이트
	m_pPlayer->OnPrepareRender();
	m_pPlayer->UpdateOOBB();

	//Player의 Pos가 레일의 OOBB에 포함되면 다음레일로 진행
	if (m_ppRails[m_nDest]->m_xmOOBB.Contains(XMLoadFloat3(&(m_pPlayer->GetPosition())))) {
		m_ppRails[m_nDest]->SetColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f));
		m_pPlayer->SetAxisFromMatrix(m_ppRails[m_nDest]->m_xmf4x4World);
		m_nDest = (m_nDest +1) % m_nRails;
		
		if (m_ppMeshs[1]->m_nInstance < m_nRails) {
			m_ppMeshs[1]->m_nInstance++;
			m_nEnd++;

			XMFLOAT3 NextPos = Vector3::Add((m_ppRails[m_nEnd-1]->GetPosition()), Vector3::ScalarProduct(m_ppRails[m_nEnd - 1]->GetLook(), 6.0f));
			m_ppRails[m_nEnd]->m_xmf4x4World = m_ppRails[m_nEnd - 1]->m_xmf4x4World;							// 마지막 레일의 월드변환행렬에 가장 앞에있는 레일의 월드변환행렬 대입(회전정보 획득)
			m_ppRails[m_nEnd]->Rotate(m_Degree*m_Pitch, m_Degree*m_Yaw, m_Degree*m_Roll);

			//다음 배치될 위치가 지형 아래라면 보정
			float TerrainHeight = m_pTerrain->GetHeight(NextPos.x, NextPos.z);
			if (NextPos.y < TerrainHeight) {
				NextPos.y = TerrainHeight;
				m_ppRails[m_nEnd]->MatchUp(m_pTerrain->GetNormal(NextPos.x, NextPos.z));
			}

			m_ppRails[m_nEnd]->SetPosition(NextPos);															// 계산된 위치로 마지막 레일을 배치한다.
			m_ppRails[m_nEnd]->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
			m_ppRails[m_nEnd]->UpdateOOBB();

			//매 10번째 레일마다 기둥을 배치한다
			if (m_nEnd % 10 == 0) {
				int PillarIndex = m_nEnd / 10 *2;
				m_ppMeshs[11]->m_nInstance = m_ppMeshs[1]->m_nInstance / 10 * 5;
				SetPillar(m_ppRails[m_nEnd], m_ppPillars[PillarIndex], XMFLOAT3(-10.0f, 0.0f, 0.0f));
				SetPillar(m_ppRails[m_nEnd], m_ppPillars[PillarIndex+1], XMFLOAT3(+10.0f, 0.0f, 0.0f));
			}
		}
		else {
			XMFLOAT3 NextPos = Vector3::Add((m_ppRails[m_nEnd]->GetPosition()), Vector3::ScalarProduct(m_ppRails[m_nEnd]->GetLook(), 6.0f));
			m_ppRails[m_nFirst]->m_xmf4x4World = m_ppRails[m_nEnd]->m_xmf4x4World;							// 마지막 레일의 월드변환행렬에 가장 앞에있는 레일의 월드변환행렬 대입(회전정보 획득)
			m_ppRails[m_nFirst]->Rotate(m_Degree*m_Pitch, m_Degree*m_Yaw, m_Degree*m_Roll);

			//다음 배치될 위치가 지형 아래라면 보정
			float TerrainHeight = m_pTerrain->GetHeight(NextPos.x, NextPos.z);
			if (NextPos.y < TerrainHeight) {
				NextPos.y = TerrainHeight;
				m_ppRails[m_nFirst]->MatchUp(m_pTerrain->GetNormal(NextPos.x, NextPos.z));
			}

			m_ppRails[m_nFirst]->SetPosition(NextPos);														// 계산된 위치로 마지막 레일을 배치한다.
			m_ppRails[m_nFirst]->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
			m_ppRails[m_nFirst]->UpdateOOBB();

			//매 10번째 레일마다 기둥을 배치한다
			if (m_nFirst % 10 == 0) {
				int PillarIndex = m_nFirst / 10 *2;
				SetPillar(m_ppRails[m_nFirst], m_ppPillars[PillarIndex], XMFLOAT3(-10.0f, 0.0f, 0.0f));
				SetPillar(m_ppRails[m_nFirst], m_ppPillars[PillarIndex+1], XMFLOAT3(+10.0f, 0.0f, 0.0f));
			}

			m_nFirst = (m_nFirst + 1) % m_nRails;
			m_nEnd = (m_nEnd + 1) % m_nRails;
		}
	}

	//새와의 충돌체크
	for (int i = 0; i < m_nBirds; i++) {
		m_ppBirds[i]->Animate(fTimeElapsed);

		//새와 지형의 충돌체크
		XMFLOAT3 Pos = m_ppBirds[i]->GetPosition();
		float Height = m_pTerrain->GetHeight(Pos.x, Pos.z);
		if (Pos.y < Height) {
			m_ppBirds[i]->SetDirection();
			Pos.y = Height;
			m_ppBirds[i]->SetPosition(Pos);
		}

		if(!m_pPlayer->GetStatus())
			if (m_ppBirds[i]->m_xmOOBB.Intersects(m_pPlayer->m_xmOOBB)) {
				m_ppBirds[i]->SetDirection();
				m_pPlayer->SetPower();
				if (--m_nLife == 0) {
					Renew();
				}
			}
	}

	//나무와의 충돌체크
	if (!m_pPlayer->GetStatus()) {
		for (int i = 0; i < m_nTrees; i++) {
			if (m_ppTrees[i]->m_xmOOBB.Intersects(m_pPlayer->m_xmOOBB)) {
				m_pPlayer->SetPower();
				if (--m_nLife == 0) {
					Renew();
				}
			}
		}
	}

	//카메라 수정 및 플레이어 액션
	m_pPlayer->Update(fTimeElapsed);
}
void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, D3D12_CPU_DESCRIPTOR_HANDLE *pHandle)
{
	m_pPlayer->OnPrepareRender();
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (pCamera) pCamera->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);

	//오브젝트 쉐이더 랜더
	((CInstancingShader*)m_ppShaders[0])->Render(pd3dCommandList, pCamera, pHandle, m_pPlayer->GetStatus());
	((CUIShader*)m_ppShaders[1])->Render(pd3dCommandList, pCamera, pHandle, m_nLife);
}

CPlayer* CScene::getPlayer() {
	return m_pPlayer;
}
bool CScene::mapOut() {
	XMFLOAT3 pos = m_pPlayer->GetPosition();

	if (pos.x < 0) return true;
	if (pos.x > m_nWidth) return true;
	if (pos.z < 0) return true;
	if (pos.z > m_nHeight) return true;

	return false;
}
void CScene::Renew() {
	//플레이어 재배치
	float fHeightOffset = 500.0f;

	m_nLife = 3;
	float fHeight = m_pTerrain->GetHeight(float(m_nWidth / 2), 0.0f);
	m_pPlayer->SetPosition(XMFLOAT3(float(m_nWidth / 2), float(fHeight+ fHeightOffset), 0.0f));

	//레일 재배치
	m_nFirst = 0;
	m_nDest = 0;
	m_nEnd = 4;

	m_ppMeshs[1]->m_nInstance = 5;
	for (int i = 0; i < 5; i++) {
		m_ppRails[i]->ClearMatrix();
		m_ppRails[i]->SetPosition(float(m_nWidth / 2), fHeight + fHeightOffset, float(i * 6));
		m_ppRails[i]->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		m_ppRails[i]->UpdateOOBB();
	}

	//기둥 재배치
	m_ppMeshs[11]->m_nInstance = 2;
	SetPillar(m_ppRails[0], m_ppPillars[0], XMFLOAT3(-10.0f, 0.0f, 0.0f));
	SetPillar(m_ppRails[0], m_ppPillars[1], XMFLOAT3(+10.0f, 0.0f, 0.0f));
}
void CScene::SetPillar(CGameObject *pRail, CGameObject *pPillar, XMFLOAT3 xmfOffset) {
	//오프셋에 따른 기둥 위치 설정
	XMFLOAT3 NewPos = Vector3::TransformCoord(xmfOffset, pRail->m_xmf4x4World);
	pPillar->SetPosition(NewPos);

	//기둥 높이 설정
	float fHeight = m_pTerrain->GetHeight(NewPos.x, NewPos.z);
	pPillar->m_xmf4x4World._22 = NewPos.y - fHeight + 5.0f;
}