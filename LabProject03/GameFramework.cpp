﻿#include "stdafx.h"
#include "GameFramework.h"
#include <dxgidebug.h>

CGameFramework::CGameFramework() {
	m_pdxgiFactory = nullptr;
	m_pdxgiSwapChain = nullptr;
	m_pd3dDevice = nullptr;
	m_pd3dCommandAllocator = nullptr;
	m_pd3dCommandQueue = nullptr;
	m_pd3dPipelineState = nullptr;
	m_pd3dCommandList = nullptr;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dRenderTargetBuffers[i] = nullptr;
	m_pd3dRtvDescriptorHeap = nullptr;
	m_pd3dDepthStencilBuffer = nullptr;
	m_pd3dDsvDescriptorHeap = nullptr;
	m_nSwapChainBufferIndex = 0;
	m_hFenceEvent = nullptr;
	m_pd3dFence = nullptr;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;
	m_pScene = nullptr;
	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;
}

//다음 함수는 응용 프로그램이 실행되어 주 윈도우가 생성되면 호출된다는 것에 유의하라. 
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd) {
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	//Direct3D 디바이스, 명령 큐와 명령 리스트, 스왑 체인 등을 생성하는 함수를 호출한다. 
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateRenderTargetViews();
	CreateDepthStencilView();
	BuildObjects();
	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
	//렌더링할 게임 객체를 생성한다. 
	return(true);
}
void CGameFramework::OnDestroy() {
	WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다. 
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다. 
	::CloseHandle(m_hFenceEvent);
	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, nullptr);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();
#if defined(_DEBUG)
	IDXGIDebug1 *pdxgiDebug = nullptr;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

void CGameFramework::CreateSwapChain() {
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	//m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd,
	//	&dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1
	//		**)&m_pdxgiSwapChain);
	////스왑체인을 생성한다. 
	//m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	////“Alt+Enter” 키의 동작을 비활성화한다.
	//m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	////스왑체인의 현재 후면버퍼 인덱스를 저장한다. 



	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels -
		1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue,
		&dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}
void CGameFramework::CreateDirect3DDevice() {
	HRESULT hResult;
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = nullptr;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void
		**)&pd3dDebugController);
	if (pd3dDebugController) {
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}

	Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;

	typedef HRESULT(WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);

	HMODULE dxgidebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (dxgidebug) {
		auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
			reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));

		if (SUCCEEDED(dxgiGetDebugInterface(IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			/*dxgiInfoQueue.GetAddressOf()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);*/
		}
		ComPtr<IDXGIDebug> debug;
		if (SUCCEEDED(dxgiGetDebugInterface(IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		}
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);
	IDXGIAdapter1 *pd3dAdapter = nullptr;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i,
		&pd3dAdapter); i++) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}
	//모든 하드웨어 어댑터 대하여 특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성한다. 
	if (!pd3dAdapter) {
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	//특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스를 생성한다. 
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다. 
	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	memset(m_nFenceValues, 0, m_nSwapChainBuffers);
	//펜스를 생성하고 펜스 값을 0으로 설정한다.
	m_hFenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	/*펜스와 동기화를 위한 이벤트 객체를 생성한다(이벤트 객체의 초기값을 FALSE이다).
	이벤트가 실행되면(Signal) 이벤트의 값을 자동적으로 FALSE가 되도록 생성한다.*/
	//m_d3dViewport.TopLeftX = 0;
	//m_d3dViewport.TopLeftY = 0;
	//m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	//m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	//m_d3dViewport.MinDepth = 0.0f;
	//m_d3dViewport.MaxDepth = 1.0f;
	////뷰포트를 주 윈도우의 클라이언트 영역 전체로 설정한다.
	//m_d3dScissorRect = {0, 0, m_nWndClientWidth, m_nWndClientHeight};
	//씨저 사각형을 주 윈도우의 클라이언트 영역 전체로 설정한다.
	::gnCbvSrvUavDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	::gnCbvSrvUavDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	
	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList() {
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);
	//직접(Direct) 명령 큐를 생성한다. 
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);
	//직접(Direct) 명령 할당자를 생성한다. 
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void
			**)&m_pd3dCommandList);
	//직접(Direct) 명령 리스트를 생성한다. 
	hResult = m_pd3dCommandList->Close();
	//명령 리스트는 생성되면 열린(Open) 상태이므로 닫힌(Closed) 상태로 만든다.
}

// 전체화면으로 바꿈
void CGameFramework::ChangeSwapChainState() {
	WaitForGpuComplete();
	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, nullptr);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, nullptr);

	m_pd3dDepthStencilBuffer->Release();

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	//for(int i = 0; i<m_nSwapChainBuffers; i++) if(m_ppd3dSwapChainBackBuffers[i])
	//	m_ppd3dSwapChainBackBuffers[i]->Release();
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews();
	CreateDepthStencilView();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + 1; // RT Tex
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Render Target View
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	//렌더 타겟 서술자 힙(서술자의 개수는 스왑체인 버퍼의 개수)을 생성한다.

	m_pd3dRtvDescriptorHeap->SetName(L"CGameFramework::m_pd3dRtvDescriptorHeap");
	
	//렌더 타겟 서술자 힙의 원소의 크기를 저장한다. 
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // Depth Stencil View
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	//깊이-스텐실 서술자 힙(서술자의 개수는 1)을 생성한다.
	//깊이-스텐실 서술자 힙의 원소의 크기를 저장한다.

	////////////////////////////////////////////////////////////
	// 랜더텍스처 만들기
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0, 0, 1 };
	m_pColorRenderTex = new CTexture(2, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1, 1, 1);
	m_pColorRenderTex->CreateTexture(m_pd3dDevice, m_pd3dCommandList, 0, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	m_pColorRenderTex->SetRootParameterIndex(0, 17);
	m_pColorRenderTex->CreateTexture(m_pd3dDevice, m_pd3dCommandList, 1, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	
	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

	RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVDesc.Texture2D.MipSlice = 0;

	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	
	
	d3dDescriptorHeapDesc.NumDescriptors = 0 + 1; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvUavDescriptorHeap);
}

//스왑체인의 각 후면 버퍼에 대한 렌더 타겟 뷰를 생성한다.
void CGameFramework::CreateRenderTargetViews() {
	m_pcbMappedFrameworkInfo = new CB_FRAMEWORK_INFO;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto m_d3dSrvCPUDescriptorNextHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		wstring n(L"RenderTarget");
		m_ppd3dRenderTargetBuffers[i]->SetName((n+ to_wstring(i)).c_str());
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], nullptr, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += gnRtvDescriptorIncrementSize;
		//m_d3dSrvCPUDescriptorNextHandle.ptr += gnCbvSrvUavDescriptorIncrementSize;
	}

	////////////////////////////////////////////////////////////
	// 렌더텍스처 만들기 위함

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	d3dShaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_pColorRenderTex->GetResource(0)->SetName(L"ColorRenderTex");
	//m_pd3dDevice->CreateShaderResourceView(m_pColorRenderTex->GetResource(0), &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle); // 씬에 힙쓸거임
	m_pd3dDevice->CreateRenderTargetView(m_pColorRenderTex->GetResource(0), nullptr, d3dRtvCPUDescriptorHandle); //d3dRtvCPUDescriptorHandle의 2 인덱스에 컬러렌더텍스처 뷰 넣음

	m_pd3dCbvSrvUavDescriptorHeap->SetName(L"GameFramework::m_pd3dCbvSrvUavDescriptorHeap");// 이제 안씀
	//m_pColorRenderTex->SetSrvGpuDescriptorHandle(0, m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); 씬에 힙씀
}
void CGameFramework::CreateDepthStencilView() {
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);
	//깊이-스텐실 버퍼를 생성한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, nullptr, d3dDsvCPUDescriptorHandle);
	//깊이-스텐실 버퍼 뷰를 생성한다.
}

void CGameFramework::BuildObjects() {
	m_GameTimer.Reset();
	assert(SUCCEEDED(m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL)));
	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, nullptr);
	CAirplanePlayer *pAirplanePlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pScene->m_pPlayer = m_pPlayer = pAirplanePlayer;
	pAirplanePlayer->SetPosition(XMFLOAT3(300, 500, 300));
	m_pCamera = m_pPlayer->GetCamera();

	//m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 1, 1);
	//m_pTexture->CreateTexture(m_pd3dDevice, m_pd3dCommandList, 2, RESOURCE_TEXTURE2D, 1024, 1024, 1,1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, NULL);

	GetScene()->CreateShaderResourceViews(m_pd3dDevice, m_pColorRenderTex, 0);
	GetScene()->CreateUnorderedAccessView(m_pd3dDevice, m_pColorRenderTex, 1); //TODO 문제생길 가능성 있음 nIndex 1에서 0으로 바꿈

	m_pColorRenderTex->SetComputeSrvRootParameterIndex(0, 0, 0);
	m_pColorRenderTex->SetComputeUavRootParameterIndex(0, 1, 1);

	auto* pComputeShader = new CEdgeDetectionComputeShader();
	pComputeShader->CreateShader(m_pd3dDevice, GetScene()->GetComputeRootSignature(), (UINT)(FRAME_BUFFER_WIDTH / 32.0f + 0.5f), (UINT)(FRAME_BUFFER_HEIGHT / 32.0f + 0.5f), 1);

	m_vpComputeShaders.push_back(pComputeShader);

	auto rectMesh = new CTexturedRectMesh(m_pd3dDevice, m_pd3dCommandList, 2, 2, 0,0,0,0.1f);
	auto screenQuadObj = new CGameObject(1);
	auto screenMaterial = new CMaterial();
	screenMaterial->SetTexture(m_pColorRenderTex);
	screenQuadObj->SetMaterial(0, screenMaterial);
	screenQuadObj->SetMesh(rectMesh);
	m_pScreenShader = new CViewportShader();
	m_pScreenShader->m_ppObjects.push_back(screenQuadObj);
	m_pScreenShader->CreateShader(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());

	assert(SUCCEEDED(m_pd3dCommandList->Close()));
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();
	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
}
void CGameFramework::ReleaseObjects() {
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
	LPARAM lParam) {
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스가 눌려지면 마우스 픽킹을 하여 선택한 게임 객체를 찾는다. 
		m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다. 
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		m_pSelectedObject = nullptr;
		//마우스 캡쳐를 해제한다. 
		::ReleaseCapture();
		break;

	}
}
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM
	wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
			/*‘F1’ 키를 누르면 1인칭 카메라, ‘F2’ 키를 누르면 스페이스-쉽 카메라로 변경한다, ‘F3’ 키를 누르면 3인칭 카메라로 변경한다.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer) m_pCamera = m_pPlayer->ChangeCamera(((DWORD)wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_DELETE:
			break;
		case 'R':
			m_pcbMappedFrameworkInfo->m_nRenderMode = 0x00;
			break;
		case 'Y':
			m_pcbMappedFrameworkInfo->m_nRenderMode ^= DYNAMIC_TESSELLATION;
			break;
		case 'D':
			m_pcbMappedFrameworkInfo->m_nRenderMode ^= DEBUG_TESSELLATION;
			break;
		case 'W':
			::gbTerrainTessellationWireframe = !::gbTerrainTessellationWireframe;
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
	}
}
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID,
	WPARAM wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
			//“F9” 키가 눌려지면 윈도우 모드와 전체화면 모드의 전환을 처리한다. 
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}
void CGameFramework::ProcessInput() {
	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;
	if (::GetKeyboardState(pKeyBuffer)) {
		if (pKeyBuffer[VK_UP] & 0x80) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0x80) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0x80) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_RIGHT] & 0x80) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0x80) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0x80) dwDirection |= DIR_DOWN;
	}
	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다. 마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의
	메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다. 그러므로 마우스가 캡쳐된
	것은 마우스 버튼이 눌려진 상태를 의미한다. 마우스 버튼이 눌려진 상태에서 마우스를 좌우 또는 상하로 움직이면 플
	레이어를 x-축 또는 y-축으로 회전한다.*/
	if (::GetCapture() == m_hWnd) {
		::SetCursor(nullptr);
		::GetCursorPos(&ptCursorPos);
		//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다. 
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}
	//마우스 또는 키 입력이 있으면 플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f)) {
		if (m_pSelectedObject) {
			ProcessSelectedObject(dwDirection, cxDelta, cyDelta);
		} else {
			if (cxDelta || cyDelta) {
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다.*/
			if (dwDirection)
				m_pPlayer->Move(dwDirection, 5000.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}
void CGameFramework::AnimateObjects() {
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	/*auto a = dynamic_cast<CAirplanePlayer*>(m_pPlayer);
	float distance;
	CGameObject* obj = NULL;
	for (size_t i = 0; i < a->child.size(); i++) {
		obj = m_pScene->GetNearestObject(a->child[i]->GetPosition(), &distance);
		if (obj) {
			if (distance < 10) {
				delete a->child[i];
				a->child.erase(a->child.begin() + i);
				auto t = dynamic_cast<CExplosibleObject*>(obj);
				if(t) t->isExploed = true;
			}
		}
	}*/
}
void CGameFramework::WaitForGpuComplete() {
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame() {
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}
void CGameFramework::UpdateShaderVariables() {
	float fCurrentTime = m_GameTimer.GetTotalTime();
	float fElapsedTime = m_GameTimer.GetTimeElapsed();

	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 1, &fCurrentTime, 0);
	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 1, &fElapsedTime, 1);

	POINT ptCursorPos;
	::GetCursorPos(&ptCursorPos);
	::ScreenToClient(m_hWnd, &ptCursorPos);
	float fxCursorPos = (ptCursorPos.x < 0) ? 0.0f : float(ptCursorPos.x);
	float fyCursorPos = (ptCursorPos.y < 0) ? 0.0f : float(ptCursorPos.y);

	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 1, &fxCursorPos, 2);
	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 1, &fyCursorPos, 3);

	// screen size
	auto screen = XMFLOAT2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 2, &screen, 4);

	// render mode
	m_pd3dCommandList->SetGraphicsRoot32BitConstants(16, 1, &m_pcbMappedFrameworkInfo->m_nRenderMode, 6);
}
//#define _WITH_PLAYER_TOP
void CGameFramework::FrameAdvance() {
	m_GameTimer.Tick(60.0f);
	ProcessInput();
	AnimateObjects();
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, nullptr);
	
	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 렌더 텍스처 렌더타겟으로
	::SynchronizeResourceTransition(m_pd3dCommandList, m_pColorRenderTex->GetResource(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->
		GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (2 * gnRtvDescriptorIncrementSize);
	//d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * gnRtvDescriptorIncrementSize); 
#ifdef  RENDER_WIREFRAME
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, nullptr);
#endif
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->
		GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
	                                         D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);


	if (m_pScene){
		m_pScene->PrepareRender(m_pd3dCommandList);
		UpdateShaderVariables();


		m_pScene->Render(m_pd3dCommandList, m_pCamera);
	}
	//3인칭 카메라일 때 플레이어가 항상 보이도록 렌더링한다. 
#ifdef _WITH_PLAYER_TOP
	//렌더 타겟은 그대로 두고 깊이 버퍼를 1.0으로 지우고 플레이어를 렌더링하면 플레이어는 무조건 그려질 것이다. 
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
	D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	//3인칭 카메라일 때 플레이어를 렌더링한다. 
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	////////////////////////////////////////////////////////

	// 렌더 텍스처 랜더 타겟에서 텍스처로
	::SynchronizeResourceTransition(m_pd3dCommandList, m_pColorRenderTex->GetResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * gnRtvDescriptorIncrementSize);

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	// 뷰포트 설정
	D3D12_VIEWPORT m_d3dViewport = {0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f};
	D3D12_RECT m_d3dScissorRect = {0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT};
	m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	// 포스트 프로세싱 시작
	//::SynchronizeResourceTransition(m_pd3dCommandList, m_pColorRenderTex->GetResource(1), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//m_pd3dCommandList->SetDescriptorHeaps(1, &GetScene()->m_pd3dCbvSrvUavDescriptorHeap);
	//GetScene()->SetComputeRootSignature(m_pd3dCommandList);
	//m_pColorRenderTex->UpdateComputeShaderVariables(m_pd3dCommandList);

	//for (size_t i = 0; i < m_vpComputeShaders.size(); i++)
	//{
	//	m_vpComputeShaders[i]->Dispatch(m_pd3dCommandList);
	//}

	//::SynchronizeResourceTransition(m_pd3dCommandList, m_pColorRenderTex->GetResource(1), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	// 쿼드 렌더 시작
	//m_pScene->PrepareRender(m_pd3dCommandList);
	//m_pd3dCommandList->SetDescriptorHeaps(1, &GetScene()->m_pd3dCbvSrvUavDescriptorHeap);
	//auto GPUDescriptorHandleForHeapStart = GetScene()->m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//m_pd3dCommandList->SetGraphicsRootDescriptorTable(17, GPUDescriptorHandleForHeapStart);

	//m_pScreenShader->OnPrepare(m_pd3dCommandList, 0);
	//m_pScreenShader->m_ppObjects[0]->m_ppMaterials[0]->m_pTexture->UpdateShaderVariables(m_pd3dCommandList);
	//m_pScreenShader->m_ppObjects[0]->m_pMesh->Render(m_pd3dCommandList, 0);

	////////////////////////////////////////////////////////

	SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex],
	                              D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = {m_pd3dCommandList};
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();
	m_pdxgiSwapChain->Present(0, 0);
	MoveToNextFrame();
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float
	cyDelta) {
	//픽킹으로 선택한 게임 객체가 있으면 키보드를 누르거나 마우스를 움직이면 게임 개체를 이동 또는 회전한다. 
	if (dwDirection != 0)
		{
			if (dwDirection & DIR_FORWARD) m_pSelectedObject->MoveForward(+1.0f);
			if (dwDirection & DIR_BACKWARD) m_pSelectedObject->MoveForward(-1.0f);
			if (dwDirection & DIR_LEFT) m_pSelectedObject->MoveStrafe(+1.0f);
			if (dwDirection & DIR_RIGHT) m_pSelectedObject->MoveStrafe(-1.0f);
			if (dwDirection & DIR_UP) m_pSelectedObject->MoveUp(+1.0f);
			if (dwDirection & DIR_DOWN) m_pSelectedObject->MoveUp(-1.0f);
		} else if ((cxDelta != 0.0f) || (cyDelta != 0.0f)) {
			m_pSelectedObject->Rotate(cyDelta, cxDelta, 0.0f);
		}
}