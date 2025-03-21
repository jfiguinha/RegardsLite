#pragma once
#include "EffectParameter.h"
//#include <ParamInit.h>
//#include <RegardsConfigParam.h>
//const float[] aspectRatio = { 1.1f,  1.33f, 1.66f, 1.85f, 2.35f };
/*
1:1 Square
4:3 Silent Film/NTSC
1.37:1 Academy Ratio
1.43:1 IMAX
3:2 Classic 35mm
7:4 Metroscope
1.85:1 Vistavision
2:1 Panascope & RED
2.2:1 Todd AI
2.35:1 Cinemascope
2.39:1 Theatrical & Blu-ray
2.55:1 Vintage Cinemascope
2.75:1 Ultra Panavision
2.76:1 MGM Camera 65
3:1 Extreme Scope
4:1 PolyVision

*/


    

class CVideoEffectParameter : public CEffectParameter
{
public:
    
    vector<float> zoom{1.0f, 1.33f, 1.66f, 1.77f, 1.85f, 2.35f};
    vector<float> vect{
    0.01f, 0.02f, 0.03f, 0.04f, 0.05f, 0.06f, 0.08f, 0.12f, 0.16f, 0.25f, 0.33f, 0.5f, 0.66f, 0.75f, 1.0f,
    1.33f, 1.5f, 1.66f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 12.0f, 16.0f
    };

	CVideoEffectParameter(): streamAudioIndex(0), streamVideoIndex(0), streamSubtitleIndex(0)
	{

        
        
		effectEnable = 0;
		//Video Parameter
		sharpness = 1.5f;
		//Sky Light
		//Tone mapping
		contrast = 0.0f;
		brightness = 0.0f;
		//exponent = 1.0f;

		//Color Boost
		color_boost[0] = 0.0f;
		color_boost[1] = 0.0f;
		color_boost[2] = 0.0f;
		color_boost[3] = 0.0f;

		SharpenEnable = 1;
		MedianEnable = 1;
		ColorBoostEnable = 1;
		BicubicEnable = 0;
		rotation = 0;
		showFPS = 0;
		grayEnable = 0;
		vhsEnable = 0;
		sepiaEnable = 0;
		enableSubtitle = 0;
		streamAudioUpdate = 0;
		streamVideoUpdate = 0;
		streamSubtitleUpdate = 0;
		//enableOpenCL = 0;
		denoiseEnable = 0;
		bandcEnable = 1;

		uSigma = 11.0;
		uThreshold = 18;
		uKSigma = 2;
		openglDenoise = false;

		denoisingLevel = 4;
		templateWindowSize = 6;
		searchWindowSize = 4;

		effectEnable = 0;

		filmgrainenable = 0;
		filmcolorisation = 0;
		filmEnhance = 0;
		ratioSelect = 0;

		tabRatio = zoom;
		tabZoom = vect;
		for (int i = 0; i < tabZoom.size(); i++)
		{
			if (tabZoom[i] == 1.0f)
				zoomSelect = i;
		}
	};
    
	/*
    CVideoEffectParameter& operator=(const CVideoEffectParameter& x)
    {
		CVideoEffectParameter effect;

		effect.effectEnable = x.effectEnable;
		//Video Parameter
		effect.sharpness = x.sharpness;
		//Sky Light
		//Tone mapping
		effect.contrast = x.contrast;
		effect.brightness = x.brightness;
		//exponent = 1.0f;

		//Color Boost
		effect.color_boost[0] = x.color_boost[0];
		effect.color_boost[1] = x.color_boost[1];
		effect.color_boost[2] = x.color_boost[2];
		effect.color_boost[3] = x.color_boost[3];

		effect.SharpenEnable = x.SharpenEnable;
		effect.MedianEnable = x.MedianEnable;
		effect.ColorBoostEnable = x.ColorBoostEnable;
		effect.BicubicEnable = x.BicubicEnable;;
		effect.rotation = x.rotation;
		effect.showFPS = x.showFPS;
		effect.grayEnable = x.grayEnable;
		effect.vhsEnable = x.vhsEnable;
		effect.sepiaEnable = x.sepiaEnable;
		effect.enableSubtitle =x.enableSubtitle;
		effect.streamAudioUpdate = x.streamAudioUpdate;
		effect.streamVideoUpdate = x.streamVideoUpdate;
		effect.streamSubtitleUpdate = x.streamSubtitleUpdate;
		//enableOpenCL = 0;
		effect.denoiseEnable = x.denoiseEnable;
		effect.bandcEnable =x.bandcEnable;

		effect.uSigma = x.uSigma;
		effect.uThreshold =x.uThreshold;
		effect.uKSigma = x.uKSigma;
		effect.openglDenoise = x.openglDenoise;

		effect.denoisingLevel = x.denoisingLevel;
		effect.templateWindowSize = x.templateWindowSize;
		effect.searchWindowSize = x.searchWindowSize;

		effect.filmgrainenable = x.filmgrainenable;
		effect.filmcolorisation = x.filmcolorisation;
		effect.filmEnhance = x.filmEnhance;
		effect.ratioSelect = x.ratioSelect;

		return effect;
    }
	*/

	~CVideoEffectParameter() override
	{
	};

	//Video Denoising Parameter
	bool openglDenoise;
	float uSigma;
	float uThreshold;
	float uKSigma;
	//float exponent;

	float denoisingLevel;

	//Video Parameter
	float sharpness;

	//Tone mapping
	float contrast;
	float brightness;

	//Color Boost
	float color_boost[4];

	int SharpenEnable;
	int MedianEnable;
	int denoiseEnable;
	int ColorBoostEnable;
	int BicubicEnable;
	//int   enableOpenCL;
	int rotation;
	int bandcEnable;
	int showFPS;
	int effectEnable;
	int grayEnable;
	int sepiaEnable;
	int vhsEnable;
	int enableSubtitle;
	int streamAudioIndex;
	int streamVideoIndex;
	int streamSubtitleIndex;
	int streamAudioUpdate;
	int streamVideoUpdate;
	int streamSubtitleUpdate;
	int filmgrainenable;
	int filmcolorisation;
	int filmEnhance;
	vector<float> tabRatio;
	vector<float> tabZoom;
	int ratioSelect = 4;
	int zoomSelect = 0;
	bool zoomUpdate = false;

	bool autoConstrast = false;
	bool stabilizeVideo = false;
	int stabilizeImageBuffere = 2;


	int templateWindowSize = 7;
	int searchWindowSize = 21;
	int h = 3;
	int hColor = 3;
 
    float subtitleSize = 1.0f;
    
    //0.5, 0.8f, 0.2f
    int subtitleRedColor = 128;
    int subtitleGreenColor = 204;
    int subtitleBlueColor = 51;
};
