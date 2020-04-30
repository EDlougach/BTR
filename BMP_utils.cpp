#include "StdAfx.h"
#include "BMP_utils.h"
#include <stdlib.h>

/* This function is from book
 * "Windows Graphics programming" by Feng Yuan
 * ISBN 5-318-00297-8
 */
BITMAPINFO *BitmapToDIB(HPALETTE hPal, HBITMAP hBmp, WORD nBitCount, int nCompression, int &nInfoSize, int &nTotalSize)
{
	typedef struct {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD		bmiColors[256+3];
	} DIBINFO;

	BITMAP ddbinfo;
	DIBINFO dibinfo;

	if (GetObject(hBmp, sizeof(BITMAP), &ddbinfo)==0)
		return NULL;
	memset(&dibinfo, 0, sizeof(dibinfo));
	dibinfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	dibinfo.bmiHeader.biWidth       = ddbinfo.bmWidth;
	dibinfo.bmiHeader.biHeight      = ddbinfo.bmHeight;
	dibinfo.bmiHeader.biPlanes      = 1;
	dibinfo.bmiHeader.biBitCount    = nBitCount;
	dibinfo.bmiHeader.biCompression = nCompression;

	HDC hDC = GetDC(NULL);
	HGDIOBJ hpalOld;

	if (hPal)
		hpalOld = SelectPalette(hDC, hPal, FALSE);
	else
		hpalOld = NULL;

	GetDIBits(hDC, hBmp, 0, ddbinfo.bmHeight, NULL, (BITMAPINFO*) &dibinfo, DIB_RGB_COLORS);

	nInfoSize  = sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*dibinfo.bmiHeader.biClrUsed;
	nTotalSize = nInfoSize+dibinfo.bmiHeader.biBitCount*dibinfo.bmiHeader.biHeight*dibinfo.bmiHeader.biWidth;

	BYTE *pDIB = new BYTE[nTotalSize];

	if (pDIB)
	{
		memcpy(pDIB, &dibinfo, nInfoSize);
		if (ddbinfo.bmHeight != GetDIBits(hDC, hBmp, 0, ddbinfo.bmHeight, pDIB+nInfoSize,
			(BITMAPINFO*) pDIB, DIB_RGB_COLORS) )
		{
			delete [] pDIB;
			pDIB = NULL;
		}
	}

	if (hpalOld)
		SelectObject(hDC, hpalOld);
	ReleaseDC(NULL, hDC);
	return (BITMAPINFO *) pDIB;
}

bool SaveBitmap(HBITMAP h_bmp, CString str)
{
	BITMAPFILEHEADER bmp_file_hdr;
	int nInfoSize, nTotalSize;
	BITMAPINFO* bi = BitmapToDIB(NULL, h_bmp, 32, BI_RGB, nInfoSize, nTotalSize);
	bmp_file_hdr.bfType = 0x4d42; // "BM" - signature of bmp file
	bmp_file_hdr.bfSize = sizeof(BITMAPFILEHEADER)+nTotalSize;
	bmp_file_hdr.bfReserved1 = bmp_file_hdr.bfReserved2 = 0;
	bmp_file_hdr.bfOffBits = sizeof(BITMAPFILEHEADER)+nInfoSize;
	DWORD bytes_written;
	HANDLE file = CreateFile((LPCTSTR)str, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, NULL, NULL);
	WriteFile(file, &bmp_file_hdr, sizeof(BITMAPFILEHEADER), &bytes_written, NULL);
	WriteFile(file, bi, nTotalSize, &bytes_written, NULL);
	CloseHandle(file);
	return true;
}

HBITMAP CaptureWindow(HWND hWnd)
{
	RECT wnd;

	if (!GetWindowRect(hWnd, &wnd)) return NULL;

	HDC hDC = GetWindowDC(hWnd);

	HBITMAP hBmp = CreateCompatibleBitmap(hDC, wnd.right - wnd.left, wnd.bottom - wnd.top);

	if (hBmp)
	{
		 HDC hMemDC = CreateCompatibleDC(hDC);
		 HGDIOBJ hOld = SelectObject(hMemDC, hBmp);

		 BitBlt(hMemDC, 0, 0, wnd.right - wnd.left, wnd.bottom - wnd.top, hDC, 0, 0, SRCCOPY);

		 SelectObject(hMemDC, hOld);
		 DeleteObject(hMemDC);
	}
	ReleaseDC(hWnd, hDC);

	return hBmp;
}