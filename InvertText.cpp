#include "StdAfx.h"
#include "InvertText.h"

InvertText::InvertText(void)
{
	memDC = 0;
}

InvertText::InvertText(CString str, HDC hDC)//(char* str, HDC hDC)
{
	//int len = (int)strlen(str);
	memDC = ::CreateCompatibleDC(hDC);
	HFONT hfont = ::CreateFontA(-12, 0, 0, 0, 800, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");
	HFONT hOldFont = (HFONT)::SelectObject(hDC, hfont);
	RECT textRect;
	memset(&textRect, 0, sizeof(textRect));
	textRect.right = 100;
	textRect.bottom = ::DrawText(hDC, str, -1, &textRect, DT_NOCLIP)+10;
	width = textRect.right;
	height = textRect.bottom;
	hBmp = ::CreateCompatibleBitmap(hDC, width+10, height+10);
	::SelectObject(hDC, hfont);

	::SelectObject(memDC, hfont);
	::SelectObject(memDC, hBmp);
	::SetTextColor(memDC, RGB(255, 255, 255));
	::SetBkColor(memDC, RGB(0, 0, 0));
	::DrawText(memDC, str, -1, &textRect, DT_NOCLIP);
	::SelectObject(memDC, hOldFont);
}

void InvertText::display(HDC hDC, int x, int y)
{
	BitBlt(hDC, x, y, width, height, memDC, 0, 0, SRCINVERT);
	::SetROP2(hDC, R2_NOT); 
	//::Rectangle(hDC, x-1, y-1, x+1, y+1);
	::Ellipse(hDC, x-2, y-2, x+2, y+2);
	::SetROP2(hDC, R2_COPYPEN);
}

InvertText::~InvertText(void)
{
	if (memDC != 0)
	{
		DeleteObject(hBmp);
		DeleteDC(memDC);
	}
}
