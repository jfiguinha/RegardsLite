#pragma once
#include "ConfigParam.h"
using namespace rapidxml;


class CVideoEffectParameter;

class CRegardsConfigParam : public CConfigParam
{
public:
	CRegardsConfigParam();
	~CRegardsConfigParam() override;

	int GetThumbnailOpenCV();
	void SetThumbnailOpenCV(const int& isThumbnailOpenCV);

	int GetBufferSize();
	void SetBufferSize(const int& value);

	int GetInterpolationType();
	void SetInterpolationType(const int& numInterpolation);

	int GetPreviewLibrary();
	void SetPreviewLibrary(const int& numLib);

	bool GetDatabaseInMemory();
	void SetDatabaseInMemory(const int& value);

	float GetIconSizeRatio();
	void SetIconSizeRatio(const float& ratio);

	int GetAutoConstrast();
	void SetAutoConstrast(const int& autoContrast);

	int GetFaceDetection();
	void SetFaceDetection(const int& faceDetection);

	int GetVideoLibrary();
	void SetVideoLibrary(const int& numLib);

	int GetInverseVideoRotation();
	void SetInverseVideoRotation(const int& inverseRot);

	int GetSuperResolutionType();
	void SetSuperResolutionType(const int& value);

	int GetUseSuperResolution();
	void SetUseSuperResolution(const int& value);

	wxString GetMusicDiaporama();
	void SetMusicDiaporama(const wxString& musicDiaporama);

	int GetEffectLibrary();
	void SetEffectLibrary(const int& numLib);

	int GetSoundVolume();
	void SetSoundVolume(const int& soundVolume);

	int GetOpenCLPlatformIndex();
	void SetOpenCLPlatformIndex(const int& numIndex);

	void SetFastDetectionFace(const int& fastDetection);
	int GetFastDetectionFace();

	int GetFaceVideoDetection();
	void SetFaceVideoDetection(const int& videoFaceDetection);

	int GetFaceDetectionPictureSize();
	void SetFaceDetectionPictureSize(const int& numIndex);

	wxString GetOpenCLPlatformName();
	void SetOpenCLPlatformName(const wxString& platformName);

	int GetOpenCLLoadFromBinaries();
	void SetOpenCLLoadFromBinaries(const int& loadFromBinaries);

	int GetEffect();
	void SetEffect(const int& numEffect);

	wxString GetUrlServer();
	int GetNbGpsIterationByMinute();

	int GetDiaporamaTime();
	void SetDiaporamaTime(const int& diaporamaTime);

	int GetDiaporamaTransitionEffect();
	void SetDiaporamaTransitionEffect(const int& diaporamaEffect);

	int GetDiaporamaFullscreen();
	void SetDiaporamaFullscreen(const int& diaporamaFullscreen);

	int GetThumbnailQuality();
	void SetThumbnailQuality(const int& quality);

	int GetThumbnailIconeCache();
	void SetThumbnailIconeCache(const int& iconeCache);

	int GetThumbnailProcess();
	void SetThumbnailProcess(const int& nbProcess);

	int GetFaceProcess();
	void SetFaceProcess(const int& nbProcess);

	int GetFaceOpenCLProcess();
	void SetFaceOpenCLProcess(const int& openclProcess);

	int GetExifProcess();
	void SetExifProcess(const int& nbProcess);

	int GetNumLanguage();
	void SetNumLanguage(const int& numLanguage);

	bool GetIsOpenCLSupport();
	void SetIsOpenCLSupport(const int& openCLSupport);


	int GetIsCudaSupport();
	void SetIsCudaSupport(const int& openCLSupport);
    
	int GetIsUseCuda();
	void SetIsUseCuda(const int& useCuda);

	void SetHardwareDecoder(const wxString& hardwareDecoder);
	wxString GetHardwareDecoder();

	void SetHardwareEncoder(const wxString& hardwareEncoder);
	wxString GetHardwareEncoder();

	int GetDetectOrientation();
	void SetDectectOrientation(const int& detectOrientation);

	bool GetIsOpenCLOpenGLInteropSupport();
	void SetIsOpenCLOpenGLInteropSupport(const int& openCLOpenGLInteropSupport);
    
	wxString GetOpenGLOutputColor();
	void SetOpenGLOutputColor(const wxString& openGLOutputColor);

	int GetSkinWindowMode();
	void SetSkinWindowMode(const int& skinWindowMode);
    
    CVideoEffectParameter * GetVideoEffectParameter();
	void SetVideoEffectParameter(const CVideoEffectParameter * videoEffect);

protected:
	void LoadParameter() override;
	void SaveParameter() override;

	void SetIconParameter(xml_node<>* sectionPosition);
	void GetIconParameter(xml_node<>* position_node);

	void SetImageLibrary(xml_node<>* sectionPosition);
	void GetImageLibrary(xml_node<>* position_node);

	void SetDatabaseParameter(xml_node<>* sectionPosition);
	void GetDatabaseParameter(xml_node<>* position_node);

	void SetEffectLibrary(xml_node<>* sectionPosition);
	void GetEffectLibrary(xml_node<>* position_node);

	void SetVideoLibrary(xml_node<>* sectionPosition);
	void GetVideoLibrary(xml_node<>* position_node);

	void SetDiaporamaParameter(xml_node<>* sectionPosition);
	void GetDiaporamaParameter(xml_node<>* position_node);

	void SetGeolocalisationServer(xml_node<>* sectionPosition);
	void GetGeolocalisationServer(xml_node<>* position_node);

	void SetThumbnail(xml_node<>* sectionPosition);
	void GetThumbnail(xml_node<>* position_node);

	void SetWindowParameter(xml_node<>* sectionPosition);
	void GetWindowParameter(xml_node<>* position_node);
    
    void SetVideoEffectParameter(xml_node<>* sectionPosition);
	void GetVideoEffectParameter(xml_node<>* position_node);

	int pictureSize;
	int numLibPreview;
	int numLibEffect;
	int numLibVideo;
	int numEffect;
	int openCLNumIndex;
	int numLanguage;
	int numInterpolation;
	int detectOrientation;

	//Diaporama
	int diaporamaTime;
	int diaporamaEffect;
	int diaporamaFullscreen;

	int thumbnailQuality;
	int thumbnailIconeCache;

	float iconSizeRatio;
	wxString openCLPlatformName;
	wxString geolocUrl;

	int dataInMemory;
	int inverseRotation = 0;

	int nbProcessThumbnail;
	int nbProcessExif;
	int nbProcessFace;
	int nbGpsFileByMinute;

	int loadFromBinaries = 0;
	int openCLSupport = 0;
	int openCLOpenGLInteropSupport = 0;
	int openCLFaceSupport = 0;

	int cudaSupport = 0;
    int useCuda = 0;

	int autoContrast = 0;
	int soundVolume = 100;

	int numSuperResolution = 0;
	int useSuperResolution = 0;
	int skinWindowMode = 0;

	int videoFaceDetection = 0;
	int faceDetection = 1;
	int fastFaceDetection = 1;
	wxString videoDecoderHardware;
	wxString videoEncoderHardware;

	wxString musicDiaporama;
	int bufferSize;
	int isThumbnailOpenCV = 1;
    wxString openGLOutputColor = "RGBA";
    CVideoEffectParameter * videoEffectParameter = nullptr;
};
