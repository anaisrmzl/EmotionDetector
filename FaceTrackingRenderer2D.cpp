#include "FaceTrackingRenderer2D.h"
#include "FaceTrackingUtilities.h"
#include "pxccapture.h"
#include <string.h>
#include <windows.h>
#include "resource.h"
#include <vector>
//#include "PicControl.h"
//#include "main.cpp"
//#include <afxwin.h>

//CStatic*  m_picture;
//HWND m_picture = GetDlgItem(dialogWindow, IDC_PANEL);
std::wstring bnames[13] = {(L"Doubt"), (L"Exclamation"), (L"Neutral")};
std::wstring enames[13] = {(L"Wink"), (L"Closed Eyes"), (L"Opened Eyes")};
std::wstring mnames[7] = {(L"Serious"), (L"Surprised"), (L"Content"), (L"Happy"), (L"Euforic"), (L"Flirty"), (L"Goofy")};
//picture.SetBitmap((HBITMAP)LoadImage(0,_T("C:\\1.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE));
int value [16];
double mrules[7];
double erules[3];
double brules[3];

std::wstring mouthresult;

double smiles=0;
double smilec=0;
double smileh=0;
double kissl=0;
double kissm=0;
double mouthc=0;
double moutho=0;
double tonguei=0;
double tongueo=0;

double eyesresult=0;

double righteo=0;
double rightec=0;
double lefteo=0;
double leftec=0;

double browsresult=0;

double rbrowm=0;
double rbrowl=0;
double lbrowm=0;
double lbrowl=0;


FaceTrackingRenderer2D::~FaceTrackingRenderer2D()
{
}

FaceTrackingRenderer2D::FaceTrackingRenderer2D(HWND window) : FaceTrackingRenderer(window)
{
}

void FaceTrackingRenderer2D::DrawGraphics(PXCFaceData* faceOutput)
{
	assert(faceOutput != NULL);
	if (!m_bitmap) return;

	const int numFaces = faceOutput->QueryNumberOfDetectedFaces();
	for (int i = 0; i < numFaces; ++i) 
	{
		PXCFaceData::Face* trackedFace = faceOutput->QueryFaceByIndex(i);		
		assert(trackedFace != NULL);
		if (FaceTrackingUtilities::IsModuleSelected(m_window, IDC_LOCATION) && trackedFace->QueryDetection() != NULL)
			DrawLocation(trackedFace);
		if (FaceTrackingUtilities::IsModuleSelected(m_window, IDC_LANDMARK) && trackedFace->QueryLandmarks() != NULL) 
			DrawLandmark(trackedFace);
		if (FaceTrackingUtilities::IsModuleSelected(m_window, IDC_POSE) || FaceTrackingUtilities::IsModuleSelected(m_window, IDC_PULSE))
			DrawPoseAndPulse(trackedFace, i);
		if (FaceTrackingUtilities::IsModuleSelected(m_window, IDC_EXPRESSIONS) && trackedFace->QueryExpressions() != NULL)
			DrawExpressions(trackedFace, i);
		if (FaceTrackingUtilities::IsModuleSelected(m_window, IDC_RECOGNITION) && trackedFace->QueryRecognition() != NULL)
			DrawRecognition(trackedFace, i);
	}
}

void FaceTrackingRenderer2D::DrawBitmap(PXCCapture::Sample* sample)
{
	if (m_bitmap) 
	{
		DeleteObject(m_bitmap);
		m_bitmap = 0;
	}

	PXCImage* image = sample->color;

	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;
	if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data) >= PXC_STATUS_NO_ERROR)
	{
		HWND hwndPanel = GetDlgItem(m_window, IDC_PANEL);
		HDC dc = GetDC(hwndPanel);
		BITMAPINFO binfo;
		memset(&binfo, 0, sizeof(binfo));
		binfo.bmiHeader.biWidth = data.pitches[0]/4;
		binfo.bmiHeader.biHeight = - (int)info.height;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biCompression = BI_RGB;
		Sleep(1);
		m_bitmap = CreateDIBitmap(dc, &binfo.bmiHeader, CBM_INIT, data.planes[0], &binfo, DIB_RGB_COLORS);

		ReleaseDC(hwndPanel, dc);
		image->ReleaseAccess(&data);
	}
}

void FaceTrackingRenderer2D::DrawRecognition(PXCFaceData::Face* trackedFace, const int faceId)
{
	PXCFaceData::RecognitionData* recognitionData = trackedFace->QueryRecognition();
	if(recognitionData == NULL)
		return;

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);

	if (!dc1)
	{
		return;
	}
	HDC dc2 = CreateCompatibleDC(dc1);
	if(!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap; 
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HFONT hFont = CreateFont(FaceTrackingUtilities::TextHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"MONOSPACE");
	SelectObject(dc2, hFont);

	WCHAR line1[64];
	int recognitionID = recognitionData->QueryUserID();
	if(recognitionID != -1)
	{
		swprintf_s<sizeof(line1) / sizeof(pxcCHAR)>(line1, L"Registered ID: %d",recognitionID);
	}
	else
	{
		swprintf_s<sizeof(line1) / sizeof(pxcCHAR)>(line1, L"Not Registered");
	}
	PXCRectI32 rect;
	memset(&rect, 0, sizeof(rect));
	int yStartingPosition;
	if (trackedFace->QueryDetection())
	{
		SetBkMode(dc2, TRANSPARENT);
		trackedFace->QueryDetection()->QueryBoundingRect(&rect);
		yStartingPosition = rect.y;
	}	
	else
	{		
		const int yBasePosition = bitmap.bmHeight - FaceTrackingUtilities::TextHeight;
		yStartingPosition = yBasePosition - faceId * FaceTrackingUtilities::TextHeight;
		WCHAR userLine[64];
		swprintf_s<sizeof(userLine) / sizeof(pxcCHAR)>(userLine, L" User: %d", faceId);
		wcscat_s(line1, userLine);
	}
	SIZE textSize;
	GetTextExtentPoint32(dc2, line1, std::char_traits<wchar_t>::length(line1), &textSize);
	int x = rect.x + rect.w + 1;
	if(x + textSize.cx > bitmap.bmWidth)
		x = rect.x - 1 - textSize.cx;

	TextOut(dc2, x, yStartingPosition, line1, std::char_traits<wchar_t>::length(line1));

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	DeleteObject(hFont);
}

void FaceTrackingRenderer2D::DrawExpressions(PXCFaceData::Face* trackedFace, const int faceId)
{
	PXCFaceData::ExpressionsData* expressionsData = trackedFace->QueryExpressions();
	if (!expressionsData)
		return;

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);
	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);
	BITMAP bitmap; 
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	const int maxColumnDisplayedFaces = 5;
	const int widthColumnMargin = 570;
	const int rowMargin = FaceTrackingUtilities::TextHeight;
	const int yStartingPosition = faceId % maxColumnDisplayedFaces * m_expressionMap.size() * FaceTrackingUtilities::TextHeight;
	const int xStartingPosition = widthColumnMargin * (faceId / maxColumnDisplayedFaces);

	WCHAR tempLine[200];
	int yPosition = yStartingPosition;
	swprintf_s<sizeof(tempLine) / sizeof(pxcCHAR)> (tempLine, L"ID: %d", trackedFace->QueryUserID());
	TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));
	yPosition += rowMargin;

	int count=0;
	for (auto expressionIter = m_expressionMap.begin(); expressionIter != m_expressionMap.end(); expressionIter++)
	{
		PXCFaceData::ExpressionsData::FaceExpressionResult expressionResult;
		if (expressionsData->QueryExpression(expressionIter->first, &expressionResult))
		{
			
			int intensity = expressionResult.intensity;
			std::wstring expressionName = expressionIter->second;
			swprintf_s<sizeof(tempLine) / sizeof(WCHAR)> (tempLine, L"%s = %d", expressionName.c_str(), intensity);
			TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));
			
			value[count]=intensity;
			yPosition += rowMargin;
			count++;
		}
		//--------------------------------------------------------------------------------------------------------
		////////////Author of this section: Anaïs Ramírez
		////////////December 10 2015
		//Membership Function -> Smile - serious
				if(value[4] <= 10){
					smiles=1;	
				}
				else if(value[4] > 10 && value[4] < 30){
					smiles=(((value[4])*-1.0)/20.0) + (3.0/2.0);
				}
				else if(value[4] >= 30){
					smiles=0;
				}
		//Membership Function -> Smile - content
				if(value[4] <= 20 || value[4] >= 80){
					smilec=0;	
				}
				else if(value[4] > 20 && value[4] < 50){
					smilec=((value[4])/30.0) - (2.0/3.0);
				}
				else if(value[4] > 50 && value[4] < 80){
					smilec=((value[4])/(-30.0)) + (8.0/3.0);
				}
		//Membership Function -> Smile - happy
				if(value[4] <= 60){
					smileh=0;	
				}
				else if(value[4] > 60 && value[4] < 80){
					smileh=((value[4])/20.0) - (3.0);
				}
				else if(value[4] >= 80){
					smileh=1;
				}
		//Membership Function -> kiss: little + big
				if(value[5] <= 30){
					kissl=1;
					kissm=0;
				}
				else if(value[5] > 30 && value[5] < 70){
					kissl=(((value[5])*-1.0)/40.0) + (7.0/4.0);
					kissm=((value[5])/40.0) - (3.0/4.0);

				}
				else if(value[5] >= 70){
					kissl=0;
					kissm=1;
				}
		//Membership Function -> tongue: inside + outside
				if(value[13] <= 30){
					tonguei=1;
					tongueo=0;
				}
				else if(value[13] > 30 && value[13] < 70){
					tonguei=(((value[5])*-1.0)/40.0) + (7.0/4.0);
					tongueo=((value[5])/40.0) - (3.0/4.0);

				}
				else if(value[13] >= 70){
					tonguei=0;
					tongueo=1;
				}
		//Membership Function -> mouth: closed
				if(value[6] <= 10){
					 mouthc=1;
				}
				else if(value[6] > 10 && value[6] < 30){
					mouthc=(((value[6])*-1.0)/20.0) + (3.0/2.0);
				}
				else if(value[6] >= 30){
					mouthc=0;
				}
		//Membership Function -> mouth: opened
				if(value[6] <= 20){
					 moutho=0;
				}
				else if(value[6] > 20 && value[6] < 40){
					moutho=((value[6])/20.0) - (1.0);
				}
				else if(value[6] >= 40){
					moutho=1;
				}
		///Rules MOUTH
				mrules[0] = min((min((min(smiles, mouthc)),kissl)), tonguei);
				mrules[1] = min((min(smiles, moutho)), tonguei);
				mrules[2] = min((min(smilec, mouthc)), tonguei); 
				mrules[3] = min((min(smileh, mouthc)), tonguei);
				mrules[4] = min((min(smileh, moutho)), tonguei);
				mrules[5] = min((min(mouthc, kissm)), tonguei);
				mrules[6] = max((min((min((min(smiles, mouthc)),kissl)), smileh)), tongueo);
		
		//--------------------------------------------------------------------------------------------
		//Membership Function -> Right eye: closed and open
				if(value[7] <= 30){
					righteo=1;
					rightec=0;
				}
				else if(value[7] > 30 && value[7] < 70){
					righteo=(((value[7])*-1.0)/40.0) + (7.0/4.0);
					rightec=((value[7])/40.0) - (3.0/4.0);

				}
				else if(value[7] >= 70){
					righteo=0;
					rightec=1;
				}
		//Membership Function -> Left eye: closed and open
				if(value[8] <= 30){
					lefteo=1;
					leftec=0;
				}
				else if(value[8] > 30 && value[8] < 70){
					lefteo=(((value[8])*-1.0)/40.0) + (7.0/4.0);
					leftec=((value[8])/40.0) - (3.0/4.0);

				}
				else if(value[8] >= 70){
					lefteo=0;
					leftec=1;
				}

	    ///Rules EYES
				erules[0] = max((min(rightec, lefteo)), min(righteo, leftec) );
				erules[1] = min(rightec, leftec);
				erules[2] = min(righteo, lefteo);

		//-------------------------------------------------------------------------------------------
		//Membership Function -> Right Brow: less much
				if(value[1] <= 30){
					rbrowl=1;
					rbrowm=0;
				}
				else if(value[1] > 30 && value[1] < 70){
					rbrowl=(((value[1])*-1.0)/40.0) + (7.0/4.0);
					rbrowm=((value[1])/40.0) - (3.0/4.0);

				}
				else if(value[1] >= 70){
					rbrowl=0;
					rbrowm=1;
				}
		//Membership Function -> Left brow: less much
				if(value[0] <= 30){
					lbrowl=1;
					lbrowm=0;
				}
				else if(value[0] > 30 && value[0] < 70){
					lbrowl=(((value[0])*-1.0)/40.0) + (7.0/4.0);
					lbrowm=((value[0])/40.0) - (3.0/4.0);

				}
				else if(value[0] >= 70){
					lbrowl=0;
					lbrowm=1;
				}

	    ///Rules BROWS
				brules[0] = max((min(rbrowm, lbrowl)), min(lbrowl, lbrowm) );
				brules[1] = min(rbrowm, lbrowm);
				brules[2] = min(rbrowl, lbrowl);


		//In this section I check wich of my outputs(rules) is the highest for each section and I print the one that is higher.
		//mouth rules ->
				double temp = -1.0;
				int pos = -1.0;
				for(int i=0; i<=6; i++){
					if(temp < mrules[i]){
						temp = mrules[i];
						pos=i;
					}
				}

		swprintf_s<sizeof(tempLine) / sizeof(WCHAR)> (tempLine, L"%s = %f", mnames[pos].c_str(), temp);
		TextOut(dc2, (xStartingPosition + 475), (yStartingPosition+340), tempLine, std::char_traits<wchar_t>::length(tempLine));

		//eye rules ->
		double temp2 = -1.0;
				int pos2 = -1.0;
				for(int i=0; i<=2; i++){
					if(temp2 < erules[i]){
						temp2 = erules[i];
						pos2=i;
					}
				}

		swprintf_s<sizeof(tempLine) / sizeof(WCHAR)> (tempLine, L"%s = %f", enames[pos2].c_str(), temp2);
		TextOut(dc2, (xStartingPosition + 475), (yStartingPosition + 320), tempLine, std::char_traits<wchar_t>::length(tempLine));

		//brow rules ->
		double temp3 = -1.0;
				int pos3 = -1.0;
				for(int i=0; i<=2; i++){
					if(temp3 < brules[i]){
						temp3 = brules[i];
						pos3=i;
					}
				}

		swprintf_s<sizeof(tempLine) / sizeof(WCHAR)> (tempLine, L"%s = %f", bnames[pos3].c_str(), temp3);
		TextOut(dc2, (xStartingPosition + 475), (yStartingPosition + 300), tempLine, std::char_traits<wchar_t>::length(tempLine));
		///End of section
		//--------------------------------------------------------------------------------------------------------------------------------
	}
	
	DeleteObject(cyan);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void FaceTrackingRenderer2D::DrawPoseAndPulse(PXCFaceData::Face* trackedFace, const int faceId)
{
	const PXCFaceData::PoseData* poseData = trackedFace->QueryPose();
	pxcBool poseAnglesExist;
	PXCFaceData::PoseEulerAngles angles;

	if (poseData == NULL) 
		poseAnglesExist = 0;
	else
		poseAnglesExist = poseData->QueryPoseAngles(&angles);

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);
	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);
	BITMAP bitmap; 
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);
	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, cyan);

	const int maxColumnDisplayedFaces = 5;
	const int widthColumnMargin = 570;
	const int rowMargin = FaceTrackingUtilities::TextHeight;
	const int yStartingPosition = faceId % maxColumnDisplayedFaces * 6 * FaceTrackingUtilities::TextHeight; 
	const int xStartingPosition = bitmap.bmWidth - 64 - - widthColumnMargin * (faceId / maxColumnDisplayedFaces);

	WCHAR tempLine[64];
	int yPosition = yStartingPosition;
	swprintf_s<sizeof(tempLine) / sizeof(pxcCHAR)> (tempLine, L"ID: %d", trackedFace->QueryUserID());
	TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));
	if (poseAnglesExist)
	{
		if (poseData->QueryConfidence() > 0)
		{
			SetTextColor(dc2, RGB(0, 0, 0));	
		}
		else
		{
			SetTextColor(dc2, RGB(255, 0, 0));	
		}
		yPosition += rowMargin;
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Yaw : %.0f", angles.yaw);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));

		yPosition += rowMargin;
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Pitch: %.0f", angles.pitch);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));

		yPosition += rowMargin;
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Roll : %.0f ", angles.roll);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));
	}	
	else
	{
		SetTextColor(dc2, RGB(255, 0, 0));	
	}

	const PXCFaceData::PulseData* pulse = trackedFace->QueryPulse();
	if (pulse != NULL)
	{	
		pxcF32 hr = pulse->QueryHeartRate();	
		yPosition += rowMargin;	
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"HR: %f", hr);

		TextOut(dc2, xStartingPosition, yPosition, tempLine, std::char_traits<wchar_t>::length(tempLine));							
	}

	DeleteObject(cyan);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void FaceTrackingRenderer2D::DrawLandmark(PXCFaceData::Face* trackedFace)
{
	const PXCFaceData::LandmarksData* landmarkData = trackedFace->QueryLandmarks();
	if (landmarkData == NULL)
		return;

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	HFONT hFont = CreateFont(8, 0, 0, 0, FW_LIGHT, 0, 0, 0, 0, 0, 0, 2, 0, L"MONOSPACE");

	if (!hFont)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}


	SetBkMode(dc2, TRANSPARENT);

	SelectObject(dc2, m_bitmap);
	SelectObject(dc2, hFont);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	pxcI32 numPoints = landmarkData->QueryNumPoints();
	if (numPoints != m_numLandmarks)
	{
		DeleteObject(hFont);
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}

	landmarkData->QueryPoints(m_landmarkPoints);
	for (int i = 0; i < numPoints; ++i) 
	{
		int x = (int)m_landmarkPoints[i].image.x + LANDMARK_ALIGNMENT;
		int y = (int)m_landmarkPoints[i].image.y + LANDMARK_ALIGNMENT;		
		if (m_landmarkPoints[i].confidenceImage)
		{
			SetTextColor(dc2, RGB(255, 255, 255));
			TextOut(dc2, x, y, L"•", 1);
		}
		else
		{
			SetTextColor(dc2, RGB(255, 0, 0));
			TextOut(dc2, x, y, L"x", 1);
		}
	}

	DeleteObject(hFont);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void FaceTrackingRenderer2D::DrawLocation(PXCFaceData::Face* trackedFace)
{
	const PXCFaceData::DetectionData* detectionData = trackedFace->QueryDetection();
	if (detectionData == NULL) 
		return;	

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255 ,255 , 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	PXCRectI32 rectangle;
	pxcBool hasRect = detectionData->QueryBoundingRect(&rectangle);
	if (!hasRect)
	{
		DeleteObject(cyan);
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}

	MoveToEx(dc2, rectangle.x, rectangle.y, 0);
	LineTo(dc2, rectangle.x, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y);
	LineTo(dc2, rectangle.x, rectangle.y);

	WCHAR line[64];
	swprintf_s<sizeof(line)/sizeof(pxcCHAR)>(line,L"%d",trackedFace->QueryUserID());
	TextOut(dc2,rectangle.x, rectangle.y, line, std::char_traits<wchar_t>::length(line));
	DeleteObject(cyan);

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}
