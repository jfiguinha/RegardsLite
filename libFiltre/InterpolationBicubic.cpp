#include <header.h>
#include "InterpolationBicubic.h"


CInterpolationBicubic::CInterpolationBicubic(const double & dWidth)
{
	wX = nullptr;
	wY = nullptr;
	m_dWidth = dWidth;
}


CInterpolationBicubic::~CInterpolationBicubic()
{
	if (wX != nullptr)
		delete[] wX;

	if (wY != nullptr)
		delete[] wY;
}

void CInterpolationBicubic::CalculWeight(const int32_t &width, const int32_t &height, const float &ratioY, const float &ratioX, const float &posTop, const float &posLeft)
{
	wX = new weightX[width];
	wY = new weightX[height];

#pragma omp parallel for
	for (auto y = 0; y < height; y++)
	{
		float posY = (float)y * ratioY + posTop;
		int valueB = (int)posY;
		float realB = posY - valueB;
		wY[y].tabF[0] = Filter(-(-1.0f - realB));
		wY[y].tabF[1] = Filter(-(0.0f - realB));
		wY[y].tabF[2] = Filter(-(1.0f - realB));
		wY[y].tabF[3] = Filter(-(2.0f - realB));
	}
#pragma omp parallel for
	for (auto x = 0; x < width; x++)
	{
		float posX = (float)x * ratioX + posLeft;
		int valueA = (int)posX;
		float realA = posX - valueA;
		wX[x].tabF[0] = Filter((-1.0f - realA));
		wX[x].tabF[1] = Filter((0.0f - realA));
		wX[x].tabF[2] = Filter((1.0f - realA));
		wX[x].tabF[3] = Filter((2.0f - realA));
	}
}

void CInterpolationBicubic::Execute(const cv::Mat & in, cv::Mat & out)
{

	if(in.rows > 0 && in.cols > 0 && out.rows > 0 && out.cols > 0)
	{

		float ratioX = (float)in.cols / (float)out.cols;
		float ratioY = (float)in.rows / (float)out.rows;

		CalculWeight(out.cols, out.rows, ratioY, ratioX, 0.0f, 0.0f);

	#pragma omp parallel for
		for (auto y = 0; y < out.rows; y++)
		{
	#pragma omp parallel for
			for (auto x = 0; x < out.cols; x++)
			{
				float posY = (float)y * ratioY;
				float posX = (float)x * ratioX;

				s_rgb value = Bicubic(in, posX, posY, wY[y].tabF, wX[x].tabF);
				
				//int position = (posX) * 4 + (posY) * in.cols * 4;

				
				int i =  (x << 2) + (y * (out.cols << 2));// int i = Out->GetPosition(x, y);
				memcpy(out.data + i, &value, sizeof(s_rgb));

			}
		}
	}

}

s_rgb CInterpolationBicubic::Bicubic(const cv::Mat& in, const float& x, const float& y, float* tabF1, float* tabF)
{
	s_rgb out{ 0,0,0,0 };
	float nDenom = 0.0;
	int valueA = (int)x;
	int valueB = (int)y;

	float r = 0.0, g = 0.0, b = 0.0, a = 0.0;

	int debutN = -1;
	int finN = 2;
	//Calcul démarrage du y;
	if (valueB == 0)
	{
		debutN = 0;
	}

	if (valueB == in.rows - 2)
	{
		finN = 1;
	}

	if (valueB == in.rows - 1)
	{
		finN = 0;
	}

	int debutM = -1;
	int finM = 2;
	//Calcul démarrage du y;
	if (valueA == 0)
	{
		debutM = 0;
	}

	if (valueA == in.cols - 2)
	{
		finM = 1;
	}

	if (valueA == in.cols - 1)
	{
		finM = 0;
	}

	int posX = valueA + debutM;
	if (valueA == 1)
		posX = valueA;

	int posY = valueB + debutN;
	if (valueB == 1)
		posY = valueB;


	for (auto n = debutN; n <= finN; n++)
	{
		for (auto m = debutM; m <= finM; m++)
		{
			int position = (posX + m) * 4 + (posY + n) * in.cols * 4;
			float f = tabF1[n + 1] * tabF[m + 1];
			nDenom += f;
			r += in.data[position] * f;
			g += in.data[position+1] * f;
			b += in.data[position+2] * f;
			a += 0;
		}
	}
	out.b = uint8_t(r / nDenom);
	out.g = uint8_t(g / nDenom);
	out.r = uint8_t(b / nDenom);
	out.alpha = uint8_t(a / nDenom);

	return out;
}

double CInterpolationBicubic::Filter(const double &f)
{
	
	return (f < -2.0 || f > 2.0) ? 0.0 : (
		(f < -1.0) ? (2.0 + f)*(2.0 + f)*(2.0 + f) / 6.0 : (
		(f < 0.0) ? (4.0 + f*f*(-6.0 - 3.0*f)) / 6.0 : (
		(f < 1.0) ? (4.0 + f*f*(-6.0 + 3.0*f)) / 6.0 : (2.0 - f)*(2.0 - f)*(2.0 - f) / 6.0)));
	
}