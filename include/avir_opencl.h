#pragma once
#include <libResource.h>
#include "OpenCLParameter.h"
#include "OpenCLContext.h"

extern cv::ocl::OpenCLExecutionContext clExecCtx;
using namespace Regards::OpenCL;
using namespace cv;

class CAvirFilterOpenCL
{
public:
	static void GetDataOpenCLHtoV_dither(cv::UMat& src, float*& dest, const int& iPos, float gm);
	static UMat ConvertToFloat(cv::UMat& src, const int& width, const int& height, const int& yPosition);
	static UMat ConvertToFloat(cv::UMat& src, const int& width, const int& height);
	static UMat ConvertToFloat(const uchar* data, const int& width, const int& height); 
	static UMat UpSampleUMat(cv::UMat& src, const int& width, const int& height, int widthSrc, int start, int ResampleFactor);
	static UMat UpSample2D(cv::UMat& src, const int& width, const int& height, int widthSrc, int start, int outLen, int ResampleFactor); 
	static float* UpSample(cv::UMat& src, float*& dest, const int& width, const int& height, int widthSrc, int start, int ResampleFactor);
	static UMat doResize2OpenCL(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
	static void doResize2OpenCL(cv::UMat& src, float*& dest, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
	static void doFilterOpenCL(cv::UMat& src, float*& dest, const int& width, const int& height, const float* f, int flen);
	static cv::UMat GetDataOpenUMat(cv::UMat& src, const int& iPos); 
	static cv::Mat GetDataOpenCL(cv::UMat& src, const int& iPos);
	static void GetDataOpenCL(cv::UMat& src, float*& dest, const int& iPos);
	static cv::Mat GetDataOpenCLHtoV(cv::UMat& src, const int& iPos);
	static cv::UMat GetDataOpenCLHtoV2D(cv::UMat& src);
	static void GetDataOpenCLHtoV(cv::UMat& src, float*& dest, const int& iPos);
	static UMat GetDataOpenCLHtoV_dither(cv::UMat& src, int iPos, float gm);
	static UMat GetDataOpenCLHtoV_dither2D(cv::UMat& src, float gm);
	static void DitherOpenCL(cv::UMat& src, float*& dest, int srcLen, float PkOut, float TrMul0);
	static UMat DitherOpenCL2D(cv::UMat& src, float PkOut, float TrMul0);
	static void doCopyOpenCL_UMat(cv::UMat& output, cv::UMat& src, const int& iPos);
	static UMat doFilterOpenCL_UMat(cv::UMat& src, const int& width, const int& height,
		const float* f, int flen);
	static UMat doFilterOpenCL2D(cv::UMat& src, const int& width, const int& height,
		const float* f, int flen);
	static void doResizeOpenCL(cv::UMat& src, float*& dest, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
	
	static UMat doResize2OpenCL2D(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
	static UMat doResizeOpenCL2D(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
};