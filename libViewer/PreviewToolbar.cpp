#include <header.h>
#include "PreviewToolbar.h"
#include "ToolbarSlide.h"
#include <LibResource.h>
#include <ToolbarInterface.h>
#include "MainWindow.h"
#include <window_id.h>
using namespace Regards::Window;
using namespace Regards::Viewer;


#define IDM_WINDOWSEARCH 152
#define IDM_THUMBNAILFACE 153
#define IDM_VIEWERMODE 154
#define IDM_EXPLORERMODE 155
#define IDM_SHOWINFOS 156
#define IDM_QUITTER 157
#define IDM_PRINT 158
#define IDM_SCANNER 159
#define IDM_PICTUREMODE 160
#define IDM_EDIT 161
#define IDM_EXPORT 162
#define IDM_NEWVERSION 163
#define IDM_EXPORT_DIAPORAMA 164

CPreviewToolbar::CPreviewToolbar(wxWindow* parent, wxWindowID id, const CThemeToolbar& theme,
                                 CToolbarInterface* toolbarInterface, const bool& vertical)
	: CToolbarWindow(parent, id, theme, vertical)
{
	imagePlayDiaporama = nullptr;
	imageStopDiaporama = nullptr;
	fullscreen = nullptr;

	wxString libelleFullscreen = CLibResource::LoadStringFromResource(L"LBLFULLSCREEN", 1);
	wxString libelleFirst = CLibResource::LoadStringFromResource(L"LBLFIRST", 1);
	wxString libellePrevious = CLibResource::LoadStringFromResource(L"LBLPREVIOUS", 1);
	wxString libelleRotate90 = CLibResource::LoadStringFromResource(L"LBLROTATE90", 1);
	wxString libelleRotate270 = CLibResource::LoadStringFromResource(L"LBLROTATE270", 1);
	wxString libellePlay = CLibResource::LoadStringFromResource(L"LBLPLAY", 1);
	wxString libelleStop = CLibResource::LoadStringFromResource(L"LBLSTOP", 1);
	wxString libelleFlipVertical = CLibResource::LoadStringFromResource(L"LBLFLIPV", 1);
	wxString libelleFlipHorizontal = CLibResource::LoadStringFromResource(L"LBLFLIPH", 1);
	wxString libelleNext = CLibResource::LoadStringFromResource(L"LBLNEXT", 1);
	wxString libelleEnd = CLibResource::LoadStringFromResource(L"LBLEND", 1);
	wxString saveLibelle = CLibResource::LoadStringFromResource("LBLSAVE", 1); // "Save";
    wxString export_label = CLibResource::LoadStringFromResource(L"LBLEXPORT", 1);
    wxString lblEditor = CLibResource::LoadStringFromResource(L"LBLEDITORMODE", 1);
    
	fullscreen = new CToolbarButton(themeToolbar.button);
	fullscreen->SetButtonResourceId(L"IDB_SCREENPNG");
	fullscreen->SetCommandId(IDM_SETFULLSCREEN);
	fullscreen->SetLibelle(libelleFullscreen);
	navElement.push_back(fullscreen);

	save = new CToolbarButton(themeToolbar.button);
	save->SetButtonResourceId("IDB_SAVE");
	save->SetCommandId(WM_SAVE);
	save->SetLibelle(saveLibelle);
	navElement.push_back(save);
    
	auto editor = new CToolbarButton(themeToolbar.button);
	editor->SetButtonResourceId(L"IDB_OPEN");
	editor->SetLibelle(lblEditor);
	editor->SetCommandId(IDM_EDIT);
	navElement.push_back(editor);
    
    exportFile= new CToolbarButton(themeToolbar.button);
	exportFile->SetButtonResourceId("IDB_EXPORT");
	exportFile->SetCommandId(IDM_EXPORT);
	exportFile->SetLibelle(export_label);
	navElement.push_back(exportFile);

	imageFirst = new CToolbarButton(themeToolbar.button);
	imageFirst->SetButtonResourceId(L"IDB_ARROWTRACKLPNG");
	imageFirst->SetLibelle(libelleFirst);
	imageFirst->SetCommandId(WM_IMAGES_FIRST);
	navElement.push_back(imageFirst);

	imagePrec = new CToolbarButton(themeToolbar.button);
	imagePrec->SetButtonResourceId(L"IDB_ARROWLPNG");
	imagePrec->SetLibelle(libellePrevious);
	imagePrec->SetCommandId(WM_IMAGES_PRCDENTE);
	imagePrec->SetRepeatable(true);
	navElement.push_back(imagePrec);

	auto rotate90 = new CToolbarButton(themeToolbar.button);
	rotate90->SetButtonResourceId(L"IDB_ROTATION90");
	rotate90->SetCommandId(WM_ROTATE90);
	rotate90->SetLibelle(libelleRotate90);
	navElement.push_back(rotate90);

	auto rotate180 = new CToolbarButton(themeToolbar.button);
	rotate180->SetButtonResourceId(L"IDB_ROTATION270");
	rotate180->SetCommandId(WM_ROTATE270);
	rotate180->SetLibelle(libelleRotate270);
	navElement.push_back(rotate180);

	imagePlayDiaporama = new CToolbarButton(themeToolbar.button);
	imagePlayDiaporama->SetButtonResourceId(L"IDB_PLAY");
	imagePlayDiaporama->SetCommandId(WM_DIAPORAMA_PLAY);
	imagePlayDiaporama->SetLibelle(libellePlay);
	navElement.push_back(imagePlayDiaporama);

	imageStopDiaporama = new CToolbarButton(themeToolbar.button);
	imageStopDiaporama->SetButtonResourceId(L"IDB_STOP");
	imageStopDiaporama->SetCommandId(WM_DIAPORAMA_STOP);
	imageStopDiaporama->SetLibelle(libelleStop);
	imageStopDiaporama->SetVisible(false);
	navElement.push_back(imageStopDiaporama);

	auto flipVertical = new CToolbarButton(themeToolbar.button);
	flipVertical->SetButtonResourceId(L"IDB_FLIPVERT");
	flipVertical->SetCommandId(WM_FLIPVERTICAL);
	flipVertical->SetLibelle(libelleFlipVertical);
	navElement.push_back(flipVertical);

	auto flipHorizontal = new CToolbarButton(themeToolbar.button);
	flipHorizontal->SetButtonResourceId(L"IDB_FLIPHORZ");
	flipHorizontal->SetCommandId(WM_FLIPHORIZONTAL);
	flipHorizontal->SetLibelle(libelleFlipHorizontal);
	navElement.push_back(flipHorizontal);

	imageSuiv = new CToolbarButton(themeToolbar.button);
	imageSuiv->SetButtonResourceId(L"IDB_ARROWRPNG");
	imageSuiv->SetCommandId(WM_IMAGES_SUIVANTE);
	imageSuiv->SetRepeatable(true);
	imageSuiv->SetLibelle(libelleNext);
	navElement.push_back(imageSuiv);

	imageEnd = new CToolbarButton(themeToolbar.button);
	imageEnd->SetButtonResourceId(L"IDB_ARROWTRACKRPNG");
	imageEnd->SetCommandId(WM_IMAGES_END);
	imageEnd->SetLibelle(libelleEnd);
	navElement.push_back(imageEnd);
	this->toolbarInterface = toolbarInterface;
}

void CPreviewToolbar::EnableScreenButton()
{
	fullscreen->SetVisible(true);
	needToRefresh = true;
}

void CPreviewToolbar::DisableScreenButton()
{
	fullscreen->SetVisible(false);
	needToRefresh = true;
}

void CPreviewToolbar::EnableExportButton()
{
	exportFile->SetVisible(true);
	needToRefresh = true;
}


void CPreviewToolbar::DisableExportButton()
{
	exportFile->SetVisible(false);
	needToRefresh = true;
}

void CPreviewToolbar::EnableSaveButton()
{
	save->SetVisible(true);
	needToRefresh = true;
}


void CPreviewToolbar::DisableSaveButton()
{
	save->SetVisible(false);
	needToRefresh = true;
}

void CPreviewToolbar::DisableNavigationButton()
{
	if (navigationButtonEnable)
	{
		navigationButtonEnable = false;
		imagePlayDiaporama->SetVisible(false);
		imageStopDiaporama->SetVisible(false);
		fullscreen->SetVisible(false);
		imageEnd->SetVisible(false);
		imageSuiv->SetVisible(false);
		imageFirst->SetVisible(false);
		imagePrec->SetVisible(false);
		needToRefresh = true;
	}
}

void CPreviewToolbar::EnableNavigationButton()
{
	if (!navigationButtonEnable)
	{
		navigationButtonEnable = true;
		imagePlayDiaporama->SetVisible(true);
		imageStopDiaporama->SetVisible(true);
		fullscreen->SetVisible(true);
		imageEnd->SetVisible(true);
		imageSuiv->SetVisible(true);
		imageFirst->SetVisible(true);
		imagePrec->SetVisible(true);
		needToRefresh = true;
	}
}


CPreviewToolbar::~CPreviewToolbar()
{
}

void CPreviewToolbar::SetFullscreen()
{
	wxString libelleFullscreen = CLibResource::LoadStringFromResource(L"LBLSCREEN", 1);
	fullscreen->SetLibelle(libelleFullscreen);
	needToRefresh = true;
}

void CPreviewToolbar::SetScreen()
{
	wxString libelleFullscreen = CLibResource::LoadStringFromResource(L"LBLFULLSCREEN", 1);
	fullscreen->SetLibelle(libelleFullscreen);
	needToRefresh = true;
}

void CPreviewToolbar::Rotate90()
{
	if (toolbarInterface != nullptr)
		toolbarInterface->ClickShowButton(WM_ROTATE90);
}

void CPreviewToolbar::Rotate270()
{
	if (toolbarInterface != nullptr)
		toolbarInterface->ClickShowButton(WM_ROTATE270);
}

void CPreviewToolbar::FlipVertical()
{
	if (toolbarInterface != nullptr)
		toolbarInterface->ClickShowButton(WM_FLIPVERTICAL);
}

void CPreviewToolbar::FlipHorizontal()
{
	if (toolbarInterface != nullptr)
		toolbarInterface->ClickShowButton(WM_FLIPHORIZONTAL);
}

void CPreviewToolbar::Save()
{
	if (toolbarInterface != nullptr)
		toolbarInterface->ClickShowButton(WM_SAVE);
}

void CPreviewToolbar::Fullscreen()
{
	auto mainWindow = static_cast<CMainWindow*>(this->FindWindowById(MAINVIEWERWINDOWID));
	if (mainWindow != nullptr)
	{
		if (mainWindow->IsFullscreen())
			mainWindow->SetScreen();
		else
			mainWindow->SetFullscreen();
	}
}

void CPreviewToolbar::NextPicture()
{
	wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (mainWindow != nullptr)
	{
		wxCommandEvent evt(wxEVENT_PICTURENEXT);
		mainWindow->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CPreviewToolbar::PreviousPicture()
{
	wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (mainWindow != nullptr)
	{
		wxCommandEvent evt(wxEVENT_PICTUREPREVIOUS);
		mainWindow->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CPreviewToolbar::LastPicture()
{
	wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (mainWindow != nullptr)
	{
		wxCommandEvent evt(wxEVENT_PICTURELAST);
		mainWindow->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CPreviewToolbar::FirstPicture()
{
	wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (mainWindow != nullptr)
	{
		wxCommandEvent evt(wxEVENT_PICTUREFIRST);
		mainWindow->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CPreviewToolbar::DiaporamaStart()
{
	auto mainWindow = static_cast<CMainWindow*>(this->FindWindowById(MAINVIEWERWINDOWID));
	if (mainWindow != nullptr)
	{
		imagePlayDiaporama->SetVisible(false);
		imageStopDiaporama->SetVisible(true);

		wxWindow* window = this->FindWindowById(CENTRALVIEWERWINDOWID);
		if (window != nullptr)
		{
			wxCommandEvent evt(wxEVENT_STARTDIAPORAMA);
			window->GetEventHandler()->AddPendingEvent(evt);
		}

		this->Resize();
	}
}

void CPreviewToolbar::DiaporamaStop()
{
	auto mainWindow = static_cast<CMainWindow*>(this->FindWindowById(MAINVIEWERWINDOWID));
	if (mainWindow != nullptr)
	{
		imageStopDiaporama->SetVisible(false);
		imagePlayDiaporama->SetVisible(true);

		wxWindow* window = this->FindWindowById(CENTRALVIEWERWINDOWID);
		if (window != nullptr)
		{
			wxCommandEvent evt(wxEVENT_STOPDIAPORAMA);
			window->GetEventHandler()->AddPendingEvent(evt);
		}

		this->Resize();
	}
}


void CPreviewToolbar::EventManager(const int& id)
{
	switch (id)
	{
	case WM_SAVE:
		Save();
		break;

	case WM_DIAPORAMA_PLAY:
		DiaporamaStart();
		break;

	case WM_DIAPORAMA_STOP:
		DiaporamaStop();
		break;

	case IDM_SETFULLSCREEN:
		Fullscreen();
		break;
	case WM_ROTATE90:
		Rotate90();
		break;
	case WM_ROTATE270:
		Rotate270();
		break;
	case WM_FLIPVERTICAL:
		FlipVertical();
		break;
	case WM_FLIPHORIZONTAL:
		FlipHorizontal();
		break;

	case WM_IMAGES_FIRST:
		FirstPicture();
		break;

	case WM_IMAGES_PRCDENTE:
		PreviousPicture();
		break;

	case WM_IMAGES_SUIVANTE:
		NextPicture();
		break;

	case WM_IMAGES_END:
		LastPicture();
		break;
	case IDM_EDIT:
		{
			wxWindow* central = this->FindWindowById(MAINVIEWERWINDOWID);
			auto event = new wxCommandEvent(wxEVENT_EDITFILE);
			wxQueueEvent(central, event);
			break;
		}
	case IDM_EXPORT:
		{
			wxWindow* central = this->FindWindowById(MAINVIEWERWINDOWID);
			auto event = new wxCommandEvent(wxEVENT_EXPORTFILE);
			wxQueueEvent(central, event);
			break;
		}
    
	default: ;
	}
}
