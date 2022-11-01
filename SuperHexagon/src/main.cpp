#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <Windows.h>
#include <iostream>

constexpr float crop = 0.1;

bool sortContourArea(std::vector<cv::Point> a, std::vector<cv::Point> b)
{
	return cv::contourArea(a) > cv::contourArea(b);
}

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

	int key = 0;
	while (key != 27)
	{
		//Capture images
		cv::Mat src = captureScreen(hwndWindow), hsvImg;
		cv::Mat grayImg = cv::Mat::zeros(src.size(), CV_32FC(1));
		
		// Extract features
		cv::cvtColor(src, hsvImg, cv::COLOR_BGR2HSV);
		cv::Mat hsvChannels[3];
		cv::split(hsvImg, hsvChannels);		
		cv::threshold(hsvChannels[2], grayImg, 120, 255, cv::THRESH_BINARY);

		//Crop middle
		cv::Rect centerRect(
			grayImg.size().width / 2 - grayImg.size().width * 0.15,
			grayImg.size().height / 2 - grayImg.size().height * 0.15,
			grayImg.size().width * 0.3,
			grayImg.size().height * 0.3
		);

		cv::Mat centerImg = grayImg(centerRect);
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(centerImg, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		
		if (contours.size() >= 2)
		{
			std::vector<cv::Point> approx;
			std::sort(contours.begin(), contours.end(), sortContourArea);

			float epsilon = 0.04 * cv::arcLength(contours[1], true);
			cv::approxPolyDP(contours[1], approx, epsilon, true);

			for (auto& p : approx)
			{
				p.x += centerRect.x;
				p.y += centerRect.y;
			}

			cv::drawContours(src, std::vector<std::vector<cv::Point>>(1, approx), -1, { 0,255,0 }, 4);
		}

		//Display window
		cv::imshow("Super Hexagon | CV", src);

		key = cv::waitKey(1);
	}
}