#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <random>

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

#include <DirectXCollision.h>
#include <DXGIDebug.h>

#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;
using Microsoft::WRL::ComPtr;

#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#define FRAME_BUFFER_WIDTH 640
#define FRAME_BUFFER_HEIGHT 480

#define DegreeToRadian(x) float((x)*3.141592654f/180.0f)

namespace Matrix4x4{
	XMFLOAT4X4 Identity();
}

// -1.0 ~ 1.0
float random();

void normalize(XMFLOAT3& t);

void print(LPCTSTR pszStr, ...);

template <class T>
class Singleton{
public:
	static T& getInstance(){
		static T object;
		return object;
	} 
	static T* getPointer(){
		return &getInstance();
	} 
	static const T& getConst(){
		return getInstance();
	} 
protected: 
	Singleton(){
	} 
	~Singleton(){
	}
};

extern ID3D12Resource *CreateBufferResource(ID3D12Device *pd3dDevice,
	ID3D12GraphicsCommandList *pd3dCommandList, void *pData, UINT nBytes, D3D12_HEAP_TYPE
	d3dHeapType = D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATES d3dResourceStates =
	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ID3D12Resource **ppd3dUploadBuffer =
	NULL);