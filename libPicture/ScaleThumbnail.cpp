#include <header.h>
#include "ScaleThumbnail.h"
#include "ImageLoadingFormat.h"
using namespace Regards::Picture;

CScaleThumbnail::CScaleThumbnail(void)
{
}


CScaleThumbnail::~CScaleThumbnail(void)
{
}


//-----------------------------------------------------------------
//Calcul du ratio pour l'image plein écran
//-----------------------------------------------------------------
float CScaleThumbnail::CalculRatio(CImageLoadingFormat* pBitmap, const int& xMax, const int& yMax)
{
	float newRatio;

	//int tailleAffichageWidth = 0, tailleAffichageHeight = 0;

	if (pBitmap->GetWidth() > pBitmap->GetHeight())
		newRatio = static_cast<float>(xMax) / static_cast<float>(pBitmap->GetWidth());
	else
		newRatio = static_cast<float>(yMax) / static_cast<float>(pBitmap->GetHeight());

	if ((pBitmap->GetHeight() * newRatio) > yMax)
	{
		newRatio = static_cast<float>(yMax) / static_cast<float>(pBitmap->GetHeight());
	}
	else
	{
		if ((pBitmap->GetWidth() * newRatio) > xMax)
		{
			newRatio = static_cast<float>(xMax) / static_cast<float>(pBitmap->GetWidth());
		}
	}

	return newRatio;
}

//-----------------------------------------------------------------
//Calcul du ratio pour l'image plein écran
//-----------------------------------------------------------------
float CScaleThumbnail::CalculRatio(const int& width, const int& height, const int& xMax, const int& yMax)
{
	float newRatio;

	//int tailleAffichageWidth = 0, tailleAffichageHeight = 0;

	if (width > height)
		newRatio = static_cast<float>(xMax) / static_cast<float>(width);
	else
		newRatio = static_cast<float>(yMax) / static_cast<float>(height);

	if ((height * newRatio) > yMax)
	{
		newRatio = static_cast<float>(yMax) / static_cast<float>(height);
	}
	else
	{
		if ((width * newRatio) > xMax)
		{
			newRatio = static_cast<float>(xMax) / static_cast<float>(width);
		}
	}

	return newRatio;
}

void CScaleThumbnail::CreateScaleBitmap(CImageLoadingFormat* pBitmap, const int& width, const int& height)
{
	if (!pBitmap || width <= 0 || height <= 0)
		return;

	float newRatio = CalculRatio(pBitmap, width, height);
	if (newRatio == 0.0f)
		return;

	int nTailleAffichageWidth = static_cast<int>(pBitmap->GetWidth() * newRatio);
	int nTailleAffichageHeight = static_cast<int>(pBitmap->GetHeight() * newRatio);

	if (nTailleAffichageWidth == pBitmap->GetWidth() && nTailleAffichageHeight == pBitmap->GetHeight())
		return;

	cv::Mat resized_down;
	try {
		resize(pBitmap->GetMatrix().getMat(), resized_down, cv::Size(nTailleAffichageWidth, nTailleAffichageHeight), cv::INTER_CUBIC);
	}
	catch (const cv::Exception& e) {
		// Gérer l'erreur ici
		return;
	}
}
