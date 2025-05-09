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
	static cv::UMat GetDataOpenCLHtoV2D(cv::UMat& src);
	static UMat ConvertToFloat(cv::UMat& src, const int& width, const int& height);
	static UMat UpSample2D(cv::UMat& src, const int& width, const int& height, int widthSrc, int start, int outLen, int ResampleFactor);
	static UMat doFilterOpenCL2D(cv::UMat& src, const int& width, const int& height, const float* f, int flen, int step);
	static UMat GetDataOpenCLHtoVDither2D(cv::UMat& src, float gm, float PkOut, float TrMul0);
	static UMat doResize2OpenCL2D(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
	static UMat doResizeOpenCL2D(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen);
};