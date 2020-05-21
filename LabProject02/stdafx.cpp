#include "stdafx.h"

namespace Matrix4x4 {
	XMFLOAT4X4 Identity() {
		XMFLOAT4X4 transFloat4x4;
		XMStoreFloat4x4(&transFloat4x4, XMMatrixIdentity());
		return transFloat4x4;
	}
}

float random(){
	return (float)rand()/(float)RAND_MAX*2.0f-1.0f;
}

void normalize(XMFLOAT3& t){
	auto direcVec = XMLoadFloat3(&t);
	direcVec = XMVector3Normalize(direcVec);
	XMStoreFloat3(&t, direcVec);
}

void print(LPCTSTR pszStr, ...){
#ifdef _DEBUG
	TCHAR szMsg[256];
	va_list args;
	va_start(args, pszStr);
	_vstprintf_s(szMsg, 256, pszStr, args);
	OutputDebugString(szMsg);
#endif   
}