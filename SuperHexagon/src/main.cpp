#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <Windows.h>
#include <iostream>

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
		
		//Find middle shape
		// 
		//Crop middle
		cv::Rect centerRect(
			grayImg.size().width / 2 - grayImg.size().width * 0.15,
			grayImg.size().height / 2 - grayImg.size().height * 0.15,
			grayImg.size().width * 0.3,
			grayImg.size().height * 0.3
			);

		cv::Mat centerImg = grayImg(centerRect), foundMat = cv::Mat::zeros({centerRect.width, centerRect.height}, CV_8UC1);
		int maxRad = min(centerImg.size().width, centerImg.size().height);
		for (int i = 5; i < maxRad / 2; i++)
		{
			cv::Mat circleMask = cv::Mat::zeros(centerImg.size(), CV_8UC1);
			cv::circle(circleMask,{centerImg.size().width / 2 - 1, centerImg.size().height / 2}, i, cv::Scalar(255, 255, 255));
			centerImg.copyTo(foundMat, circleMask);

			if (cv::countNonZero(foundMat) > 100)
				break;
		}

		//Get edges
		cv::blur(foundMat, foundMat, { 3,3 });
		cv::Canny(foundMat, foundMat, 125, 125 * 2, 3);
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 4,4 });
		cv::dilate(foundMat, foundMat, kernel);

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(foundMat, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		//Draw the edges
		cv::Mat test = src(centerRect);
		for (auto& i : contours)
		{
			cv::Rect bounds = cv::boundingRect(i);
			cv::circle(test, {bounds.x + bounds.width/2, bounds.y + bounds.height/2}, 1, {0,255,0});
			cv::line(src, { centerRect.x + bounds.x + bounds.width / 2, centerRect.y + bounds.y + bounds.height / 2 }, { src.size().width / 2 - 1, src.size().height / 2 }, { 0, 255, 0 });
		}

		//Display window
		cv::imshow("Super Hexagon | CV", src);
		cv::imshow("Center", test);


		key = cv::waitKey(1);
	}
}