// ReSharper disable CppEntityAssignedButNoRead
#include "header.h"
#include "RegardsConfigParam.h"
#include <fstream>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <config_id.h>
#include <ConvertUtility.h>
#include <EffectVideoParameter.h>
using namespace rapidxml;

CRegardsConfigParam::CRegardsConfigParam()
{
	iconSizeRatio = 1.0;
	numLibPreview = 2;
	numEffect = 0;
	openCLNumIndex = 0;
	numLibEffect = 0;
	numLibVideo = 0;
	diaporamaTime = 3;
	diaporamaEffect = 400;
	diaporamaFullscreen = 1;
	openCLPlatformName = "";
	dataInMemory = 0;
	geolocUrl = "http://www.geoplugin.net";
	thumbnailQuality = 1;
	thumbnailIconeCache = 1;
	pictureSize = 0;
	nbProcessThumbnail = 1;
	nbProcessExif = 1;
	nbProcessFace = 1;
	nbGpsFileByMinute = 60;
	numLanguage = 1;
	numInterpolation = 0;
	openCLSupport = 1;
	openCLOpenGLInteropSupport = 0;
	detectOrientation = 0;
	autoContrast = 0;
	videoDecoderHardware = "none";
	videoEncoderHardware = "none";
	openCLFaceSupport = 0;
	videoFaceDetection = 0;
	faceDetection = 0;
	fastFaceDetection = 1;
	musicDiaporama = "";
	isThumbnailOpenCV = 1;
	bufferSize = 100;
	numSuperResolution = 0;
	useSuperResolution = 0;
	cudaSupport = 0;
    useCuda = 0;
    openGLOutputColor = "RGBA";
    videoEffectParameter = new CVideoEffectParameter();
}

CVideoEffectParameter * CRegardsConfigParam::GetVideoEffectParameter()
{
    return videoEffectParameter;
}

void CRegardsConfigParam::SetVideoEffectParameter(const CVideoEffectParameter * videoEffect)
{
    *videoEffectParameter = *videoEffect;
}

wxString CRegardsConfigParam::GetOpenGLOutputColor()
{
    return openGLOutputColor;
}

void CRegardsConfigParam::SetOpenGLOutputColor(const wxString& openGLOutputColor)
{
    this->openGLOutputColor = openGLOutputColor;
}

int CRegardsConfigParam::GetThumbnailOpenCV()
{
	return isThumbnailOpenCV;
}
void CRegardsConfigParam::SetThumbnailOpenCV(const int& isThumbnailOpenCV)
{
	this->isThumbnailOpenCV = isThumbnailOpenCV;
}

int CRegardsConfigParam::GetSkinWindowMode()
{
	return skinWindowMode;
}

void CRegardsConfigParam::SetSkinWindowMode(const int& skinWindowMode)
{
	this->skinWindowMode = skinWindowMode;
}

int CRegardsConfigParam::GetBufferSize()
{
	return bufferSize;
}

void CRegardsConfigParam::SetBufferSize(const int& value)
{
	bufferSize = value;
}


wxString CRegardsConfigParam::GetMusicDiaporama()
{
	return musicDiaporama;
}

void CRegardsConfigParam::SetMusicDiaporama(const wxString& musicDiaporama)
{
	this->musicDiaporama = musicDiaporama;
}

void CRegardsConfigParam::SetFastDetectionFace(const int& fastDetection)
{
	this->fastFaceDetection = fastDetection;
}

int CRegardsConfigParam::GetFastDetectionFace()
{
	return fastFaceDetection;
}

int CRegardsConfigParam::GetFaceVideoDetection()
{
	return videoFaceDetection;
}

void CRegardsConfigParam::SetFaceVideoDetection(const int& videoFaceDetection)
{
	this->videoFaceDetection = videoFaceDetection;
}


int CRegardsConfigParam::GetAutoConstrast()
{
	return autoContrast;
}

void CRegardsConfigParam::SetAutoConstrast(const int& autoContrast)
{
	this->autoContrast = autoContrast;
}

void CRegardsConfigParam::SetHardwareDecoder(const wxString& hardwareDecoder)
{
	videoDecoderHardware = hardwareDecoder;
}

wxString CRegardsConfigParam::GetHardwareDecoder()
{
	return videoDecoderHardware;
}

void CRegardsConfigParam::SetHardwareEncoder(const wxString& hardwareEncoder)
{
	videoEncoderHardware = hardwareEncoder;
}

wxString CRegardsConfigParam::GetHardwareEncoder()
{
	return videoEncoderHardware;
}

bool CRegardsConfigParam::GetIsOpenCLOpenGLInteropSupport()
{
	return openCLOpenGLInteropSupport;
}

void CRegardsConfigParam::SetIsOpenCLOpenGLInteropSupport(const int& openCLOpenGLInteropSupport)
{
	this->openCLOpenGLInteropSupport = openCLOpenGLInteropSupport;
}

bool CRegardsConfigParam::GetIsOpenCLSupport()
{
	return openCLSupport;
}

void CRegardsConfigParam::SetIsOpenCLSupport(const int& openCLSupport)
{
	this->openCLSupport = openCLSupport;
}

int CRegardsConfigParam::GetIsCudaSupport()
{
	return cudaSupport;
}

void CRegardsConfigParam::SetIsUseCuda(const int& useCuda)
{
	this->useCuda = useCuda;
}

int CRegardsConfigParam::GetIsUseCuda()
{
	return useCuda;
}

void CRegardsConfigParam::SetIsCudaSupport(const int& cudaSupport)
{
	this->cudaSupport = cudaSupport;
}

int CRegardsConfigParam::GetInterpolationType()
{
	return numInterpolation;
}

int CRegardsConfigParam::GetSuperResolutionType()
{
	return numSuperResolution;
}

int CRegardsConfigParam::GetUseSuperResolution()
{
	return useSuperResolution;
}

void CRegardsConfigParam::SetSuperResolutionType(const int& value)
{
	numSuperResolution = value;
}


void CRegardsConfigParam::SetUseSuperResolution(const int& value)
{
	useSuperResolution = value;
}


void CRegardsConfigParam::SetInterpolationType(const int& numInterpolation)
{
	this->numInterpolation = numInterpolation;
}

int CRegardsConfigParam::GetOpenCLLoadFromBinaries()
{
	return loadFromBinaries;
}

void CRegardsConfigParam::SetOpenCLLoadFromBinaries(const int& loadFromBinaries)
{
	this->loadFromBinaries = loadFromBinaries;
}

int CRegardsConfigParam::GetThumbnailProcess()
{
	return nbProcessThumbnail;
}

void CRegardsConfigParam::SetThumbnailProcess(const int& nbProcess)
{
	nbProcessThumbnail = nbProcess;
}

int CRegardsConfigParam::GetNumLanguage()
{
	return numLanguage;
}

void CRegardsConfigParam::SetNumLanguage(const int& numLanguage)
{
	this->numLanguage = numLanguage;
}

int CRegardsConfigParam::GetFaceDetection()
{
	return faceDetection;
}

void CRegardsConfigParam::SetFaceDetection(const int& faceDetection)
{
	this->faceDetection = faceDetection;
}

int CRegardsConfigParam::GetFaceOpenCLProcess()
{
	return openCLFaceSupport;
}

void CRegardsConfigParam::SetFaceOpenCLProcess(const int& openclProcess)
{
	this->openCLFaceSupport = openclProcess;
}

int CRegardsConfigParam::GetFaceProcess()
{
	return nbProcessFace;
}

void CRegardsConfigParam::SetFaceProcess(const int& nbProcess)
{
	nbProcessFace = nbProcess;
}

int CRegardsConfigParam::GetExifProcess()
{
	return nbProcessExif;
}

void CRegardsConfigParam::SetExifProcess(const int& nbProcess)
{
	nbProcessExif = nbProcess;
}

int CRegardsConfigParam::GetFaceDetectionPictureSize()
{
	return pictureSize;
}

void CRegardsConfigParam::SetFaceDetectionPictureSize(const int& numIndex)
{
	pictureSize = numIndex;
}

bool CRegardsConfigParam::GetDatabaseInMemory()
{
	return dataInMemory;
}

void CRegardsConfigParam::SetDatabaseInMemory(const int& value)
{
	dataInMemory = value;
}

float CRegardsConfigParam::GetIconSizeRatio()
{
	return iconSizeRatio;
}

void CRegardsConfigParam::SetIconSizeRatio(const float& ratio)
{
	this->iconSizeRatio = ratio;
}

int CRegardsConfigParam::GetThumbnailQuality()
{
	return thumbnailQuality;
}

void CRegardsConfigParam::SetThumbnailQuality(const int& quality)
{
	thumbnailQuality = quality;
}

int CRegardsConfigParam::GetThumbnailIconeCache()
{
	return thumbnailIconeCache;
}

void CRegardsConfigParam::SetThumbnailIconeCache(const int& iconeCache)
{
	thumbnailIconeCache = iconeCache;
}

wxString CRegardsConfigParam::GetUrlServer()
{
	wxString value = geolocUrl;
	return value;
}

int CRegardsConfigParam::GetNbGpsIterationByMinute()
{
	return nbGpsFileByMinute;
}

CRegardsConfigParam::~CRegardsConfigParam()
{
	doc.clear();
}

int CRegardsConfigParam::GetVideoLibrary()
{
	return numLibVideo;
}

void CRegardsConfigParam::SetVideoLibrary(const int& numLib)
{
	numLibVideo = numLib;
}

int CRegardsConfigParam::GetEffectLibrary()
{
	return numLibEffect;
}

int CRegardsConfigParam::GetSoundVolume()
{
	return soundVolume;
}

void CRegardsConfigParam::SetSoundVolume(const int& soundVolume)
{
	this->soundVolume = soundVolume;
}

void CRegardsConfigParam::SetEffectLibrary(const int& numLib)
{
	numLibEffect = numLib;
}

int CRegardsConfigParam::GetPreviewLibrary()
{
	return LIBOPENCL;
}

void CRegardsConfigParam::SetPreviewLibrary(const int& numLib)
{
	this->numLibPreview = numLib;
}

int CRegardsConfigParam::GetOpenCLPlatformIndex()
{
	return openCLNumIndex;
}

void CRegardsConfigParam::SetOpenCLPlatformIndex(const int& numIndex)
{
	openCLNumIndex = numIndex;
}

wxString CRegardsConfigParam::GetOpenCLPlatformName()
{
	return openCLPlatformName;
}

void CRegardsConfigParam::SetOpenCLPlatformName(const wxString& platformName)
{
	openCLPlatformName = platformName;
}

void CRegardsConfigParam::SetInverseVideoRotation(const int& inverseRot)
{
	inverseRotation = inverseRot;
}
int CRegardsConfigParam::GetInverseVideoRotation()
{
	return inverseRotation;
}

int CRegardsConfigParam::GetEffect()
{
	if (numEffect < 300)
		return 0;
	return numEffect;
}

void CRegardsConfigParam::SetEffect(const int& numEffect)
{
	this->numEffect = numEffect;
}

void CRegardsConfigParam::SetEffectLibrary(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("NumLibrary", to_string(numLibEffect)));
	sectionPosition->append_node(node("NumEffect", to_string(numEffect)));
	sectionPosition->append_node(node("NbProcessThumbnail", to_string(nbProcessThumbnail)));
	sectionPosition->append_node(node("NbProcessExif", to_string(nbProcessExif)));
	sectionPosition->append_node(node("NbProcessFace", to_string(nbProcessFace)));
	sectionPosition->append_node(node("GpsFileByMinute", to_string(nbGpsFileByMinute)));
	sectionPosition->append_node(node("videoFaceDetection", to_string(videoFaceDetection)));
	sectionPosition->append_node(node("FaceDetection", to_string(faceDetection)));
	sectionPosition->append_node(node("FastFaceDetection", to_string(fastFaceDetection)));
}

void CRegardsConfigParam::SetVideoLibrary(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("NumLibrary", to_string(numLibVideo)));
	sectionPosition->append_node(node("HardwareDecoder", videoDecoderHardware));
	sectionPosition->append_node(node("HardwareEncoder", videoEncoderHardware));
	sectionPosition->append_node(node("SoundVolume", to_string(soundVolume)));
}

void CRegardsConfigParam::SetImageLibrary(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("NumLibrary", to_string(numLibPreview)));
	sectionPosition->append_node(node("OpenCLNumIndex", to_string(openCLNumIndex)));
	sectionPosition->append_node(node("OpenCLPlatformName", openCLPlatformName));
	sectionPosition->append_node(node("LoadFromBinaries", to_string(loadFromBinaries)));
	sectionPosition->append_node(node("NumInterpolation", to_string(numInterpolation)));
	sectionPosition->append_node(node("OpenCLSupport", to_string(openCLSupport)));
	sectionPosition->append_node(node("openCLOpenGLInteropSupport", to_string(openCLOpenGLInteropSupport)));
    sectionPosition->append_node(node("openGLOutputColor", openGLOutputColor));
	sectionPosition->append_node(node("detectOrientation", to_string(detectOrientation)));
	sectionPosition->append_node(node("autoContrast", to_string(autoContrast)));
	sectionPosition->append_node(node("openCLFaceSupport", to_string(openCLFaceSupport)));
	sectionPosition->append_node(node("useSuperResolution", to_string(useSuperResolution)));
	sectionPosition->append_node(node("numSuperResolution", to_string(numSuperResolution)));
	sectionPosition->append_node(node("isThumbnailOpenCV", to_string(isThumbnailOpenCV)));
	sectionPosition->append_node(node("cudaSupport", to_string(cudaSupport)));
    sectionPosition->append_node(node("useCuda", to_string(useCuda)));
	sectionPosition->append_node(node("inverseRotation", to_string(inverseRotation)));
}

void CRegardsConfigParam::SetVideoEffectParameter(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("effectEnable", to_string(videoEffectParameter->effectEnable)));
	sectionPosition->append_node(node("sharpness", to_string(videoEffectParameter->sharpness)));
	sectionPosition->append_node(node("contrast", to_string(videoEffectParameter->contrast)));
	sectionPosition->append_node(node("brightness", to_string(videoEffectParameter->brightness)));
	sectionPosition->append_node(node("color_boost_0", to_string(videoEffectParameter->color_boost[0])));
	sectionPosition->append_node(node("color_boost_1", to_string(videoEffectParameter->color_boost[1])));
	sectionPosition->append_node(node("color_boost_2", to_string(videoEffectParameter->color_boost[2])));
    sectionPosition->append_node(node("color_boost_3", to_string(videoEffectParameter->color_boost[3])));
	sectionPosition->append_node(node("SharpenEnable", to_string(videoEffectParameter->SharpenEnable)));
	sectionPosition->append_node(node("MedianEnable", to_string(videoEffectParameter->MedianEnable)));
	sectionPosition->append_node(node("ColorBoostEnable", to_string(videoEffectParameter->ColorBoostEnable)));
	sectionPosition->append_node(node("BicubicEnable", to_string(videoEffectParameter->BicubicEnable)));
	sectionPosition->append_node(node("rotation", to_string(videoEffectParameter->rotation)));
	sectionPosition->append_node(node("showFPS", to_string(videoEffectParameter->showFPS)));
	sectionPosition->append_node(node("grayEnable", to_string(videoEffectParameter->grayEnable)));
    sectionPosition->append_node(node("vhsEnable", to_string(videoEffectParameter->vhsEnable)));
    sectionPosition->append_node(node("sepiaEnable", to_string(videoEffectParameter->sepiaEnable)));
    sectionPosition->append_node(node("denoiseEnable", to_string(videoEffectParameter->denoiseEnable)));
    sectionPosition->append_node(node("uSigma", to_string(videoEffectParameter->uSigma)));
    sectionPosition->append_node(node("uThreshold", to_string(videoEffectParameter->uThreshold)));
    sectionPosition->append_node(node("uKSigma", to_string(videoEffectParameter->uKSigma)));
    sectionPosition->append_node(node("openglDenoise", to_string(videoEffectParameter->openglDenoise)));
    sectionPosition->append_node(node("denoisingLevel", to_string(videoEffectParameter->denoisingLevel)));
	sectionPosition->append_node(node("templateWindowSize", to_string(videoEffectParameter->templateWindowSize)));
	sectionPosition->append_node(node("searchWindowSize", to_string(videoEffectParameter->searchWindowSize)));
	sectionPosition->append_node(node("filmgrainenable", to_string(videoEffectParameter->filmgrainenable)));
	sectionPosition->append_node(node("filmcolorisation", to_string(videoEffectParameter->filmcolorisation)));
	sectionPosition->append_node(node("filmEnhance", to_string(videoEffectParameter->filmEnhance)));
	sectionPosition->append_node(node("ratioSelect", to_string(videoEffectParameter->ratioSelect)));

	sectionPosition->append_node(node("subtitleSize", to_string(videoEffectParameter->subtitleSize)));
	sectionPosition->append_node(node("subtitleRedColor", to_string(videoEffectParameter->subtitleRedColor)));
	sectionPosition->append_node(node("subtitleGreenColor", to_string(videoEffectParameter->subtitleGreenColor)));
	sectionPosition->append_node(node("subtitleBlueColor", to_string(videoEffectParameter->subtitleBlueColor)));

	
}

void CRegardsConfigParam::GetVideoEffectParameter(xml_node<>* position_node)
{
 	wxString value;
	wxString nodeName;
    vector<wxString> listParam = {"effectEnable","sharpness","contrast","brightness","color_boost_0","color_boost_1","color_boost_2","color_boost_3","SharpenEnable","MedianEnable","ColorBoostEnable","BicubicEnable","rotation","showFPS","grayEnable","vhsEnable","sepiaEnable","denoiseEnable","uSigma","uThreshold","uKSigma","openglDenoise","denoisingLevel","templateWindowSize","searchWindowSize","filmgrainenable","filmcolorisation","filmEnhance","ratioSelect","subtitleSize", "subtitleRedColor", "subtitleGreenColor", "subtitleBlueColor" };
	for(int i = 0;i < listParam.size();i++)
    {
        xml_node<>* child_node = position_node->first_node(listParam[i]);
        if (child_node != nullptr)
        {
            
            
            if(listParam[i] == "effectEnable")
                videoEffectParameter->effectEnable = atoi(child_node->value());
            else if(listParam[i] == "sharpness")
                videoEffectParameter->sharpness = atoi(child_node->value());
            else if(listParam[i] == "contrast") 
                videoEffectParameter->contrast = atoi(child_node->value());
            else if(listParam[i] == "brightness") 
                videoEffectParameter->brightness = atoi(child_node->value());
            else if(listParam[i] == "color_boost_0") 
                videoEffectParameter->color_boost[0] = atoi(child_node->value());
            else if(listParam[i] == "color_boost_1") 
                videoEffectParameter->color_boost[1] = atoi(child_node->value());
            else if(listParam[i] == "color_boost_2") 
                videoEffectParameter->color_boost[2] = atoi(child_node->value());
            else if(listParam[i] == "color_boost_3") 
                videoEffectParameter->color_boost[3] = atoi(child_node->value());
            else if(listParam[i] == "SharpenEnable") 
                videoEffectParameter->SharpenEnable = atoi(child_node->value());
            else if(listParam[i] == "MedianEnable") 
                videoEffectParameter->MedianEnable = atoi(child_node->value());
            else if(listParam[i] == "ColorBoostEnable") 
                videoEffectParameter->ColorBoostEnable = atoi(child_node->value());
            else if(listParam[i] == "BicubicEnable") 
                videoEffectParameter->BicubicEnable = atoi(child_node->value());
            else if(listParam[i] == "rotation") 
                videoEffectParameter->rotation = atoi(child_node->value());
            else if(listParam[i] == "showFPS") 
                videoEffectParameter->showFPS = atoi(child_node->value());
            else if(listParam[i] == "grayEnable") 
                videoEffectParameter->grayEnable = atoi(child_node->value());
            else if(listParam[i] == "vhsEnable") 
                videoEffectParameter->vhsEnable = atoi(child_node->value());
            else if(listParam[i] == "sepiaEnable") 
                videoEffectParameter->sepiaEnable = atoi(child_node->value());
            else if(listParam[i] == "denoiseEnable") 
                videoEffectParameter->denoiseEnable = atoi(child_node->value());
            else if(listParam[i] == "uSigma") 
                videoEffectParameter->uSigma = atoi(child_node->value());
            else if(listParam[i] == "uThreshold") 
                videoEffectParameter->uThreshold = atoi(child_node->value());
            else if(listParam[i] == "uKSigma") 
                videoEffectParameter->uKSigma = atoi(child_node->value());
            else if(listParam[i] == "openglDenoise") 
                videoEffectParameter->openglDenoise = atoi(child_node->value());
            else if(listParam[i] == "denoisingLevel") 
                videoEffectParameter->denoisingLevel = atoi(child_node->value());
            else if(listParam[i] == "templateWindowSize") 
                videoEffectParameter->templateWindowSize = atoi(child_node->value());
            else if(listParam[i] == "searchWindowSize") 
                videoEffectParameter->searchWindowSize = atoi(child_node->value());
            else if(listParam[i] == "filmgrainenable") 
                videoEffectParameter->filmgrainenable = atoi(child_node->value());
            else if(listParam[i] == "filmcolorisation") 
                videoEffectParameter->filmcolorisation = atoi(child_node->value());
            else if(listParam[i] == "filmEnhance") 
                videoEffectParameter->filmEnhance = atoi(child_node->value());
            else if(listParam[i] == "ratioSelect") 
                videoEffectParameter->ratioSelect = atoi(child_node->value());
			else if (listParam[i] == "subtitleSize")
				videoEffectParameter->subtitleSize = atof(child_node->value());
			else if (listParam[i] == "subtitleRedColor")
				videoEffectParameter->subtitleRedColor = atoi(child_node->value());
			else if (listParam[i] == "subtitleGreenColor")
				videoEffectParameter->subtitleGreenColor = atoi(child_node->value());
			else if (listParam[i] == "subtitleBlueColor")
				videoEffectParameter->subtitleBlueColor = atoi(child_node->value());
        }
    }

}

int CRegardsConfigParam::GetDiaporamaTime()
{
	return diaporamaTime;
}

void CRegardsConfigParam::SetDiaporamaTime(const int& diaporamaTime)
{
	this->diaporamaTime = diaporamaTime;
}

void CRegardsConfigParam::GetVideoLibrary(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("NumLibrary");
	if (child_node != nullptr)
	{
		
		
		numLibVideo = atoi(child_node->value());
	}

	child_node = position_node->first_node("HardwareDecoder");
	if (child_node != nullptr)
	{
		
		
		videoDecoderHardware = child_node->value();
	}

	child_node = position_node->first_node("HardwareEncoder");
	if (child_node != nullptr)
	{
		
		
		videoEncoderHardware = child_node->value();
	}

	child_node = position_node->first_node("SoundVolume");
	if (child_node != nullptr)
	{
		
		
		soundVolume = atoi(child_node->value());
	}
}

void CRegardsConfigParam::GetEffectLibrary(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("NumLibrary");
	if (child_node != nullptr)
	{
		
		
		numLibEffect = atoi(child_node->value());
	}

	child_node = position_node->first_node("NumEffect");
	if (child_node != nullptr)
	{


		numEffect = atoi(child_node->value());
	}

	child_node = position_node->first_node("NbProcessThumbnail");
	if (child_node != nullptr)
	{
		
		
		nbProcessThumbnail = atoi(child_node->value());
	}

	child_node = position_node->first_node("NbProcessExif");
	if (child_node != nullptr)
	{
		
		
		nbProcessExif = atoi(child_node->value());
	}

	child_node = position_node->first_node("NbProcessFace");
	if (child_node != nullptr)
	{
		
		
		nbProcessFace = atoi(child_node->value());
	}

	child_node = position_node->first_node("GpsFileByMinute");
	if (child_node != nullptr)
	{
		
		
		nbGpsFileByMinute = atoi(child_node->value());
	}

	child_node = position_node->first_node("videoFaceDetection");
	if (child_node != nullptr)
	{
		
		
		videoFaceDetection = atoi(child_node->value());
	}

	child_node = position_node->first_node("FaceDetection");
	if (child_node != nullptr)
	{
		faceDetection = atoi(child_node->value());
	}

	child_node = position_node->first_node("FastFaceDetection");
	if (child_node != nullptr)
	{
		fastFaceDetection = atoi(child_node->value());
	}
}

void CRegardsConfigParam::GetImageLibrary(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("NumLibrary");
	if (child_node != nullptr)
	{
		numLibPreview = atoi(child_node->value());
	}

	child_node = position_node->first_node("OpenCLNumIndex");
	if (child_node != nullptr)
	{
		openCLNumIndex = atoi(child_node->value());
	}

	child_node = position_node->first_node("OpenCLPlatformName");
	if (child_node != nullptr)
	{
		
		
		openCLPlatformName = child_node->value();
	}

	child_node = position_node->first_node("LoadFromBinaries");
	if (child_node != nullptr)
	{
		
		
		loadFromBinaries = atoi(child_node->value());
	}

	child_node = position_node->first_node("NumInterpolation");
	if (child_node != nullptr)
	{
		
		
		numInterpolation = atoi(child_node->value());
	}

	child_node = position_node->first_node("OpenCLSupport");
	if (child_node != nullptr)
	{
		openCLSupport = atoi(child_node->value());
	}

	child_node = position_node->first_node("cudaSupport");
	if (child_node != nullptr)
	{
		cudaSupport = atoi(child_node->value());
	}
    
	child_node = position_node->first_node("useCuda");
	if (child_node != nullptr)
	{
		useCuda = atoi(child_node->value());
	}

	child_node = position_node->first_node("inverseRotation");
	if (child_node != nullptr)
	{
		inverseRotation = atoi(child_node->value());
	}

	child_node = position_node->first_node("openCLOpenGLInteropSupport");
	if (child_node != nullptr)
	{
		
		
		openCLOpenGLInteropSupport = atoi(child_node->value());
	}

	child_node = position_node->first_node("openGLOutputColor");
	if (child_node != nullptr)
	{
		
		
		openGLOutputColor = child_node->value();
	}
    
	child_node = position_node->first_node("detectOrientation");
	if (child_node != nullptr)
	{
		
		
		detectOrientation = atoi(child_node->value());
	}

	child_node = position_node->first_node("autoContrast");
	if (child_node != nullptr)
	{
		
		
		autoContrast = atoi(child_node->value());
	}

	child_node = position_node->first_node("openCLFaceSupport");
	if (child_node != nullptr)
	{
		
		
		openCLFaceSupport = atoi(child_node->value());
	}

	child_node = position_node->first_node("useSuperResolution");
	if (child_node != nullptr)
	{
		
		
		useSuperResolution = atoi(child_node->value());
	}

	child_node = position_node->first_node("numSuperResolution");
	if (child_node != nullptr)
	{
		
		
		numSuperResolution = atoi(child_node->value());
	}

	child_node = position_node->first_node("isThumbnailOpenCV");
	if (child_node != nullptr)
	{
		
		
		isThumbnailOpenCV = atoi(child_node->value());
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//Saving Parameter
//////////////////////////////////////////////////////////////////////////////////////////
void CRegardsConfigParam::SaveParameter()
{
	doc.clear();
	// xml declaration
	xml_node<>* decl = doc.allocate_node(node_declaration);
	xml_attribute<>* ver = doc.allocate_attribute(stralloc("version"), stralloc("1.0"));
	xml_attribute<>* encoding = doc.allocate_attribute(stralloc("encoding"), stralloc("ISO-8859-1"));
	doc.append_attribute(ver);
	doc.append_attribute(encoding);
	doc.append_node(decl);

	xml_node<>* root = doc.allocate_node(node_element, stralloc("Parameter"));
	doc.append_node(root);

	xml_node<>* sectionImageLib = node("ImageLibrary");
	SetImageLibrary(sectionImageLib);
	root->append_node(sectionImageLib);
	xml_node<>* sectionVideoLib = node("VideoLibrary");
	SetVideoLibrary(sectionVideoLib);
	root->append_node(sectionVideoLib);
	xml_node<>* sectionEffectLib = node("EffectLibrary");
	SetEffectLibrary(sectionEffectLib);
	root->append_node(sectionEffectLib);
	xml_node<>* sectionDatabase = node("Database");
	SetDatabaseParameter(sectionDatabase);
	root->append_node(sectionDatabase);
	xml_node<>* sectionDiaporama = node("Diaporama");
	SetDiaporamaParameter(sectionDiaporama);
	root->append_node(sectionDiaporama);
	xml_node<>* sectionGeolocalisation = node("Geolocalisation");
	SetGeolocalisationServer(sectionGeolocalisation);
	root->append_node(sectionGeolocalisation);
	xml_node<>* sectionThumbnail = node("Thumbnail");
	SetThumbnail(sectionThumbnail);
	root->append_node(sectionThumbnail);
	xml_node<>* sectionIcon = node("Icon");
	SetIconParameter(sectionIcon);
	root->append_node(sectionIcon);
	xml_node<>* videoEffect = node("VideoEffect");
	SetVideoEffectParameter(videoEffect);
	root->append_node(videoEffect);
	xml_node<>* windowMode = node("Window");
	SetWindowParameter(windowMode);
	root->append_node(windowMode);
	// save the xml data to a file (could equally well use any other ostream)
	std::ofstream file(CConvertUtility::ConvertToStdString(filename));
	if (file.is_open())
	{
		file << doc;
		file.close();
	}
}


void CRegardsConfigParam::SetWindowParameter(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("Mode", to_string(skinWindowMode)));
}

void CRegardsConfigParam::GetWindowParameter(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("Mode");
	if (child_node != nullptr)
	{
		
		
		skinWindowMode = static_cast<int>(atoi(child_node->value()));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//Loading Parameter
//////////////////////////////////////////////////////////////////////////////////////////
void CRegardsConfigParam::LoadParameter()
{
	xml_node<>* root_node;
	//long nodeSize = 0;
	root_node = doc.first_node("Parameter");

	xml_node<>* child_node = root_node->first_node("ImageLibrary");
	if (child_node != nullptr)
		GetImageLibrary(child_node);

	child_node = root_node->first_node("VideoLibrary");
	if (child_node != nullptr)
		GetVideoLibrary(child_node);

	child_node = root_node->first_node("EffectLibrary");
	if (child_node != nullptr)
		GetEffectLibrary(child_node);

	child_node = root_node->first_node("Database");
	if (child_node != nullptr)
		GetDatabaseParameter(child_node);

	child_node = root_node->first_node("Diaporama");
	if (child_node != nullptr)
		GetDiaporamaParameter(child_node);

	child_node = root_node->first_node("Geolocalisation");
	if (child_node != nullptr)
		GetGeolocalisationServer(child_node);

	child_node = root_node->first_node("Thumbnail");
	if (child_node != nullptr)
		GetThumbnail(child_node);

	child_node = root_node->first_node("Icon");
	if (child_node != nullptr)
		GetIconParameter(child_node);
        
	child_node = root_node->first_node("VideoEffect");
	if (child_node != nullptr)
		GetVideoEffectParameter(child_node);

	child_node = root_node->first_node("Window");
	if (child_node != nullptr)
		GetWindowParameter(child_node);
}

void CRegardsConfigParam::SetIconParameter(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("Ratio", to_string(iconSizeRatio)));
}

void CRegardsConfigParam::GetIconParameter(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("Ratio");
	if (child_node != nullptr)
	{
		
		
		iconSizeRatio = static_cast<float>(atof(child_node->value()));
	}
}


void CRegardsConfigParam::SetDatabaseParameter(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("InMemory", to_string(dataInMemory)));
	sectionPosition->append_node(node("Language", to_string(numLanguage)));
}

void CRegardsConfigParam::GetDatabaseParameter(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("InMemory");
	if (child_node != nullptr)
	{
		
		
		dataInMemory = atoi(child_node->value());
	}

	child_node = position_node->first_node("Language");
	if (child_node != nullptr)
	{
		
		
		numLanguage = atoi(child_node->value());
	}
}

void CRegardsConfigParam::SetDiaporamaParameter(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("Time", to_string(diaporamaTime)));
	sectionPosition->append_node(node("Effect", to_string(diaporamaEffect)));
	sectionPosition->append_node(node("Fullscreen", to_string(diaporamaFullscreen)));
	sectionPosition->append_node(node("MusicDiaporama", musicDiaporama));
}

void CRegardsConfigParam::GetDiaporamaParameter(xml_node<>* position_node)
{
	wxString value;
	wxString node_name;
	xml_node<>* child_node = position_node->first_node("Time");
	if (child_node != nullptr)
	{
		
		node_name = child_node->name();
		diaporamaTime = atoi(child_node->value());
	}

	child_node = position_node->first_node("Effect");
	if (child_node != nullptr)
	{
		
		node_name = child_node->name();
		diaporamaEffect = atoi(child_node->value());
	}

	child_node = position_node->first_node("Fullscreen");
	if (child_node != nullptr)
	{
		
		node_name = child_node->name();
		diaporamaFullscreen = atoi(child_node->value());
	}

	child_node = position_node->first_node("MusicDiaporama");
	if (child_node != nullptr)
	{
		
		node_name = child_node->name();
		musicDiaporama = child_node->value();
	}
}

int CRegardsConfigParam::GetDetectOrientation()
{
	return detectOrientation; // diaporamaEffect;
}

void CRegardsConfigParam::SetDectectOrientation(const int& detectOrientation)
{
	this->detectOrientation = detectOrientation;
}

int CRegardsConfigParam::GetDiaporamaTransitionEffect()
{
	return diaporamaEffect;
}

void CRegardsConfigParam::SetDiaporamaTransitionEffect(const int& diaporamaEffect)
{
	this->diaporamaEffect = diaporamaEffect;
}

int CRegardsConfigParam::GetDiaporamaFullscreen()
{
	return diaporamaFullscreen;
}

void CRegardsConfigParam::SetDiaporamaFullscreen(const int& diaporamaFullscreen)
{
	this->diaporamaFullscreen = diaporamaFullscreen;
}

void CRegardsConfigParam::SetGeolocalisationServer(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("Url", geolocUrl));
}

void CRegardsConfigParam::GetGeolocalisationServer(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("Url");
	if (child_node != nullptr)
	{
		
		
		geolocUrl = child_node->value();
	}
}

void CRegardsConfigParam::SetThumbnail(xml_node<>* sectionPosition)
{
	sectionPosition->append_node(node("Quality", to_string(thumbnailQuality)));
	sectionPosition->append_node(node("IconeCache", to_string(thumbnailIconeCache)));
	sectionPosition->append_node(node("FacePictureSize", to_string(pictureSize)));
	sectionPosition->append_node(node("BufferCache", to_string(bufferSize)));
	
}

void CRegardsConfigParam::GetThumbnail(xml_node<>* position_node)
{
	wxString value;
	wxString nodeName;
	xml_node<>* child_node = position_node->first_node("Quality");
	if (child_node != nullptr)
	{
		
		
		thumbnailQuality = atoi(child_node->value());
	}

	child_node = position_node->first_node("IconeCache");
	if (child_node != nullptr)
	{
		
		
		thumbnailIconeCache = atoi(child_node->value());
	}

	child_node = position_node->first_node("FacePictureSize");
	if (child_node != nullptr)
	{
		
		
		pictureSize = atoi(child_node->value());
	}

	child_node = position_node->first_node("BufferCache");
	if (child_node != nullptr)
	{
		
		
		bufferSize = atoi(child_node->value());
	}
}
