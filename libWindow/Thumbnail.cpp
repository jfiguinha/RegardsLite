#include "header.h"
#include "Thumbnail.h"
#include <FileUtility.h>
#include <wxmd5.h>
#include "ScrollbarHorizontalWnd.h"
#include "ScrollbarWnd.h"
#include <ParamInit.h>
#include <RegardsConfigParam.h>
#include <SqlPhotosWithoutThumbnail.h>
#include <ImageLoadingFormat.h>
#include <ImageVideoThumbnail.h>
#include <SqlThumbnailVideo.h>
#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include "LibResource.h"
#include <ThumbnailData.h>
#include "ThumbnailMessage.h"
#include <libPicture.h>
#include <ThumbnailDataStorage.h>
#include <ThreadLoadingBitmap.h>
using namespace Regards::Picture;
using namespace Regards::Window;


class CImageLoadingFormat;
extern bool preprocessisAvailable;
extern std::mutex muProcessAvailable;
#define TIMER_LOADING 4
#define TIMER_ANIMATION 6
#define TIMER_CLICK 7
#define TIMER_MOVE 3

#define TIMER_REFRESH_THUMBNAIL 10
#define TIMER_REFRESH_ACTIF 8
#define TIMER_REFRESH_SELECT 9
#define TIMER_TIME_REFRESH 1000 / 25


class CListToClean
{
public:
	int type = 0;
	CIconeList* list;
	std::vector<CIcone*> pIconeListToClean;
	std::time_t timeToAdd;
};


#define WM_NEWFOLDER 501
#define WM_DISPLAYPICTURE 502
#define WM_STOPPICTURELOAD 503
#define WM_UPDATESTATUSBAR 504
#define WM_LOADTHUMBNAIL 505

bool CThumbnail::ItemCompFonct(int xPos, int yPos, CIcone *  icone, CWindowMain* parent) /* Définit une fonction. */
{
	wxRect rc = icone->GetPos();
	if ((rc.x < xPos && xPos < rc.width) && (rc.y < yPos && yPos < rc.height))
	{
		return true;
	}
	return false;
}


CIcone *  CThumbnail::FindElement(const int& xPos, const int& yPos)
{
	pItemCompFonct _pf = &ItemCompFonct;
	return iconeList->FindElement(xPos, yPos, &_pf, this);
}


void CThumbnail::GenerateCleanupListFile(std::vector<CIcone*>& pIconeListToClean)
{
	for (CIcone* ico : pIconeList)
	{
		CThumbnailData* thumbnailData = (CThumbnailData*)ico->GetData();
		wxString filename = thumbnailData->GetFilename();
		bool find = iconeList->FindElement(filename);
		if (!find)
			pIconeListToClean.push_back(ico);
	}

	//------------------------------------
	for (CIcone* ico : pIconeListToClean)
	{
		CThumbnailData * _clean = (CThumbnailData*)ico->GetData();

		std::vector<CIcone*>::iterator it = std::find_if(pIconeList.begin(), pIconeList.end(), [&](CIcone* e)
			{
				CThumbnailData* thumbnailData = (CThumbnailData*)e->GetData();
				return thumbnailData->GetFilename() == _clean->GetFilename();

			});

		if (it != pIconeList.end())
			pIconeList.erase(it);
	}
}

void CThumbnail::EraseThumbnail(wxCommandEvent& event)
{
	wxString title = CLibResource::LoadStringFromResource(L"LBLSTOPALLPROCESS", 1);
	wxString message = CLibResource::LoadStringFromResource(L"LBLSTOPPROCESS", 1);
	StopAllProcess(title, message, this);

    stopToGetNbElement = false;


	CSqlThumbnail sqlThumbnail;
	sqlThumbnail.EraseThumbnail();

	CSqlThumbnailVideo sqlThumbnailVideo;
	sqlThumbnailVideo.EraseThumbnail();

	for (int i = 0; i < nbElementInIconeList; i++)
	{
		CIcone *  pIcone = iconeList->GetElement(i);
		if (pIcone != nullptr)
		{
			CThumbnailData* pThumbnailData = pIcone->GetData();
			if (pThumbnailData != nullptr)
			{
				pThumbnailData->InitLoadState();
				wxString filename = pThumbnailData->GetFilename();
				pThumbnailData->SetIsProcess(false);
				pThumbnailData->SetIsLoading(false);
			}
		}
	}

	thumbnailPos = 0;

	SetStopProcess(false);

	wxWindow* window = this->FindWindowById(MAINVIEWERWINDOWID);
	if (window != nullptr)
	{
		wxCommandEvent evt(wxEVENT_REFRESHTHUMBNAIL);
		window->GetEventHandler()->AddPendingEvent(evt);
	}

	//this->Refresh();
}

void CThumbnail::SetCheck(const bool& check)
{
	for (int i = 0; i < nbElementInIconeList; i++)
	{
		CIcone *  pIcone = iconeList->GetElement(i);
		if (pIcone != nullptr)
		{
			pIcone->ShowSelectButton(check);
		}
	}
}

void CThumbnail::GetSelectItem(vector<CThumbnailData*>& vectorData)
{
	for (int i = 0; i < nbElementInIconeList; i++)
	{
		CIcone *  pIcone = iconeList->GetElement(i);
		if (pIcone != nullptr)
		{
			if (pIcone->IsChecked())
				vectorData.push_back(pIcone->GetData());
		}
	}
}

bool CThumbnail::ItemCompFonctPath(wxString filepath, CIcone *  icone)
/* Définit une fonction. */
{
	CThumbnailData* pThumbnailData = icone->GetData();
	if (pThumbnailData->GetFilename() == filepath)
		return true;
	return false;
}

CIcone *  CThumbnail::GetIconeByPath(const wxString & filepath)
{
	pItemStringCompFonct _pf = &ItemCompFonctPath;
	return iconeList->FindElement(filepath, &_pf);
}


bool CThumbnail::ItemCompFonctPhotoId(int xPos, int yPos, CIcone *  icone, CWindowMain* parent)
/* Définit une fonction. */
{
	CThumbnailData* pThumbnailData = icone->GetData();
	if (pThumbnailData->GetNumPhotoId() == xPos)
		return true;
	return false;
}

CIcone *  CThumbnail::GetIconeById(const int& idPhoto)
{
	pItemCompFonct _pf = &ItemCompFonctPhotoId;
	return iconeList->FindElement(idPhoto, 0, &_pf, this);
}

int CThumbnail::GetNumItemById(const int& idPhoto)
{
	pItemCompFonct _pf = &ItemCompFonctPhotoId;
	CIcone *  icone = iconeList->FindElement(idPhoto, 0, &_pf, this);
	if (icone != nullptr)
		return icone->GetNumElement();
	return 0;
}

int CThumbnail::GetNumPhotoId(const int& numItem)
{
	return iconeList->GetPhotoId(numItem);
}

void CThumbnail::SetActifItem(const int& idPhoto, const bool& move)
{
	int numItem = GetNumItemById(idPhoto);

	isMovingScroll = move;

	bool refresh = false;

	if (numItem >= nbElementInIconeList)
		return;

	if (numSelectPhotoId != -1)
	{
		CIcone *  numSelect = GetIconeById(numSelectPhotoId);
		if (numSelect != nullptr)
			numSelect->SetSelected(false);
		RefreshIcone(numSelectPhotoId);
		refresh = true;
	}


	if (numActifPhotoId != -1)
	{
		CIcone *  numActif = GetIconeById(numActifPhotoId);
		if (numActif != nullptr)
			numActif->SetSelected(false);
		RefreshIcone(numActifPhotoId);
		refresh = true;
	}

	numActifPhotoId = idPhoto;// iconeList->GetPhotoId(numItem);
	isMovingScroll = false;

	if (move)
	{
		if (numItem == 0)
		{
			if (this->GetParent() != nullptr)
			{
				auto size = new wxSize();
				wxCommandEvent evt(wxEVENT_SETPOSITION);
				size->x = 0;
				size->y = 0;
				evt.SetClientData(size);
				this->GetParent()->GetEventHandler()->AddPendingEvent(evt);

				refresh = true;
			}
			posLargeur = 0;
			posHauteur = 0;
		}
		else
		{
			if (!isMoving)
			{
				wxRect rect;

				if (numActifPhotoId != -1)
				{
					CIcone *  numActif = GetIconeById(numActifPhotoId);
					if (numActif != nullptr)
						rect = numActif->GetPos();
				    RefreshIcone(numActifPhotoId);
					refresh = true;
				}
			}
		}
	}

	numSelectPhotoId = iconeList->GetPhotoId(numItem);

	if (numSelectPhotoId != -1)
	{
		CIcone *  numSelect = GetIconeById(numSelectPhotoId);
		if (numSelect != nullptr)
			numSelect->SetSelected(true);
		RefreshIcone(numSelectPhotoId);
		refresh = true;
	}

	numOldItem = numItem;
	moveOnPaint = true;

	if (refresh)
	{
		needToRefresh = true;
	}
}

int CThumbnail::ImageSuivante()
{
	numOldItem++;
	if (numOldItem == nbElementInIconeList)
		numOldItem = 0;
	return numOldItem;
}


int CThumbnail::GetNumItem()
{
	return numOldItem;
}

int CThumbnail::ImagePrecedente()
{
	numOldItem--;
	if (numOldItem < 0)
		numOldItem = nbElementInIconeList - 1;
	return numOldItem;
}

int CThumbnail::ImageFin()
{
	return nbElementInIconeList - 1;
}

int CThumbnail::ImageDebut()
{
	return 0;
}

int CThumbnail::GetNbElement()
{
	return nbElementInIconeList;
}

wxString CThumbnail::GetFilename(const int& numItem)
{
	wxString filename = "";
	if (iconeList != nullptr)
	{
		filename = iconeList->GetFilename(numItem);
	}
	return filename;
}

void CThumbnail::SetTheme(CThemeThumbnail* theme)
{
	themeThumbnail = *theme;
}

void CThumbnail::ZoomOn()
{
	positionSize++;
	if (positionSize > Max)
		positionSize = Max;

	int sizeIcone = TabSize[positionSize - 1];
	SetIconeSize(sizeIcone, sizeIcone);

	needToRefresh = true;
}

void CThumbnail::ZoomOff()
{
	positionSize--;
	if (positionSize < 1)
		positionSize = 1;

	const int sizeIcone = TabSize[positionSize - 1];
	SetIconeSize(sizeIcone, sizeIcone);

	needToRefresh = true;
}

void CThumbnail::ZoomPosition(const int& position)
{
	positionSize = position + 1;
	if (positionSize < 1)
		positionSize = 1;
	if (positionSize > Max)
		positionSize = Max;

	int sizeIcone = TabSize[positionSize - 1];
	SetIconeSize(sizeIcone, sizeIcone);

	needToRefresh = true;
}


CThumbnail::CThumbnail(wxWindow* parent, wxWindowID id, const CThemeThumbnail& themeThumbnail, const bool& testValidity)
	: CWindowMain("CThumbnail", parent, id)
{
	localid = id;
	controlWidth = 0;
	controlHeight = 0;
	isMoving = 0;
	thumbnailPos = 0;
	oldPosLargeur = 0;
	oldPosHauteur = 0;

	allThreadEnd = true;
	showLoadingBitmap = false;
	stepLoading = 0;
	m_animation = nullptr;
	render = false;
	check = false;
	threadDataProcess = true;
	iconeList = new CIconeList();
	this->testValidity = testValidity;
	thumbnailSizeX = 0;
	thumbnailSizeY = 0;
	posHauteur = 0;
	posLargeur = 0;
	nbLigneX = 0;
	nbLigneY = 0;


	numStartElement = 0;
	nbElementToShow = 0;
	isStoragePt = false;


	//this->statusbar = statusbar;

	this->themeThumbnail = themeThumbnail;
	Connect(wxEVT_IDLE, wxIdleEventHandler(CThumbnail::OnIdle));
	Connect(wxEVT_PAINT, wxPaintEventHandler(CThumbnail::on_paint));
	Connect(wxEVT_MOTION, wxMouseEventHandler(CThumbnail::OnMouseMove));
	Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(CThumbnail::OnLButtonDown));
	Connect(wxEVT_LEFT_UP, wxMouseEventHandler(CThumbnail::OnLButtonUp));
	Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CThumbnail::OnLDoubleClick));
	Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(CThumbnail::OnMouseWheel));
	Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(CThumbnail::OnKeyDown));
	Connect(wxEVT_KEY_UP, wxKeyEventHandler(CThumbnail::OnKeyUp));
	Connect(wxEVT_ENTER_WINDOW, wxMouseEventHandler(CThumbnail::OnEnterWindow));
	Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(CThumbnail::OnLeaveWindow));

    
	timeClick = new wxTimer(this, TIMER_CLICK);
	Connect(TIMER_CLICK, wxEVT_TIMER, wxTimerEventHandler(CThumbnail::OnTimerClick), nullptr, this);

	refreshActifTimer = new wxTimer(this, TIMER_REFRESH_ACTIF);
	Connect(TIMER_REFRESH_ACTIF, wxEVT_TIMER, wxTimerEventHandler(CThumbnail::OnRefreshIconeActif), nullptr, this);

	refreshSelectTimer = new wxTimer(this, TIMER_REFRESH_SELECT);
	Connect(TIMER_REFRESH_SELECT, wxEVT_TIMER, wxTimerEventHandler(CThumbnail::OnRefreshIconeSelect), nullptr, this);

	timerAnimation = new wxTimer(this, TIMER_ANIMATION);
	Connect(TIMER_ANIMATION, wxEVT_TIMER, wxTimerEventHandler(CThumbnail::OnAnimation), nullptr, this);

	refreshMouseMove = new wxTimer(this, TIMER_MOVE);
	Connect(TIMER_MOVE, wxEVT_TIMER, wxTimerEventHandler(CThumbnail::OnTimerMove), nullptr, this);

	const wxString resourcePath = CFileUtility::GetResourcesFolderPath();
	m_animation = new wxAnimation(resourcePath + "/loading.gif");

	Connect(wxEVENT_ONSTARTLOADINGPICTURE, wxCommandEventHandler(CThumbnail::StartLoadingPicture));
	Connect(wxEVENT_ONSTOPLOADINGPICTURE, wxCommandEventHandler(CThumbnail::StopLoadingPicture));
	Connect(wxEVENT_REFRESHDATA, wxCommandEventHandler(CThumbnail::EraseThumbnail));
	Connect(wxEVENT_SCROLLMOVE, wxCommandEventHandler(CThumbnail::OnScrollMove));
	Connect(wxEVENT_LEFTPOSITION, wxCommandEventHandler(CThumbnail::OnLeftPosition));
	Connect(wxEVENT_TOPPOSITION, wxCommandEventHandler(CThumbnail::OnTopPosition));
	Connect(wxEVENT_REFRESHTHUMBNAIL, wxCommandEventHandler(CThumbnail::OnRefreshThumbnail));
    Connect(wxEVENT_THUMBNAILREFRESH, wxCommandEventHandler(CThumbnail::RefreshThumbnail));
	processIdle = false;

	listProcessWindow.push_back(this);

	TabSize = {100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700};
	Max = TabSize.size();
	positionSize = 3;

	m_waitingAnimation = new wxActivityIndicator(this, wxID_ANY);
	m_waitingAnimation->Stop();
	m_waitingAnimation->Hide();


	timeActif = 1000;
	timeSelect = 1000;

	refreshActifTimer->Start(timeActif, TRUE);
	refreshSelectTimer->Start(timeSelect, TRUE);
}


void CThumbnail::OnTimerClick(wxTimerEvent& event)
{
	moveOnPaint = true;
	needToRefresh = true;
}

int CThumbnail::GetTabValue()
{
	return positionSize;
}

void CThumbnail::OnAnimation(wxTimerEvent& event)
{
	RefreshIcone(numActifPhotoId);
	needToRefresh = true;

	if (animationStart)
		timerAnimation->Start(TIMER_TIME_REFRESH, true);
	else
		timerAnimation->Stop();
}

void CThumbnail::ChangeTabValue(const vector<int>& TabNewSize, const int& positionSize)
{
	TabSize = TabNewSize;
	Max = TabSize.size();
	this->positionSize = positionSize;
	int sizeIcone = TabSize[positionSize - 1];
	SetIconeSize(sizeIcone, sizeIcone);
	needToRefresh = true;
}

void CThumbnail::RefreshIcone(const int& idPhoto)
{
	CIcone *  icone = GetIconeById(idPhoto);
	if (icone != nullptr)
	{
		wxRect rc = icone->GetPos();
		int left = rc.x - posLargeur;
		int right = rc.x - posLargeur + themeThumbnail.themeIcone.GetWidth();
		int top = rc.y - posHauteur;
		int bottom = rc.y - posHauteur + themeThumbnail.themeIcone.GetHeight();

		if ((right > 0 && left < GetWindowWidth()) && (top < GetWindowHeight() && bottom > 0))
		{
			//wxClientDC dc(this);
			//icone->RenderIcone(&dc, -posLargeur, -posHauteur, flipHorizontal, flipVertical, true);
			needToRefresh = true;
			//return;
		}

			
	}
	
}

void CThumbnail::RefreshIconeVideo(const int& idPhoto)
{
	CIcone *  icone = GetIconeById(idPhoto);
	if (icone != nullptr)
	{
		wxRect rc = icone->GetPos();
		int left = rc.x - posLargeur;
		int right = rc.x - posLargeur + themeThumbnail.themeIcone.GetWidth();
		int top = rc.y - posHauteur;
		int bottom = rc.y - posHauteur + themeThumbnail.themeIcone.GetHeight();

		if ((right > 0 && left < GetWindowWidth()) && (top < GetWindowHeight() && bottom > 0))
		{
			wxClientDC dc(this);
			icone->SetNumFrame(-1);
			icone->RenderIcone(&dc, -posLargeur, -posHauteur, flipHorizontal, flipVertical, true);
			//needToRefresh = true;
		}

	}

}

void CThumbnail::OnRefreshThumbnail(wxCommandEvent& event)
{
	int idPhoto = event.GetId();
	RefreshIcone(idPhoto);
	needToRefresh = true;
}

void CThumbnail::OnTimerMove(wxTimerEvent& event)
{
	isMoving = false;
}

void CThumbnail::MoveTop()
{
	wxWindow* parent = this->GetParent();
	moveOnPaint = false;
	if (parent != nullptr)
	{
		wxCommandEvent evt(wxEVENT_MOVETOP);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CThumbnail::MoveLeft()
{
	wxWindow* parent = this->GetParent();
	moveOnPaint = false;
	if (parent != nullptr)
	{
		wxCommandEvent evt(wxEVENT_MOVELEFT);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CThumbnail::MoveBottom()
{
	wxWindow* parent = this->GetParent();
	moveOnPaint = false;
	if (parent != nullptr)
	{
		wxCommandEvent evt(wxEVENT_MOVEBOTTOM);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CThumbnail::MoveRight()
{
	wxWindow* parent = this->GetParent();
	moveOnPaint = false;
	if (parent != nullptr)
	{
		wxCommandEvent evt(wxEVENT_MOVERIGHT);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CThumbnail::OnLeftPosition(wxCommandEvent& event)
{
	int pos = event.GetInt();
	posLargeur = pos;
	moveOnPaint = false;
	needToRefresh = true;
}

void CThumbnail::OnTopPosition(wxCommandEvent& event)
{
	int pos = event.GetInt();
	posHauteur = pos;
	moveOnPaint = false;
	needToRefresh = true;
}

void CThumbnail::OnScrollMove(wxCommandEvent& event)
{
	refreshMouseMove->Stop();
	isMovingScroll = isMoving = event.GetInt();
	moveOnPaint = false;
	refreshMouseMove->Start(1000, true);
}

void CThumbnail::OnRefreshIconeActif(wxTimerEvent& event)
{
	//needToRefresh = true;
	//RefreshIcone(numActifPhotoId);
	//RefreshIcone(numSelectPhotoId);

	CLibPicture libPicture;
	wxClientDC dc(this);
	{
		CIcone *  icone = GetIconeById(numActifPhotoId);
		if (icone != nullptr)
		{
			CThumbnailData* data = icone->GetData();

			if (libPicture.TestIsVideo(data->GetFilename()) || libPicture.TestIsPDF(data->GetFilename()) ||
				libPicture.TestIsAnimation(data->GetFilename()))
			{
				wxRect rc = icone->GetPos();
				int left = rc.x - posLargeur;
				int right = rc.x - posLargeur + themeThumbnail.themeIcone.GetWidth();
				int top = rc.y - posHauteur;
				int bottom = rc.y - posHauteur + themeThumbnail.themeIcone.GetHeight();

				if ((right > 0 && left < GetWindowWidth()) && (top < GetWindowHeight() && bottom > 0))
					icone->RenderIcone(&dc, -posLargeur, -posHauteur, flipHorizontal, flipVertical, true);
			}
		}
	}
}

void CThumbnail::OnRefreshIconeSelect(wxTimerEvent& event)
{
	CLibPicture libPicture;
	wxClientDC dc(this);
	{
		CIcone *  icone = GetIconeById(numSelectPhotoId);
		if (icone != nullptr)
		{
			CThumbnailData* data = icone->GetData();
			if (libPicture.TestIsVideo(data->GetFilename()) || libPicture.TestIsPDF(data->GetFilename()) ||
				libPicture.TestIsAnimation(data->GetFilename()))
			{
				wxRect rc = icone->GetPos();
				int left = rc.x - posLargeur;
				int right = rc.x - posLargeur + themeThumbnail.themeIcone.GetWidth();
				int top = rc.y - posHauteur;
				int bottom = rc.y - posHauteur + themeThumbnail.themeIcone.GetHeight();

				if ((right > 0 && left < GetWindowWidth()) && (top < GetWindowHeight() && bottom > 0))
					icone->RenderIcone(&dc, -posLargeur, -posHauteur, flipHorizontal, flipVertical, true);
			}
		}
	}
}

CThumbnail::~CThumbnail()
{
	threadDataProcess = false;

	refreshMouseMove->Stop();

	timeClick->Stop();

	timerAnimation->Stop();

	refreshActifTimer->Stop();

	while (refreshActifTimer->IsRunning())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	delete refreshActifTimer;


	refreshSelectTimer->Stop();

	while (refreshSelectTimer->IsRunning())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	delete refreshSelectTimer;

	delete refreshMouseMove;


	delete timeClick;

	if (m_animation != nullptr)
		delete m_animation;

	if (m_waitingAnimation != nullptr)
		delete m_waitingAnimation;
}

int CThumbnail::GetWidth()
{
	return thumbnailSizeX;
}

int CThumbnail::GetHeight()
{
	return thumbnailSizeY;
}

int CThumbnail::GetIconeHeight()
{
	return themeThumbnail.themeIcone.GetHeight();
}

int CThumbnail::GetIconeWidth()
{
	return themeThumbnail.themeIcone.GetWidth();
}

void CThumbnail::AfterSetList()
{
	for (int i = 0; i < nbElementInIconeList; i++)
	{
		CIcone *  icone = iconeList->GetElement(i);
		if (icone != nullptr)
		{
			CThumbnailData* data = icone->GetData();
			if (numSelectPhotoId == data->GetNumPhotoId())
			{
				icone->SetSelected(true);
			}
			if (numActifPhotoId == data->GetNumPhotoId())
			{
				icone->SetActive(true);
			}

			if (numSelectPhotoId != -1 && numActifPhotoId != -1)
				break;
		}
	}
    
    preprocessisAvailable = true;
}

void CThumbnail::EraseThumbnailList(CIconeList* iconeListLocal)
{
    
	if (iconeListLocal->GetNbElement() == 0)
	{
		iconeListLocal->EraseThumbnailList();
		delete iconeListLocal;
		iconeListLocal = nullptr;
	}
	else
	{
		CListToClean* listToAdd = new CListToClean();
		time(&listToAdd->timeToAdd);
		listToAdd->list = iconeListLocal;
		listToErrase.push_back(listToAdd);

		stopToGetNbElement = false;
	}


}



void CThumbnail::EraseIconeList(std::vector<CIcone*> pIconeListToClean)
{

	if (pIconeListToClean.size() == 0)
	{
		pIconeListToClean.clear();
	}
	else
	{
		CListToClean* listToAdd = new CListToClean();
		time(&listToAdd->timeToAdd);
		listToAdd->type = 1;
		listToAdd->pIconeListToClean = pIconeListToClean;
		listToErrase.push_back(listToAdd);
	}


}

void CThumbnail::SetIconeSize(const int& width, const int& height)
{
	themeThumbnail.themeIcone.SetWidth(width);
	themeThumbnail.themeIcone.SetHeight(height);

	ResizeThumbnail();
}

void CThumbnail::ExecuteTimer(const int& numId, wxTimer* refresh)
{
	CLibPicture libPicture;
	bool actifActif = false;
	CIcone *  icone = GetIconeById(numId);
	if (icone != nullptr)
	{
		CThumbnailData* data = icone->GetData();

		if (libPicture.TestIsVideo(data->GetFilename()) || libPicture.TestIsPDF(data->GetFilename()) ||
			libPicture.TestIsAnimation(data->GetFilename()))
		{
			actifActif = true;
		}
		if (libPicture.TestIsVideo(data->GetFilename()))
		{
			timeActif = 1000 / 25;
		}
		else if (libPicture.TestIsAnimation(data->GetFilename()))
		{
			timeActif = 100;
		}
		else
		{
			timeActif = 1000;
		}
	}

	if (actifActif)
		if (!refresh->IsRunning())
			refresh->Start(timeActif, TRUE);
}

void CThumbnail::OnIdle(wxIdleEvent& evt)
{
	if (endProgram)
		return;

	if (needToRefresh)
	{
		this->Refresh();
		needToRefresh = false;
        preprocessisAvailable = true;
	}

	if (processIdle)
		StartThread();

	if (enableTimer && !isMouseOnWindow)
	{
		if (refreshActifTimer->IsRunning())
			refreshActifTimer->Stop();

		if (refreshSelectTimer->IsRunning())
			refreshSelectTimer->Stop();
	}

	if (enableTimer && isMouseOnWindow)
	{
		ExecuteTimer(numActifPhotoId, refreshActifTimer);
		ExecuteTimer(numSelectPhotoId, refreshSelectTimer);
	}
    
 
    if (!listToErrase.empty())
	{
		std::vector<CListToClean*> listToErraseNew;
		int i = 0;
		time_t ending;
		time(&ending);
        //printf("%s CThumbnail::listToErrase Nb Element : %i  \n", ctime(&ending), listToErrase.size());
		for (int i = 0; i < listToErrase.size(); i++)
		{
			CListToClean* element = listToErrase[i];
			int diff = difftime(ending, element->timeToAdd);
			if (diff > 5)
			{
				if (element->type == 0)
				{
					printf("CThumbnail::listToErrase %i \n", i);
					delete element->list;
					element->list = nullptr;
				}
				else if (element->type == 1)
				{
					printf("CThumbnail::pIconeListToClean %i \n", i);
					for (CIcone* ico : element->pIconeListToClean)
					{
						delete ico;
						ico = nullptr;
					}
					element->pIconeListToClean.clear();
				}
				delete element;
				element = nullptr;
				listToErrase[i] = nullptr;
			}
			else
				listToErraseNew.push_back(element);

		}
		listToErrase = listToErraseNew;
		
	}
}

bool CThumbnail::GetProcessEnd()
{
	return true;
}

void CThumbnail::OnMouseMove(wxMouseEvent& event)
{
	if (threadDataProcess == false)
		return;

	refreshMouseMove->Stop();

	wxClientDC dc(this);

	isMoving = true;
	bool needtoRedraw = false;
	isMovingScroll = true;
	bool isChecked = false;
	if (numActifPhotoId != -1)
	{
		CIcone *  numActif = GetIconeById(numActifPhotoId);
		if (numActif != nullptr)
			isChecked = numActif->IsChecked();
	}

	if (mouseClickBlock && enableDragAndDrop && isChecked)
	{
		int xPos = event.GetX();
		int yPos = event.GetY();
		if (numActifPhotoId != -1)
		{
			if (!mouseClickMove)
				nbElementChecked = GetNbIconSelected();
			mouseClickMove = true;
			xPosDrag = xPos;
			yPosDrag = yPos;
		}

		if (yPos < 100)
			MoveTop();
		else if (yPos > this->GetWindowHeight() - 100)
			MoveBottom();
		else
			needToRefresh = true;
	}
	else
	{
		int xPos = event.GetX();
		int yPos = event.GetY();
		int iconePhotoId = -1;
		wxSetCursor(wxCursor(wxCURSOR_HAND));

		CIcone *  pBitmapIcone = FindElement(xPos, yPos);
		

		if (pBitmapIcone != nullptr)
		{

			if (pBitmapIcone->GetData() != nullptr)
				iconePhotoId = pBitmapIcone->GetData()->GetNumPhotoId();

			if (numActifPhotoId != -1)
			{
				if (iconePhotoId != numActifPhotoId)
				{
					if (numActifPhotoId != -1)
					{
						CIcone *  numActif = GetIconeById(numActifPhotoId);
						
						if (numActif != nullptr)
						{
							numActif->SetActive(false);
						}
					}
					needtoRedraw = true;
				}
			}

			if (pBitmapIcone->GetState() != ACTIFICONE)
			{
				numActifPhotoId = iconePhotoId;
				
				//pBitmapIcone = FindElement(xPos, yPos);
				
				if (pBitmapIcone != nullptr)
				{
					pBitmapIcone->SetActive(true);
					//pBitmapIcone->RenderIcone(&dc, -posLargeur, -posHauteur, flipHorizontal, flipVertical, true);
				}
				//needtoRedraw = true;
			}
		}

		
		if (needtoRedraw)
		{
			needToRefresh = true;
		}
		
		refreshMouseMove->Start(1000, true);
	}


}

void CThumbnail::RefreshThumbnail(wxCommandEvent& event)
{
    preprocessisAvailable = true;
    needToRefresh = true;
    printf("CThumbnail::RefreshThumbnail \n");
}

bool CThumbnail::UpdateThumbnail(CIcone *  pBitmapIcone)
{
    bool isProcess = false;
    if (CThumbnailData* pThumbnailData = pBitmapIcone->GetData(); pThumbnailData != nullptr)
    {
        isProcess = pThumbnailData->IsProcess();
        //const bool isLoad = pThumbnailData->IsLoad();
        //const bool isLoad = pThumbnailData->IsLoad();
        if (!isProcess) // && !isLoad)
        {
            wxWindow* window = this->FindWindowById(MAINVIEWERWINDOWID);
            if (window != nullptr)
            {
                
                printf("CThumbnail::RenderBitmap preprocessisAvailable : %d preprocess_thumbnail : %d \n", preprocessisAvailable, preprocess_thumbnail);
                wxString* localName = new wxString(pThumbnailData->GetFilename());
                wxCommandEvent evt(wxEVENT_ICONETHUMBNAILGENERATION);
                evt.SetClientData(localName);
                evt.SetInt(30);
                evt.SetExtraLong(localid);
                window->GetEventHandler()->AddPendingEvent(evt);
            }
            pThumbnailData->SetIsProcess(true);
        }
    }  
    return isProcess;  
}
void CThumbnail::RenderBitmap(wxDC* deviceContext, CIcone *  pBitmapIcone, const int& posLargeur, const int& posHauteur)
{
   // printf("CThumbnail::RenderBitmap PreprocessThumbnail localid : %d \n", localid);
    
	if (pBitmapIcone == nullptr || !pBitmapIcone->GetVisibility())
		return;

	int nbProcesseur = 1;
	if (CRegardsConfigParam* config = CParamInit::getInstance(); config != nullptr)
		nbProcesseur = config->GetThumbnailProcess();

	const int value = pBitmapIcone->RenderIcone(deviceContext, posLargeur, posHauteur, flipHorizontal, flipVertical);
	   
	if (preprocess_thumbnail)
	{
		if (value == 1)
		{
			if (pBitmapIcone != nullptr)
			{
				if (CThumbnailData* pThumbnailData = pBitmapIcone->GetData(); pThumbnailData != nullptr)
				{
					const bool isProcess = pThumbnailData->IsProcess();
					//const bool isLoad = pThumbnailData->IsLoad();
					if (!isProcess) // && !isLoad)
					{
						wxWindow* window = this->FindWindowById(MAINVIEWERWINDOWID);
						if (window != nullptr)
						{
							wxString* localName = new wxString(pThumbnailData->GetFilename());
							wxCommandEvent evt(wxEVENT_ICONETHUMBNAILGENERATION);
							evt.SetClientData(localName);
							evt.SetInt(0);
							evt.SetExtraLong(localid);
							window->GetEventHandler()->AddPendingEvent(evt);
						}
						pThumbnailData->SetIsProcess(true);
					}
				}
			}
		}
	}
	


}

void CThumbnail::UpdateScreenRatio()
{
	this->Resize();
}

void CThumbnail::OpenFileViewer(const wxString& filename)
{
	//ShellExecute(m_hWnd, L"open", filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void CThumbnail::OnLDoubleClick(wxMouseEvent& event)
{
	int xPos = event.GetX();
	int yPos = event.GetY();

	if (CIcone *  pBitmapIcone = FindElement(xPos, yPos); pBitmapIcone != nullptr)
	{
		auto pThumbnailData = pBitmapIcone->GetData();
		if (pThumbnailData != nullptr)
		{
			switch (pThumbnailData->GetTypeElement())
			{
			case TYPEPHOTO:
				{
					OpenFileViewer(pThumbnailData->GetFilename());
				}
				break;
			default: ;
			}
		}
	}
}

void CThumbnail::OnLButtonUp(wxMouseEvent& event)
{
	int xPos = event.GetX();
	int yPos = event.GetY();
	timeClick->Stop();
	mouseClickBlock = false;
	if (mouseClickMove && enableDragAndDrop)
	{
		OnMouseRelease(xPos, yPos);
		mouseClickMove = false;
		needToRefresh = true;
	}
}


void CThumbnail::OnLButtonDown(wxMouseEvent& event)
{
	wxClientDC winDC(this);
	this->SetFocus();
	int xPos = event.GetX();
	int yPos = event.GetY();

	mouseClickBlock = true;
	bool isIconeSelected = false;
	int iconePhotoId = -1;
	CIcone *  pBitmapIcone = FindElement(xPos, yPos);
	if (pBitmapIcone != nullptr)
		if (pBitmapIcone->GetData() != nullptr)
		{
			iconePhotoId = pBitmapIcone->GetData()->GetNumPhotoId();
			isIconeSelected = pBitmapIcone->IsChecked();
		}


	if (numSelectPhotoId != -1)
	{
		CIcone *  numSelect = GetIconeById(numSelectPhotoId);
		if (numSelect != nullptr)
			numSelect->SetSelected(false);
	}


	if (pBitmapIcone != nullptr)
	{
		numSelectPhotoId = iconePhotoId;
		int value = pBitmapIcone->OnClick(xPos, yPos, posLargeur, posHauteur);
		//
		if (value == 1)
		{
			CThumbnailData* data = pBitmapIcone->GetCopyData();
			OnPictureClick(data);
			delete data;
		}
		else if (value == 2)
		{
			DeleteIcone(pBitmapIcone);
		}
		else
		{
			CThumbnailData* data = pBitmapIcone->GetCopyData();
			OnPictureClick(data);
			delete data;
			pBitmapIcone->SetSelected(true);
		}
	}
	else
	{
		FindOtherElement(&winDC, xPos, yPos);
	}

	if (numActifPhotoId != -1 && enableDragAndDrop && isIconeSelected)
	{
		if (timeClick->IsRunning())
			timeClick->Stop();

		timeClick->Start(1000);

		if (numActifPhotoId != -1)
		{
			CIcone *  numActif = GetIconeById(numActifPhotoId);
			bitmapIconDrag = numActif->GetCopyIcone();
		}
		wxImage image = bitmapIconDrag.ConvertToImage();
		auto alphaData = new unsigned char[image.GetWidth() * image.GetHeight()];
		memset(alphaData, 128, image.GetWidth() * image.GetHeight());
		image.SetAlpha(alphaData);
		bitmapIconDrag = image;
	}
	needToRefresh = true;
}

int CThumbnail::GetNbIconSelected()
{
	int nbCheck = 0;
	int nbElement = nbElementInIconeList;
	for (int i = 0; i < nbElement; i++)
	{
		CIcone *  icone = iconeList->GetElement(i);
		if (icone->IsChecked())
			nbCheck++;
	}
	return nbCheck;
}


void CThumbnail::StartLoadingPicture(wxCommandEvent& event)
{
	int numItem = event.GetExtraLong();

	if (numloadingIconePhotoId != -1)
	{
		CIcone *  loadingIcone = GetIconeById(numloadingIconePhotoId);
		if (loadingIcone != nullptr)
			loadingIcone->StopLoadingPicture();
	}

	if (numItem >= nbElementInIconeList)
		return;


	numloadingIconePhotoId = iconeList->GetPhotoId(numItem);

	stepLoading = 0;
	showLoadingBitmap = true;

	if (numloadingIconePhotoId != -1)
	{
		CIcone *  loadingIcone = GetIconeById(numloadingIconePhotoId);
		if (loadingIcone != nullptr)
			loadingIcone->StartLoadingPicture();
	}
}

void CThumbnail::StopLoadingPicture(wxCommandEvent& event)
{
	if (numloadingIconePhotoId != -1)
	{
		CIcone *  loadingIcone = GetIconeById(numloadingIconePhotoId);
		if (loadingIcone != nullptr)
		{
			loadingIcone->StopLoadingPicture();
		}
	}

	showLoadingBitmap = false;
	needToRefresh = true;
}


void CThumbnail::on_paint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	Render(dc);
}


void CThumbnail::PaintNow()
{
	wxClientDC dc(this);
	Render(dc);
}

void CThumbnail::Render(wxDC& dc)
{
	int width = GetWindowWidth();
	int height = GetWindowHeight();

	if (width <= 0 || height <= 0)
		return;

	if (!threadDataProcess)
	{
		RenderBackground(dc);
		StartWaitingAnimation(width, height);
		return;
	}

	StopWaitingAnimation();

	if (numSelectPhotoId != -1 && !isMovingScroll && moveOnPaint)
		CenterSelectedIcon();

	TestMaxX();
	TestMaxY();

	render = true;

	RenderBackground(dc);
	RenderIcons(dc);

	render = false;
	oldPosLargeur = posLargeur;
	oldPosHauteur = posHauteur;

	NotifyParentOnPositionChange();

	if (mouseClickBlock && mouseClickMove && enableDragAndDrop)
		RenderDragAndDrop(dc);
}

void CThumbnail::RenderBackground(wxDC& dc)
{
	wxRect rc = GetWindowRect();
	FillRect(&dc, rc, themeThumbnail.colorBack);
}

void CThumbnail::StartWaitingAnimation(int width, int height)
{
	if (!animationStart)
	{
		m_waitingAnimation->Show();
		m_waitingAnimation->Start();
		animationStart = true;
		timerAnimation->Start(100, TIMER_TIME_REFRESH);
	}

	m_waitingAnimation->SetSize(wxSize(width, height));
	m_waitingAnimation->SetBackgroundColour(themeThumbnail.colorBack);
}

void CThumbnail::StopWaitingAnimation()
{
	if (animationStart)
	{
		timerAnimation->Stop();
		m_waitingAnimation->Stop();
		m_waitingAnimation->Hide();
		animationStart = false;
	}
}

void CThumbnail::CenterSelectedIcon()
{
	CIcone* numSelect = GetIconeById(numSelectPhotoId);
	if (numSelect != nullptr)
	{
		wxRect rect = numSelect->GetPos();
		int yPos = std::max((rect.y - this->GetWindowHeight() / 2), 0);
		int xPos = std::max((rect.x - this->GetWindowWidth() / 2), 0);
		posLargeur = xPos;
		posHauteur = yPos;
	}
}


void CThumbnail::RenderIcons(wxDC& dc)
{
	RenderIcone(&dc);
}

void CThumbnail::NotifyParentOnPositionChange()
{
	if (this->GetParent() != nullptr && moveOnPaint)
	{
		auto size = new wxSize();
		wxCommandEvent evt(wxEVENT_SETPOSITION);
		size->x = posLargeur;
		size->y = posHauteur;
		evt.SetClientData(size);
		this->GetParent()->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CThumbnail::RenderDragAndDrop(wxDC& dc)
{
	dc.DrawBitmap(bitmapIconDrag, xPosDrag - (bitmapIconDrag.GetWidth() / 2),
		yPosDrag - (bitmapIconDrag.GetHeight() / 2));

	if (nbElementChecked > 1)
	{
		wxString libelle = std::to_string(nbElementChecked);

		if (!libelle.IsEmpty())
		{
			CThemeIcone themeIcone;
			CThemeFont themeFont = themeIcone.font;
			themeFont.SetFontSize(18);
			wxSize size = GetSizeTexte(&dc, libelle, themeFont);
			int localx = xPosDrag - (bitmapIconDrag.GetWidth() / 2);
			int localy = xPosDrag - (bitmapIconDrag.GetHeight() / 2);

			int xPos = xPosDrag - size.x / 2;
			int yPos = yPosDrag - size.y / 2;

			dc.SetBrush(wxBrush(themeIcone.colorSelectTop));
			dc.DrawRoundedRectangle(localx + bitmapIconDrag.GetWidth() / 4, localy + bitmapIconDrag.GetHeight() / 4,
				bitmapIconDrag.GetWidth() / 2, bitmapIconDrag.GetHeight() / 2, -0.25);
			dc.SetBrush(wxNullBrush);

			dc.SetBrush(wxBrush(*wxWHITE));
			DrawTexte(&dc, libelle, xPos, yPos, themeFont);
			dc.SetBrush(wxNullBrush);
		}
	}
}

void CThumbnail::Resize()
{
	CalculControlSize();
	ResizeThumbnail();
	needToRefresh = true;
}

void CThumbnail::CalculControlSize()
{
	controlWidth = GetWidth();
	controlHeight = GetHeight();

	if (wxWindow* parent = this->GetParent(); parent != nullptr)
	{
		const auto controlSize = new CControlSize();
		wxCommandEvent evt(wxEVENT_SETCONTROLSIZE);
		controlSize->controlWidth = controlWidth;
		controlSize->controlHeight = controlHeight;
		evt.SetClientData(controlSize);
		parent->GetEventHandler()->AddPendingEvent(evt);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void CThumbnail::TestMaxX()
{
	if (long xValue = GetWidth() - GetWindowWidth(); posLargeur >= xValue)
		posLargeur = xValue;

	if (posLargeur < 0)
		posLargeur = 0;
}

void CThumbnail::OnEnterWindow(wxMouseEvent& event)
{
	isMouseOnWindow = true;
}

void CThumbnail::OnLeaveWindow(wxMouseEvent& event)
{
	isMouseOnWindow = false;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void CThumbnail::TestMaxY()
{
	if (long yValue = GetHeight() - GetWindowHeight(); posHauteur >= yValue)
		posHauteur = yValue;

	if (posHauteur < 0)
		posHauteur = 0;
}

void CThumbnail::OnKeyUp(wxKeyEvent& event)
{
	switch (event.m_keyCode)
	{
	case WXK_CONTROL:
		controlKeyPush = false;
		break;
	default: ;
	}
}

void CThumbnail::OnKeyDown(wxKeyEvent& event)
{
	switch (event.m_keyCode)
	{
	case WXK_UP:
		this->MoveTop();
		break;
	case WXK_LEFT:
		this->MoveLeft();
		break;
	case WXK_DOWN:
		this->MoveBottom();
		break;
	case WXK_RIGHT:
		this->MoveRight();
		break;
	case WXK_CONTROL:
		controlKeyPush = true;
		break;
	case WXK_ESCAPE:
		if (moveOnPaint)
		{
			timeClick->Stop();
			mouseClickBlock = false;
			if (mouseClickMove && enableDragAndDrop)
			{
				mouseClickMove = false;
				needToRefresh = true;
			}
		}
		break;
	default: ;
	}
}

void CThumbnail::OnMouseWheel(wxMouseEvent& event)
{
	int move;

	//printf("OnMouseWheel value : %d \n", event.GetWheelRotation());

	if (event.GetWheelRotation() > 0)
		move = 0;
	else
		move = 1;


	if (controlKeyPush)
		move += 10;


	switch (move)
	{
	case 0:
		this->MoveTop();
		break;
	case 1:
		this->MoveBottom();
		break;
	case 10:
		{
			wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
			if (mainWindow != nullptr)
			{
				wxCommandEvent evt(wxEVENT_PICTUREPREVIOUS);
				mainWindow->GetEventHandler()->AddPendingEvent(evt);
			}
		}
		break;
	case 11:
		{
			wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
			if (mainWindow != nullptr)
			{
				wxCommandEvent evt(wxEVENT_PICTURENEXT);
				mainWindow->GetEventHandler()->AddPendingEvent(evt);
			}
		}
		break;
	default: ;
	}
}


void CThumbnail::InitScrollingPos()
{
	posHauteur = 0;
	posLargeur = 0;


	wxWindow* parent = this->GetParent();

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


void CThumbnail::UpdateRenderIcone(CThreadLoadingBitmap * threadLoadingBitmap)
{

	if (threadLoadingBitmap == nullptr)
	{
		return;
	}
	if (threadDataProcess != false)
	{
		if (threadLoadingBitmap != nullptr)
		{
			if (!threadLoadingBitmap->bitmapIcone.empty())
			{
				CThumbnailData* pThumbnailData = nullptr;
				CIcone *  icone = GetIconeByPath(threadLoadingBitmap->filename);
				if (icone != nullptr)
				{
					if (pThumbnailData == nullptr)
						pThumbnailData = icone->GetData();

					if (pThumbnailData != nullptr)
					{
						if (pThumbnailData->GetTypeElement() == TYPEVIDEO)
						{
							pThumbnailData->SetTimePosition(threadLoadingBitmap->timePosition);
						}
						pThumbnailData->SetIsProcess(true);
						//pThumbnailData->SetBitmap(threadLoadingBitmap->bitmapIcone);
						pThumbnailData->SetIsLoading(false);
						icone->RefreshIcone();
					}
				}
			}
		}
	}

	//needToRefresh = true;
}
