#pragma once
class CRegardsBitmap;

namespace Regards::FiltreEffet
{
	class CMotionBlur
	{
	public:
		CMotionBlur(void);
		~CMotionBlur(void);
		static vector<wxPoint> GetOffsetKernel(int width, const double& angle);
		static vector<double> GetMotionBlurKernel(const double& radius, double sigma);
		bool MotionBlur(cv::Mat& bitmap, const double& radius, const double& sigma, const double& angle);

	private:
		static int GenerateMotionBlurKernel(int width, double sigma, vector<double>& kernel);
		void Execute(cv::Mat& bitmap, const vector<double>& kernel, const vector<wxPoint>& offsets);
	};
}
