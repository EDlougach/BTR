#pragma once

class InvertText
{
	int width, height;
	HDC memDC;
	HBITMAP hBmp;
public:
	InvertText(void);
	//InvertText(char* str, HDC hDC);
	InvertText(CString str, HDC hDC);
	void display(HDC hDC, int x, int y);
public:
	~InvertText(void);
};
