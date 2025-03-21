// ReSharper disable All
#include <header.h>
#include "ThumbnailEffect.h"
#include "InfosSeparationBarEffect.h"
#include <ThumbnailDataStorage.h>
#include <ParamInit.h>
#include <RegardsConfigParam.h>
#include <LibResource.h>
#include <FiltreEffet.h>
#include "wx/stdpaths.h"
#include "ScrollbarHorizontalWnd.h"
#include "ScrollbarWnd.h"
#include <libPicture.h>
#include <FilterData.h>
#include <LoadingResource.h>
#include <picture_id.h>
#include <ImageLoadingFormat.h>


#include <effect_id.h>
using namespace Regards::Window;
using namespace Regards::FiltreEffet;
using namespace Regards::Control;
using namespace Regards::Picture;
wxDEFINE_EVENT(EVENT_ICONEUPDATE, wxCommandEvent);


class CThreadBitmapEffect
{
public:
	CThreadBitmapEffect(): numIcone(0), photoId(0)
	{
		thumbnailData = nullptr;
		_thread = nullptr;
		thumbnail = nullptr;
		imageLoading = nullptr;
	}

	~CThreadBitmapEffect()
	{
	}

	wxString filename;
	wxString filepath;
	int numIcone;
	int photoId;
	cv::Mat picture;
	CThumbnailDataStorage* thumbnailData;
	CImageLoadingFormat* imageLoading;
	thread* _thread;
	CThumbnailEffect* thumbnail;
};

CThumbnailEffect::CThumbnailEffect(wxWindow* parent, const wxWindowID id, const CThemeThumbnail& themeThumbnail,
                                   const bool& testValidity)
	: CThumbnailVerticalSeparator(parent, id, themeThumbnail, testValidity)
{
	imageLoading = nullptr;
	isAllProcess = true;
	processIdle = false;
	moveOnPaint = false;
	barseparationHeight = 40;
	config = nullptr;
	config = CParamInit::getInstance();
	colorEffect = CLibResource::LoadStringFromResource("LBLCOLOREFFECT", 1); //"Color Effect";
	convolutionEffect = CLibResource::LoadStringFromResource("LBLCONVOLUTIONEFFECT", 1); //"Convolution Effect";
	specialEffect = CLibResource::LoadStringFromResource("LBLSPECIALEFFECT", 1); //"Special Effect";
	histogramEffect = CLibResource::LoadStringFromResource("LBLHISTOGRAMEFFECT", 1); //"Special Effect";
	blackRoomEffect = CLibResource::LoadStringFromResource("LBLBLACKROOM", 1);
	videoLabelEffect = CLibResource::LoadStringFromResource("LBLVIDEOEFFECT", 1);
	//hdrEffect = CLibResource::LoadStringFromResource("LBLHDREFFECT",1);
	rotateEffect = CLibResource::LoadStringFromResource("LBLROTATEEFFECT", 1);

	//hdrEffect = "HDR Effect";
	//rotateEffect = "Rotate Effect";
	Connect(EVENT_ICONEUPDATE, wxCommandEventHandler(CThumbnailEffect::UpdateRenderIcone));
}

CThumbnailEffect::~CThumbnailEffect(void)
{
	for (CInfosSeparationBar* infosSeparationBar : listSeparator)
	{
		delete(infosSeparationBar);
		infosSeparationBar = nullptr;
	}
	listSeparator.clear();

	if (imageLoading != nullptr)
		delete imageLoading;
}

wxString CThumbnailEffect::GetFilename()
{
	return filename;
}

bool CThumbnailEffect::ItemCompFonct(int x, int y, CIcone *  icone, CWindowMain* parent) /* Définit une fonction. */
{
	wxRect rc = icone->GetPos();
	if ((rc.x < x && x < (rc.width + rc.x)) && (rc.y < y && y < (rc.height + rc.y)))
	{
		return true;
	}
	return false;
}

CIcone * CThumbnailEffect::FindElement(const int& xPos, const int& yPos)
{
	int x = xPos + posLargeur;
	int y = yPos + posHauteur;
	pItemCompFonct _pf = &ItemCompFonct;
	return iconeList->FindElement(x, y, &_pf, this);
}


CInfosSeparationBarEffect* CThumbnailEffect::CreateNewSeparatorBar(const wxString& libelle)
{
	auto infosSeparationBar = new CInfosSeparationBarEffect(themeThumbnail.themeSeparation);
	infosSeparationBar->SetTitle(libelle);
	infosSeparationBar->SetWidth(GetWindowWidth());
	listSeparator.push_back(infosSeparationBar);
	return infosSeparationBar;
}


float CThumbnailEffect::CalculRatio(const int& width, const int& height, const int& tailleBitmapWidth,
                                    const int& tailleBitmapHeight)
{
	float new_ratio;
	//int left = 0;
	//int top = 0;

	if (width > height)
		new_ratio = static_cast<float>(tailleBitmapWidth) / static_cast<float>(width);
	else
		new_ratio = static_cast<float>(tailleBitmapHeight) / static_cast<float>(height);

	if (height * new_ratio > tailleBitmapHeight)
	{
		new_ratio = static_cast<float>(tailleBitmapHeight) / static_cast<float>(height);
	}
	else
	{
		if ((width * new_ratio) > tailleBitmapWidth)
		{
			new_ratio = static_cast<float>(tailleBitmapHeight) / static_cast<float>(width);
		}
	}

	return new_ratio;
}

void CThumbnailEffect::GetBitmapDimension(const int& width, const int& height, int& tailleAffichageBitmapWidth,
                                          int& tailleAffichageBitmapHeight, float& newRatio)
{
	newRatio = CalculRatio(width, height, tailleAffichageBitmapWidth, tailleAffichageBitmapHeight);

	tailleAffichageBitmapWidth = static_cast<int>(float(width) * newRatio);
	tailleAffichageBitmapHeight = static_cast<int>(float(height) * newRatio);
}

void CThumbnailEffect::SetFile(const wxString& filename, CImageLoadingFormat* imageLoading)
{
	auto iconeListLocal = new CIconeList();
	CIconeList* oldIconeList = nullptr;
	threadDataProcess = false;
	processIdle = false;
	this->imageLoading = imageLoading;
	CLoadingResource loadingResource;
	this->filename = filename;
	auto backcolor = CRgbaquad(themeThumbnail.colorBack.Red(), themeThumbnail.colorBack.Green(),
	                           themeThumbnail.colorBack.Blue());
	InitScrollingPos();
	//EraseThumbnailList();
	CreateOrLoadStorageFile();

	for (CInfosSeparationBar* infosSeparationBar : listSeparator)
	{
		delete(infosSeparationBar);
		infosSeparationBar = nullptr;
	}
	listSeparator.clear();

	isAllProcess = false;

	CLibPicture picture;
	int format = picture.TestImageFormat(filename);
	if (picture.TestIsVideo(filename))
	{
		CInfosSeparationBarEffect* videoEffect = CreateNewSeparatorBar(videoLabelEffect);
		int numElement = iconeListLocal->GetNbElement();

		wxImage pBitmap = loadingResource.LoadImageResource("IDB_BLACKROOM");
		auto thumbnailData = new CThumbnailDataStorage(CFiltreData::GetFilterLabel(IDM_FILTRE_VIDEO));
		videoEffect->AddPhotoToList(numElement);

		thumbnailData->SetNumPhotoId(IDM_FILTRE_VIDEO);

		thumbnailData->SetBitmap(CLibPicture::mat_from_wx(pBitmap));

		auto pBitmapIcone = new CIcone();
		pBitmapIcone->SetNumElement(thumbnailData->GetNumElement());
		pBitmapIcone->SetData(thumbnailData);
		pBitmapIcone->SetTheme(themeThumbnail.themeIcone);
		iconeListLocal->AddElement(pBitmapIcone);

		isAllProcess = true;
	}
	else
	{
		CInfosSeparationBarEffect* infosSeparationColorEffect = CreateNewSeparatorBar(colorEffect);
		CInfosSeparationBarEffect* infosSeparationConvolutionEffect = CreateNewSeparatorBar(convolutionEffect);
		CInfosSeparationBarEffect* infosSeparationSpecialEffect = CreateNewSeparatorBar(specialEffect);
		CInfosSeparationBarEffect* infosSeparationHistogramEffect = CreateNewSeparatorBar(histogramEffect);
		CInfosSeparationBarEffect* infosSeparationRotateEffect = CreateNewSeparatorBar(rotateEffect);
		//CInfosSeparationBarEffect * infosSeparationHDREffect = CreateNewSeparatorBar(hdrEffect);

		int i = 0;
		auto colorQuad = CRgbaquad(themeThumbnail.colorBack.Red(), themeThumbnail.colorBack.Green(),
		                           themeThumbnail.colorBack.Blue());

		for (int numEffect = FILTER_START; numEffect < FILTER_END; numEffect++)
		{
			int numElement = iconeListLocal->GetNbElement();
			auto thumbnailData = new CThumbnailDataStorage(filename);
			thumbnailData->SetNumElement(i++);
			thumbnailData->SetNumPhotoId(numEffect);

			switch (numEffect)
			{
			/*
		case IDM_FILTER_BM3D:
			{
				CRegardsBitmap* pBitmap = loadingResource.LoadRegardsBmpResource("IDB_FILTRE_BM3D");
				thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
				infosSeparationConvolutionEffect->AddPhotoToList(numElement);
				CImageLoadingFormat image;
				image.SetPicture(pBitmap, true);
				thumbnailData->SetBitmap(&image);
				break;
			}
			*/
            case IDM_INPAINT:
            {
                cv::Mat pBitmap = loadingResource.LoadResourceCV("IDB_INPAINT");
                thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
                infosSeparationSpecialEffect->AddPhotoToList(numElement);
                thumbnailData->SetBitmap(pBitmap);
                break;
            }


			case IDM_CROP:
				{
					cv::Mat pBitmap = loadingResource.LoadResourceCV("IDB_CROP");
					thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
					infosSeparationSpecialEffect->AddPhotoToList(numElement);
					thumbnailData->SetBitmap(pBitmap);
					break;
				}

			case IDM_WAVE_EFFECT:
				thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
				infosSeparationSpecialEffect->AddPhotoToList(numElement);
				break;

			case IDM_FILTRELENSFLARE:
				thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
				infosSeparationSpecialEffect->AddPhotoToList(numElement);
				break;

			case IDM_FILTRELENSCORRECTION:
				thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
				infosSeparationSpecialEffect->AddPhotoToList(numElement);
				break;

			default:
				thumbnailData->SetFilename(CFiltreData::GetFilterLabel(numEffect));
				int typeEffect = CFiltreData::GetTypeEffect(numEffect);


				switch (typeEffect)
				{
				case SPECIAL_EFFECT:
					infosSeparationSpecialEffect->AddPhotoToList(numElement);
					break;

				case COLOR_EFFECT:
					infosSeparationColorEffect->AddPhotoToList(numElement);
					break;

				case CONVOLUTION_EFFECT:
					infosSeparationConvolutionEffect->AddPhotoToList(numElement);
					break;

				case HISTOGRAM_EFFECT:
					infosSeparationHistogramEffect->AddPhotoToList(numElement);
					break;

				case ROTATE_EFFECT:
					infosSeparationRotateEffect->AddPhotoToList(numElement);
					break;
				default: ;

				/*
	        case HDR_EFFECT:
	            infosSeparationHDREffect->AddPhotoToList(numElement);
	            break;
				*/
				}
				break;
			}
			thumbnailData->SetNumPhotoId(numEffect);
			auto pBitmapIcone = new CIcone();
			pBitmapIcone->SetNumElement(thumbnailData->GetNumElement());
			pBitmapIcone->SetData(thumbnailData);
			pBitmapIcone->SetTheme(themeThumbnail.themeIcone);
			iconeListLocal->AddElement(pBitmapIcone);
		}


		if (format == 4)
		{
			CInfosSeparationBarEffect* blackRoom = CreateNewSeparatorBar(blackRoomEffect);
			int numElement = iconeListLocal->GetNbElement();
			auto thumbnailData = new CThumbnailDataStorage(filename);
			cv::Mat image = loadingResource.LoadResourceCV("IDB_BLACKROOM");
			thumbnailData = new CThumbnailDataStorage(CFiltreData::GetFilterLabel(IDM_DECODE_RAW));
			blackRoom->AddPhotoToList(numElement);

			thumbnailData->SetNumPhotoId(IDM_DECODE_RAW);

			thumbnailData->SetBitmap(image);

			auto pBitmapIcone = new CIcone();
			pBitmapIcone->SetNumElement(thumbnailData->GetNumElement());
			pBitmapIcone->SetData(thumbnailData);
			pBitmapIcone->SetTheme(themeThumbnail.themeIcone);
			iconeListLocal->AddElement(pBitmapIcone);
		}
	}

	oldIconeList = iconeList;
	iconeList = iconeListLocal;

	nbElementInIconeList = iconeList->GetNbElement();

	//EraseThumbnailList(oldIconeList);
    oldIconeList->EraseThumbnailListWithIcon();
    delete oldIconeList;
    //oldIconeList->EraseThumbnailList();
    //delete oldIconeList;

	threadDataProcess = true;
	processIdle = true;

	UpdateScroll();
	
    ResizeThumbnail();

	needToRefresh = true;
}

void CThumbnailEffect::LoadPicture(void* param)
{
	auto threadLoadingBitmap = static_cast<CThreadBitmapEffect*>(param);
	if (threadLoadingBitmap == nullptr)
		return;

	bool deleteData = false;
	//CImageLoadingFormat thumbnail; 
	CSqlThumbnail sqlThumbnail;
	auto colorQuad = CRgbaquad(threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Red(),
	                           threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Green(),
	                           threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Blue());
	CImageLoadingFormat* thumbnail;

	if (threadLoadingBitmap->imageLoading == nullptr)
	{
		deleteData = true;
		thumbnail = sqlThumbnail.GetPictureThumbnail(threadLoadingBitmap->filepath);
	}
	else
		thumbnail = threadLoadingBitmap->imageLoading;


	if (thumbnail != nullptr)
	{
		//thumbnail.SetPicture(bitmap);  
		auto color_quad = CRgbaquad(threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Red(),
		                            threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Green(),
		                            threadLoadingBitmap->thumbnail->themeThumbnail.colorBack.Blue());
		auto filtre = new CFiltreEffet(color_quad, false, false, thumbnail);

		switch (threadLoadingBitmap->photoId)
		{
		case IDM_WAVE_EFFECT:
			filtre->WaveFilter(20, 20, thumbnail->GetHeight() / 2, 2, 20);
			break;

		case IDM_FILTRELENSFLARE:
			filtre->LensFlare(20, 20, 20, 1, 20, 45, 20);
			break;

		default:
			{
				CEffectParameter* effect = CFiltreData::GetDefaultEffectParameter(
					threadLoadingBitmap->thumbnailData->GetNumPhotoId());
				filtre->RenderEffect(threadLoadingBitmap->thumbnailData->GetNumPhotoId(), effect);
				if (effect != nullptr)
					delete effect;
			}
			break;
		}

		threadLoadingBitmap->picture = filtre->GetBitmap(true);
		delete filtre;
	}

	if (deleteData)
		delete thumbnail;

	auto event = new wxCommandEvent(EVENT_ICONEUPDATE);
	event->SetClientData(threadLoadingBitmap);
	wxQueueEvent(threadLoadingBitmap->thumbnail, event);
}

void CThumbnailEffect::ProcessIdle()
{
	//printf("CThumbnailEffect::ProcessIdle() \n");
	//int nbProcesseur = thread::hardware_concurrency();
	int nbProcesseur = 1;
	CRegardsConfigParam* config = CParamInit::getInstance();
	if (config != nullptr)
		nbProcesseur = config->GetThumbnailProcess();

	if (isAllProcess)
	{
		processIdle = false;
		return;
	}

	while (nbProcess < nbProcesseur)
	{
		for (auto i = 0; i < nbElementInIconeList; i++)
		{
			CIcone *  icone = iconeList->GetElement(i);
			if (icone != nullptr)
			{
				CThumbnailData* pThumbnailData = icone->GetData();
				if (pThumbnailData != nullptr)
				{
					bool isLoad = pThumbnailData->IsLoad();
					bool isProcess = pThumbnailData->IsProcess();
					if (!isLoad && !isProcess)
					{
						auto pLoadBitmap = new CThreadBitmapEffect();
						pLoadBitmap->thumbnail = this;
						pLoadBitmap->thumbnailData = static_cast<CThumbnailDataStorage*>(
							pThumbnailData);
						pLoadBitmap->filepath = filename;
						pLoadBitmap->filename = pThumbnailData->GetFilename();
						pLoadBitmap->numIcone = i;
						pLoadBitmap->photoId = pThumbnailData->GetNumPhotoId();
						pLoadBitmap->imageLoading = imageLoading;
						pLoadBitmap->_thread = new thread(LoadPicture, pLoadBitmap);
						nbProcess++;
						pThumbnailData->SetIsProcess(true);
					}
				}
			}
		}
	}
	//Test si tout a été fait
	isAllProcess = true;
	for (int i = 0; i < nbElementInIconeList; i++)
	{
		CIcone *  icone = iconeList->GetElement(i);
		if (icone != nullptr)
		{
			bool isLoad = false;
			bool isProcess = false;
			CThumbnailData* pThumbnailData = icone->GetData();
			if (pThumbnailData != nullptr)
			{
				isLoad = pThumbnailData->IsLoad();
				isProcess = pThumbnailData->IsProcess();
			}

			if (!isLoad && !isProcess)
			{
				isAllProcess = false;
			}
		}
		if (!isAllProcess)
			break;
	}
}

void CThumbnailEffect::UpdateRenderIcone(wxCommandEvent& event)
{
	nbProcess--;
        if(nbProcess < 0)
            nbProcess = 0;

	auto threadLoadingBitmap = static_cast<CThreadBitmapEffect*>(event.GetClientData());
	if (threadLoadingBitmap == nullptr)
	{
		return;
	}


	if (threadDataProcess != false && threadLoadingBitmap != nullptr)
	{
		if (filename == threadLoadingBitmap->filepath)
		{
			if (!threadLoadingBitmap->picture.empty())
			{
				if (threadLoadingBitmap->numIcone >= nbElementInIconeList)
					return;

				CIcone *  icone = iconeList->GetElement(threadLoadingBitmap->numIcone);
				if (icone != nullptr)
				{
					bool needToRefresh = false;
					CThumbnailData* pThumbnailData = icone->GetData();
					if (pThumbnailData->GetFilename() == threadLoadingBitmap->filename && icone !=
						nullptr
						&& pThumbnailData != nullptr)
					{
						pThumbnailData->SetIsProcess(false);
						pThumbnailData->SetBitmap(threadLoadingBitmap->picture);
						pThumbnailData->SetIsLoading(false);
						if (!render)
						{
							needToRefresh = true;
						}
					}
					if (needToRefresh)
						needToRefresh = true;
				}
			}
		}
	}

	if (threadLoadingBitmap->_thread != nullptr)
	{
		if (threadLoadingBitmap->_thread->joinable())
			threadLoadingBitmap->_thread->join();

		delete threadLoadingBitmap->_thread;

		threadLoadingBitmap->_thread = nullptr;
	}


	if (threadLoadingBitmap != nullptr)
	{
		delete threadLoadingBitmap;
		threadLoadingBitmap = nullptr;
	}

	this->Refresh();
}

void CThumbnailEffect::UpdateScroll()
{
	//bool update = false;
	thumbnailSizeX = 0;
	thumbnailSizeY = 0;
	if (GetWindowWidth() <= 0)
		return;

	for (CInfosSeparationBar* infosSeparationBar : listSeparator)
	{
		int nbElement = static_cast<int>(infosSeparationBar->listElement.size());

		int nbElementByRow = (GetWindowWidth()) / themeThumbnail.themeIcone.GetWidth();
		if ((nbElementByRow * themeThumbnail.themeIcone.GetWidth()) < (GetWindowWidth()))
			nbElementByRow++;

		int nbElementEnY = static_cast<int>(infosSeparationBar->listElement.size()) / nbElementByRow;
		if (nbElementEnY * nbElementByRow < infosSeparationBar->listElement.size())
			nbElementEnY++;

		if (nbElement < nbElementByRow)
			nbElementByRow = nbElement;

		int sizeX = nbElementByRow * themeThumbnail.themeIcone.GetWidth();
		if (sizeX > thumbnailSizeX)
			thumbnailSizeX = nbElementByRow * themeThumbnail.themeIcone.GetWidth();
		thumbnailSizeY += nbElementEnY * themeThumbnail.themeIcone.GetHeight() + infosSeparationBar->
			GetHeight();
	}

	//int nbElement = pIconeList.size();
	wxWindow* parent = this->GetParent();

	if (parent != nullptr)
	{
		auto controlSize = new CControlSize();
		wxCommandEvent evt(wxEVENT_SETCONTROLSIZE);
		controlSize->controlWidth = thumbnailSizeX;
		controlSize->controlHeight = thumbnailSizeY;
		evt.SetClientData(controlSize);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}

	if (parent != nullptr)
	{
		auto size = new wxSize();
		wxCommandEvent evt(wxEVENT_SETPOSITION);
		size->x = posLargeur;
		size->y = posHauteur;
		evt.SetClientData(size);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}
