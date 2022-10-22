#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <string>

constexpr const char* cvWindowName = "Super Hexagon | CV";

cv::Mat captureScreen(HWND hwnd)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	cv::Mat src;
	BITMAPINFOHEADER bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowSize;
	GetClientRect(hwnd, &windowSize);

	srcheight = windowSize.bottom;
	srcwidth = windowSize.right;
	height = srcheight/5;
	width = srcwidth/5;

	src.create(height, width, CV_8UC4);

	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height; 
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	SelectObject(hwindowCompatibleDC, hbwindow);

	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY);
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);
	
	return src;
}

int main()
{
	std::string windowName = "Super Hexagon";
	std::wstring stemp = std::wstring(windowName.begin(), windowName.end());
	HWND hwndWindow = FindWindow(NULL, stemp.c_str());

	cv::namedWindow(cvWindowName);
	int key = 0;

	while (key != 27)
	{
		cv::Mat src = captureScreen(hwndWindow);
		cv::imshow(cvWindowName, src);
		key = cv::waitKey(1);
	}
}