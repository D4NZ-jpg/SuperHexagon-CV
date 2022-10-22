#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <Windows.h>
#include <iostream>
#include <string>

constexpr const char* cvWindowName = "Super Hexagon | CV";
constexpr float crop = 0.1;

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
	height = srcheight/3;
	width = srcwidth/3;

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

	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, srcwidth * crop, 
		srcheight * crop, srcwidth * (1 - crop * 2), srcheight * (1 - crop * 2), SRCCOPY);
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
		//Capture images
		cv::Mat src = captureScreen(hwndWindow), grayImg;
		
		// Extract features
		cv::cvtColor(src, grayImg, cv::COLOR_BGR2HSV);
		cv::Mat hsvChannels[3];
		cv::split(grayImg, hsvChannels);
		grayImg = hsvChannels[2];
		cv::threshold(grayImg, grayImg, 120, 255, 0);

		//Display window
		cv::imshow(cvWindowName, grayImg);

		key = cv::waitKey(1);
	}
}