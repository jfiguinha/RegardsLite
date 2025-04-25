///////////////////////////////////////////////////////////////////////////////
// Name:        convertmattowxbmp.cpp
// Purpose:     Converts OpenCV bitmap (Mat) stored as BGR CVU8 to wxBitmap
// Author:      PB
// Created:     2020-09-16
// Copyright:   (c) 2020 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////
#include "header.h"
#include <wx/wx.h>
#include <wx/rawbmp.h>
#include <opencv2/core/mat.hpp>
#include "convertmattowxbmp.h"

#ifdef __WXMSW__

namespace
{

    // Version optimisée pour Microsoft Windows.
    // matBitmap doit être continu et matBitmap.cols % 4 doit être égal à 0
    // car SetDIBits() nécessite que les lignes DIB soient alignées sur DWORD.
    bool ConvertMatBitmapTowxBitmapMSW(const cv::Mat& matBitmap, wxBitmap& bitmap)
    {
        const HDC hScreenDC = ::GetDC(nullptr);
        if (!hScreenDC) {
            wxLogError("Failed to get screen DC.");
            return false;
        }

        BITMAPINFO bitmapInfo{ 0 };
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO) - sizeof(RGBQUAD);
        bitmapInfo.bmiHeader.biWidth = bitmap.GetWidth();
        bitmapInfo.bmiHeader.biHeight = -bitmap.GetHeight(); // Negative for top-down DIB
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 24;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        bool success = ::SetDIBits(hScreenDC, bitmap.GetHBITMAP(), 0, bitmap.GetHeight(),
            matBitmap.data, &bitmapInfo, DIB_RGB_COLORS) != 0;
        ::ReleaseDC(nullptr, hScreenDC);

        if (!success) {
            wxLogError("Failed to set DIB bits.");
        }

        return success;
    }

} // namespace

#endif // __WXMSW__

bool ConvertMatBitmapTowxBitmap(const cv::Mat& matBitmap, wxBitmap& bitmap)
{
    // Vérifications des préconditions
    wxCHECK_MSG(!matBitmap.empty(), false, "Input Mat is empty.");
    wxCHECK_MSG(matBitmap.type() == CV_8UC3, false, "Input Mat must be of type CV_8UC3.");
    wxCHECK_MSG(matBitmap.dims == 2, false, "Input Mat must have 2 dimensions.");
    wxCHECK_MSG(bitmap.IsOk(), false, "wxBitmap is not valid.");
    wxCHECK_MSG(bitmap.GetWidth() == matBitmap.cols && bitmap.GetHeight() == matBitmap.rows, false,
        "wxBitmap dimensions do not match Mat dimensions.");
    wxCHECK_MSG(bitmap.GetDepth() == 24, false, "wxBitmap must have a depth of 24.");

#ifdef __WXMSW__
    if (bitmap.IsDIB() && matBitmap.isContinuous() && matBitmap.cols % 4 == 0) {
        return ConvertMatBitmapTowxBitmapMSW(matBitmap, bitmap);
    }
#endif

    wxNativePixelData pixelData(bitmap);
    if (!pixelData) {
        wxLogError("Failed to access wxBitmap pixel data.");
        return false;
    }

    wxNativePixelData::Iterator pixelDataIt(pixelData);

    const uchar* bgr = matBitmap.data;
    const bool isContinuous = matBitmap.isContinuous();

    for (int row = 0; row < pixelData.GetHeight(); ++row) {
        pixelDataIt.MoveTo(pixelData, 0, row);

        for (int col = 0; col < pixelData.GetWidth(); ++col, ++pixelDataIt) {
            if (isContinuous) {
                pixelDataIt.Blue() = *bgr++;
                pixelDataIt.Green() = *bgr++;
                pixelDataIt.Red() = *bgr++;
            }
            else {
                const cv::Vec3b& pixel = matBitmap.at<cv::Vec3b>(row, col);
                pixelDataIt.Blue() = pixel[0];
                pixelDataIt.Green() = pixel[1];
                pixelDataIt.Red() = pixel[2];
            }
        }
    }

    return bitmap.IsOk();
}