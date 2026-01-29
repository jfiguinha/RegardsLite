#include <header.h>
#include "ViewerFrame.h"
#include "MainWindow.h"
#include "PertinenceValue.h"
#include <BitmapPrintout.h>
#include "ViewerParamInit.h"
#include "SQLRemoveData.h"
#include <PrintEngine.h>
#include <LibResource.h>
//#include <OpenCLDialog.h>
#include <wx/filename.h>
#include <ConfigRegards.h>
#include "MainThemeInit.h"
#include "ViewerParam.h"
#include <wx/display.h>
#include "MainTheme.h"
#include <RegardsConfigParam.h>
#include <ParamInit.h>
#include <FileUtility.h>
#include <SqlFindPhotos.h>
#include <SqlFindFolderCatalog.h>
#include <libPicture.h>
#include <SavePicture.h>
#include <ImageLoadingFormat.h>
#include "WaitingWindow.h"
#include <wx/stdpaths.h>
#include <SqlThumbnail.h>
#include <SqlFacePhoto.h>
#include "window_mode_id.h"
#include <wx/busyinfo.h>
#include <LibResource.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <FileUtility.h>
#include <wx/dirdlg.h>
#include <wx/progdlg.h>
#include "DownloadFile.h"
#ifdef __APPLE__
#include <ToggleFullscreen.h>
#endif
using namespace std;
using namespace Regards::Print;
using namespace Regards::Control;
using namespace Regards::Viewer;
using namespace Regards::Sqlite;
using namespace Regards::Picture;
using namespace Regards::Internet;

constexpr auto TIMER_LOADPICTURE = 2;
constexpr auto TIMER_LOADPICTUREEND = 4;

#if !wxUSE_PRINTING_ARCHITECTURE
#error "You must set wxUSE_PRINTING_ARCHITECTURE to 1 in setup.h, and recompile the library."
#endif


#include <ctype.h>
#include "wx/metafile.h"
#include "wx/print.h"
#include "wx/printdlg.h"

#if wxUSE_POSTSCRIPT
#include "wx/generic/printps.h"
#include "wx/generic/prntdlgg.h"
#endif

#if wxUSE_GRAPHICS_CONTEXT
#include "wx/graphics.h"
#endif

#ifdef __WXMAC__
#include "wx/osx/printdlg.h"
#endif


bool CViewerFrame::viewerMode = true;

using namespace Regards::Viewer;

bool CViewerFrame::GetViewerMode()
{
	return viewerMode;
}


void CViewerFrame::SetViewerMode(const bool& mode)
{
	viewerMode = mode;
}


CViewerFrame::CViewerFrame(const wxString& title, const wxPoint& pos, const wxSize& size, IMainInterface* mainInterface,
                           const wxString& openfile)
	: wxFrame(nullptr, FRAMEVIEWER_ID, title, pos, size, wxMAXIMIZE | wxFRAME_EX_METAL  | wxDEFAULT_FRAME_STYLE), title_(title), pos_(pos),
	  size_(size), main_interface_(mainInterface), file_to_open_(openfile)
{
	mainWindow = nullptr;
	fullscreen = false;
	onExit = false;
	fileToOpen = openfile;
	mainWindowWaiting = nullptr;

	wxString resourcePath = CFileUtility::GetResourcesFolderPathWithExt("regardsliteicon.xpm");
	wxIcon icon(resourcePath, wxBITMAP_TYPE_XPM);
	SetIcon(icon);


	viewerParam = new CMainParam();
	CMainParamInit::Initialize(viewerParam);
	Maximize();
	viewerTheme = new CMainTheme();
	CMainThemeInit::Initialize(viewerTheme);

	this->mainInterface = mainInterface;
	this->mainInterface->parent = this;


	exitTimer = new wxTimer(this, wxTIMER_EXIT);
	Connect(wxTIMER_EXIT, wxEVT_TIMER, wxTimerEventHandler(CViewerFrame::CheckAllProcessEnd), nullptr, this);


	bool openFirstFile = true;
	
	
	//Verify if file exist
	if(!wxFileExists(fileToOpen))
		fileToOpen = "";
	
	//SetIcon(wxIcon(wxT("regards.xpm")));
	if (fileToOpen != "")
		openFirstFile = false;





	mainWindow = new CMainWindow(this, MAINVIEWERWINDOWID, this, openFirstFile, fileToOpen);

	//mainWindow->Show(true);
	//mainWindowWaiting->Show(false);
	//preview = nullptr;
	m_previewModality = wxPreviewFrame_AppModal;
	loadPictureTimer = new wxTimer(this, TIMER_LOADPICTURE);

	endLoadPictureTimer = new wxTimer(this, TIMER_LOADPICTUREEND);

	auto menuFile = new wxMenu;

	wxString labelDecreaseIconSize = CLibResource::LoadStringFromResource(L"labelDecreaseIconSize", 1);
	//L"Decrease Icon Size";
	wxString labelDecreaseIconSize_link = CLibResource::LoadStringFromResource(L"labelDecreaseIconSize_link", 1);
	//L"&Decrease Icon Size";
	wxString labelEnlargeIconSize = CLibResource::LoadStringFromResource(L"labelEnlargeIconSize", 1);
	//L"Enlarge Icon Size";
	wxString labelEnlargeIconSize_link = CLibResource::LoadStringFromResource(L"labelEnlargeIconSize_link", 1);
	//L"&Enlarge Icon Size";
	wxString labelConfiguration = CLibResource::LoadStringFromResource(L"labelConfiguration", 1); //L"Configuration";
	wxString labelConfiguration_link = CLibResource::LoadStringFromResource(L"labelConfiguration_link", 1);
	//L"&Configuration";
	wxString labelEraseDataBase = CLibResource::LoadStringFromResource(L"labelEraseDataBase", 1); //L"Erase Database";
	wxString labelEraseDataBase_link = CLibResource::LoadStringFromResource(L"labelEraseDataBase_link", 1);
	//L"&Erase Database";
	wxString labelThumbnailRight = CLibResource::LoadStringFromResource(L"labelThumbnailRight", 1); //L"Right Position";
	wxString labelThumbnailRight_link = CLibResource::LoadStringFromResource(L"labelThumbnailRight_link", 1);
	//L"&Right Position";
	wxString labelThumbnailBottom = CLibResource::LoadStringFromResource(L"labelThumbnailBottom", 1);
	//L"Bottom Position";
	wxString labelThumbnailBottom_link = CLibResource::LoadStringFromResource(L"labelThumbnailBottom_link", 1);
	//L"&Bottom Position";
	wxString labelPageSetup = CLibResource::LoadStringFromResource(L"labelPageSetup", 1); //L"Page setup";
	wxString labelPageSetup_link = CLibResource::LoadStringFromResource(L"labelPageSetup_link", 1); //L"Page Set&up...";
	wxString labelPageMargins = CLibResource::LoadStringFromResource(L"labelPageMargins", 1); // L"Page margins";
	wxString labelPageMargins_link = CLibResource::LoadStringFromResource(L"labelPageMargins_link", 1);
	//L"Page margins...";
	wxString labelFile = CLibResource::LoadStringFromResource(L"labelFile", 1); //L"&File";
	wxString labelParameter = CLibResource::LoadStringFromResource(L"labelParameter", 1); //L"&Parameter";
	wxString labelSizeIcon = CLibResource::LoadStringFromResource(L"labelSizeIcon", 1); //L"&Icon Size";
	wxString labelWindow = CLibResource::LoadStringFromResource(L"labelWindow", 1); //L"&Icon Size";
	wxString labelThumbnail = CLibResource::LoadStringFromResource(L"labelThumbnail", 1); //L"&Thumbnail";
	wxString labelHelp = CLibResource::LoadStringFromResource(L"labelHelp", 1); //L"&Help";


    
    
	wxString export_diaporama = CLibResource::LoadStringFromResource(L"LBLEXPORTDIAPORAMA", 1);
	wxString lblEditor = CLibResource::LoadStringFromResource(L"LBLEDITORMODE", 1);
    wxString lblScanner = CLibResource::LoadStringFromResource(L"LBLSCANNER", 1);
    
	auto menuView = new wxMenu;
    auto menuTools = new wxMenu;
	menuTools->Append(ID_DIAPORAMA, export_diaporama, export_diaporama);


	auto menuSizeIcon = new wxMenu;
	menuSizeIcon->Append(ID_SIZEICONLESS, labelDecreaseIconSize_link, labelDecreaseIconSize);
	menuSizeIcon->Append(ID_SIZEICONMORE, labelEnlargeIconSize_link, labelEnlargeIconSize);

	menuFile->Append(ID_OPENFILE, "Open &File", "Open File");
	menuFile->Append(ID_OPENFOLDER, "&Open Folder", "Open Folder");

#ifdef WIN32
	menuFile->Append(ID_ASSOCIATE, "&Associate", "Associate");
    menuFile->AppendSeparator();
#endif
    //menuTools->Append(wxID_EDIT, lblEditor, lblEditor);
   
	
	menuFile->Append(WXPRINT_PAGE_SETUP, labelPageSetup_link, labelPageSetup);
#ifdef __WXMAC__
	menuFile->Append(WXPRINT_PAGE_MARGINS, labelPageMargins_link, labelPageMargins);
#endif
	menuFile->Append(wxID_PRINT, wxT("&Print..."), wxT("Print"));
	menuFile->AppendSeparator();
	menuFile->Append(ID_Configuration, labelConfiguration_link, labelConfiguration);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	auto menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);
	menuHelp->Append(wxID_HELP);
	auto menuBar = new wxMenuBar;
	menuBar->Append(menuFile, labelFile);
	menuBar->Append(menuSizeIcon, labelSizeIcon);
	menuBar->Append(menuView, "View");

	//auto menuWindow = new wxMenu;
	wxString labelWindowInfos = CLibResource::LoadStringFromResource(L"labelWindowInfos", 1);
	wxString labelWindowInfosLink = CLibResource::LoadStringFromResource(L"labelWindowInfosLink", 1);
	wxString labelWindowThumbnail = CLibResource::LoadStringFromResource(L"labelWindowThumbnail", 1);
	wxString labelWindowThumbnailLink = CLibResource::LoadStringFromResource(L"labelWindowThumbnailLink", 1);
	wxString labelWindowToolbar = CLibResource::LoadStringFromResource(L"labelWindowToolbar", 1);
	wxString labelWindowToolbarLink = CLibResource::LoadStringFromResource(L"labelWindowToolbarLink", 1);
	wxString labelWindowThumbnailVideo = CLibResource::LoadStringFromResource(L"labelWindowThumbnailVideo", 1);
	wxString labelWindowThumbnailVideoLink = CLibResource::LoadStringFromResource(L"labelWindowThumbnailVideoLink", 1);

	toolbarItem = new wxMenuItem(menuView, ID_WINDOWTOOLBAR, labelWindowToolbar, labelWindowToolbarLink, wxITEM_CHECK);
	thumbnailItem = new wxMenuItem(menuView, ID_WINDOWTHUMBNAIL, labelWindowThumbnail, labelWindowThumbnailLink, wxITEM_CHECK);
	thumbnailVideoItem = new wxMenuItem(menuView, ID_WINDOWTHUMBNAILVIDEO, labelWindowThumbnailVideo, labelWindowThumbnailVideoLink, wxITEM_CHECK);
	infosItem = new wxMenuItem(menuView, ID_WINDOWINFOS, labelWindowInfos, labelWindowInfosLink, wxITEM_CHECK);

	menuView->Append(infosItem);
	menuView->Append(thumbnailItem);
	menuView->Append(thumbnailVideoItem);
	menuView->Append(toolbarItem);

    menuBar->Append(menuTools, "Tools");
	//menuBar->Append(menuWindow, labelWindow);
	menuBar->Append(menuHelp, labelHelp);
	wxFrameBase::SetMenuBar(menuBar);

	wxWindow::SetLabel(wxT("RegardsLite Viewer"));
	//Connect(wxEVT_SIZE, wxSizeEventHandler(CViewerFrame::OnSize));
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CViewerFrame::OnClose));

	Connect(wxEVENT_PICTUREENDLOADING, wxCommandEventHandler(CViewerFrame::OnPictureEndLoading));
	Connect(wxID_PRINT, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnPrint));
	Connect(ID_EXPORT, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnExport));
#ifdef WIN32
	Connect(ID_ASSOCIATE, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnAssociate));
#endif

    Connect(wxID_EDIT, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnEdit));
    Connect(ID_DIAPORAMA, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnExportDiaporama));

	Connect(ID_OPENFOLDER, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnOpenFolder));
	Connect(ID_OPENFILE, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnOpenFile));

	Connect(ID_WINDOWINFOS, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnShowInfos));
	Connect(ID_WINDOWTHUMBNAIL, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnShowThumbnail));
	Connect(ID_WINDOWTHUMBNAILVIDEO, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnShowThumbnailVideo));
	Connect(ID_WINDOWTOOLBAR, wxEVT_MENU, wxCommandEventHandler(CViewerFrame::OnShowToolbar));

	Connect(ID_WINDOWUPDATESHOW, wxCommandEventHandler(CViewerFrame::OnUpdateWindowShow));

	mainWindow->Bind(wxEVT_CHAR_HOOK, &CViewerFrame::OnKeyDown, this);
	mainWindow->Bind(wxEVT_KEY_UP, &CViewerFrame::OnKeyUp, this);

	mainInterface->HideAbout();

	Connect(TIMER_LOADPICTUREEND, wxEVT_TIMER, wxTimerEventHandler(CViewerFrame::OnTimerEndLoadPicture), nullptr, this);
	Connect(TIMER_LOADPICTURE, wxEVT_TIMER, wxTimerEventHandler(CViewerFrame::OnTimerLoadPicture), nullptr, this);
	Connect(wxEVT_FULLSCREEN,  wxCommandEventHandler(CViewerFrame::OnWindowFullScreen));
    
    wxString firstFileToShow = "";
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
		firstFileToShow = config->GetLastShowPicture();
	if (fileToOpen != "")
		firstFileToShow = fileToOpen;

    if(firstFileToShow != "")
    {
        wxFileName filename(firstFileToShow);
        lastFolder = filename.GetPath();
    }

	UpdateMenuCheck();
}

void CViewerFrame::CreateWatcherIfNecessary()
{
	if (mainWindow != nullptr)
		mainWindow->CreateWatcherIfNecessary();
}


void CViewerFrame::UpdateMenuCheck()
{

	bool isPanelVisible = false;
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
	{
		config->GetShowVideoThumbnail(isPanelVisible);
		if (isPanelVisible)
		{
			thumbnailVideoItem->Check(true);
		}
		else
		{
			thumbnailVideoItem->Check(false);
		}
		config->GetShowThumbnail(isPanelVisible);
		if (isPanelVisible)
		{
			thumbnailItem->Check(true);
		}
		else
		{
			thumbnailItem->Check(false);
		}
		config->GetShowInfos(isPanelVisible);
		if (isPanelVisible)
		{
			infosItem->Check(true);
		}
		else
		{
			infosItem->Check(false);
		}
		toolbarItem->Check(true);
	}
}

void CViewerFrame::OnShowInfos(wxCommandEvent& event)
{
	int windowMode = 0;
	bool isCheck = infosItem->IsChecked();
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
	{
		windowMode = config->GetViewerMode();
		config->SetShowInfos(isCheck);
	}
	wxWindow* window = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (window != nullptr)
	{
		wxCommandEvent event(wxEVENT_SETMODEVIEWER);
		event.SetInt(windowMode);
		wxPostEvent(window, event);
	}
}

void CViewerFrame::OnShowThumbnail(wxCommandEvent& event)
{
	int windowMode = 0;
	bool isCheck = thumbnailItem->IsChecked();
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
	{
		windowMode = config->GetViewerMode();
		config->SetShowThumbnail(isCheck);
	}
	wxWindow* window = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (window != nullptr)
	{
		wxCommandEvent event(wxEVENT_SETMODEVIEWER);
		event.SetInt(windowMode);
		wxPostEvent(window, event);
	}
}


void CViewerFrame::OnUpdateWindowShow(wxCommandEvent& event)
{
	int id = event.GetInt();
	if (id == 10)
	{
		toolbarItem->Check(true);
	}
	else if (id == 11)
	{
		toolbarItem->Check(false);
	}
	else
		UpdateMenuCheck();
}

void CViewerFrame::OnShowToolbar(wxCommandEvent& event)
{
	int windowMode = WINDOW_VIEWER;
	bool isCheck = toolbarItem->IsChecked();
	wxWindow* window = this->FindWindowById(PREVIEWVIEWERID);
	if (window != nullptr)
	{
		if (isCheck)
		{
			wxCommandEvent evt(wxEVENT_SHOWTOOLBAR);
			window->GetEventHandler()->AddPendingEvent(evt);
		}
		else
		{
			wxCommandEvent evt(wxEVENT_HIDETOOLBAR);
			window->GetEventHandler()->AddPendingEvent(evt);
		}

	}
}

void CViewerFrame::OnShowThumbnailVideo(wxCommandEvent& event)
{
	int windowMode = 0;
	bool isCheck = thumbnailVideoItem->IsChecked();
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
	{
		windowMode = config->GetViewerMode();
		config->SetShowVideoThumbnail(isCheck);
	}
	wxWindow* window = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (window != nullptr)
	{
		wxCommandEvent event(wxEVENT_SETMODEVIEWER);
		event.SetInt(windowMode);
		wxPostEvent(window, event);
	}
}

void CViewerFrame::OnOpenFile(wxCommandEvent& event)
{
	wxString openPicture = CLibResource::LoadStringFromResource(L"LBLOPENPICTUREFILE", 1);

	wxFileDialog openFileDialog(nullptr, openPicture, lastFolder, "",
		"*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea..

	wxFileName filename(openFileDialog.GetPath());
	lastFolder = filename.GetPath();
	//int filterIndex = openFileDialog.Ge
    
	mainWindow->OpenFile(openFileDialog.GetPath());
}

void CViewerFrame::OnOpenFolder(wxCommandEvent& event)
{
	wxString openPicture = CLibResource::LoadStringFromResource(L"LBLOPENPICTUREFILE", 1);
	
	wxDirDialog openFileDialog(NULL, "Choose input directory", lastFolder,
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);


	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea..

    lastFolder = openFileDialog.GetPath();
	mainWindow->OpenFolder(lastFolder);
}


void CViewerFrame::OnExportDiaporama(wxCommandEvent& event)
{
    wxWindow* central = this->FindWindowById(MAINVIEWERWINDOWID);
    auto local_event = new wxCommandEvent(wxEVENT_EXPORTDIAPORAMA);
    wxQueueEvent(central, local_event);  
}

 void CViewerFrame::OnEdit(wxCommandEvent& event)
 {
    
    wxWindow* central = this->FindWindowById(MAINVIEWERWINDOWID);
    auto local_event = new wxCommandEvent(wxEVENT_EDITFILE);
    wxQueueEvent(central, local_event);    
 }

void CViewerFrame::OnWindowFullScreen(wxCommandEvent & event)
{
    //printf("Process OnWindowFullScreen /n");
    if(!fullscreen)
        SetFullscreen();
}


int CViewerFrame::ShowScanner()
{
    wxString pathProgram  = "";
#ifdef __APPLE__
    pathProgram = CFileUtility::GetProgramFolderPath() + "/RegardsLite -p RegardsPDF";
#else
#ifdef __WXMSW__
	pathProgram = "RegardsLite.exe -p RegardsPDF";
#else
	pathProgram = CFileUtility::GetProgramFolderPath() + "/RegardsLite -p RegardsPDF";
#endif
#endif


    wxExecute(pathProgram);
	return 0;
}



void CViewerFrame::OnExport(wxCommandEvent& event)
{
	wxString filename = mainWindow->GetFilename();
	if (filename != "")
	{
		CSavePicture::SavePicture(nullptr, nullptr, filename);
	}
}
#ifdef WIN32
void CViewerFrame::OnAssociate(wxCommandEvent& event)
{
	wxString path = CFileUtility::GetProgramFolderPath() + "\\associate.exe";
	ShellExecute(this->GetHWND(), L"runas", path, nullptr, nullptr, SW_SHOWNORMAL);
}
#endif
void CViewerFrame::OnPrint(wxCommandEvent& event)
{
	const wxString filename = mainWindow->GetFilename();
	if (filename != "")
	{
		CLibPicture libPicture;
		CImageLoadingFormat* image = libPicture.LoadPicture(filename);
		if (image != nullptr)
			PrintPreview(image);
	}
}



void CViewerFrame::OnClose(wxCloseEvent& event)
{
	Exit();
}



void CViewerFrame::OnHelp(wxCommandEvent& event)
{
	wxString helpFile = CFileUtility::GetResourcesFolderPath();
	helpFile.Append("//NoticeRegardsLite.pdf");
	wxLaunchDefaultApplication(helpFile);
}

void CViewerFrame::CheckAllProcessEnd(wxTimerEvent& event)
{
	nbTime++;

	if (nbTime < 50)
	{
		for (CMasterWindow* window : CMasterWindow::listMainWindow)
		{
			if (window != nullptr)
			{
				if (!window->GetProcessEnd())
				{
					const wxString message = window->GetWaitingMessage();
					mainWindowWaiting->SetTexte(message);
					exitTimer->Start(1000, wxTIMER_ONE_SHOT);
					return;
				}
			}
		}
	}

	onExit = true;
	Exit();
}

void CViewerFrame::Exit()
{
	if (!onExit)
	{
		nbTime = 0;
		CWindowMain::SetEndProgram();
		
		loadPictureTimer->Stop();
		mainWindowWaiting = new CWaitingWindow(this, wxID_ANY);
		mainWindow->Show(false);
		mainWindowWaiting->Show(true);
		mainWindowWaiting->SetSize(0, 0, mainWindow->GetWindowWidth(), mainWindow->GetWindowHeight());
		mainWindowWaiting->Refresh();

		exitTimer->Start(10, wxTIMER_ONE_SHOT);
	}
	else
	{
		CMainThemeInit::SaveTheme();
		mainInterface->Close();
		onExit = true;
	}
}


void CViewerFrame::OnTimerLoadPicture(wxTimerEvent& event)
{
    printf("void CViewerFrame::OnTimerLoadPicture(wxTimerEvent& event) \n");
	wxWindow* mainWindow = this->FindWindowById(CENTRALVIEWERWINDOWID);
	if (mainWindow != nullptr)
	{
		wxCommandEvent evt(eventToLoop);
		mainWindow->GetEventHandler()->AddPendingEvent(evt);
	}

	if (endLoadPictureTimer->IsRunning())
		endLoadPictureTimer->Stop();

	endLoadPictureTimer->Start(1000, true);

	//if (repeatEvent)
	//	loadPictureTimer->Start(200, true);
	//else
	loadPictureTimer->Stop();
}

void CViewerFrame::OnTimerEndLoadPicture(wxTimerEvent& event)
{
	pictureEndLoading = true;
}

void CViewerFrame::OnPictureEndLoading(wxCommandEvent& event)
{
	pictureEndLoading = true;
}

void CViewerFrame::OnKeyUp(wxKeyEvent& event)
{
	if (loadPictureTimer->IsRunning())
		loadPictureTimer->Stop();
}

void CViewerFrame::OnKeyDown(wxKeyEvent& event)
{
	if (event.m_keyCode == WXK_ESCAPE && fullscreen)
	{
		mainWindow->SetScreen();
	}
	else
	{
		switch (event.GetKeyCode())
		{
		case WXK_ESCAPE:
			{
				mainWindow->SetScreen();
			}
			break;

		case WXK_SPACE:
		case WXK_PAGEUP:
			{
				repeatEvent = true;
				eventToLoop = wxEVENT_PICTURENEXT;
				if (pictureEndLoading)
					loadPictureTimer->Start(50, true);
				pictureEndLoading = false;
			}
			break;

		case WXK_PAGEDOWN:
			{
				repeatEvent = true;
				eventToLoop = wxEVENT_PICTUREPREVIOUS;
				if (pictureEndLoading)
					loadPictureTimer->Start(50, true);
				pictureEndLoading = false;
			}
			break;


		case WXK_END:
			{
				repeatEvent = false;
				eventToLoop = wxEVENT_PICTURELAST;
				if (pictureEndLoading)
					loadPictureTimer->Start(50, true);
				pictureEndLoading = false;
			}
			break;

		case WXK_HOME:
			{
				repeatEvent = false;
				eventToLoop = wxEVENT_PICTUREFIRST;
				if (pictureEndLoading)
					loadPictureTimer->Start(50, true);
				pictureEndLoading = false;
			}
			break;

		case WXK_F5:
			{
				if (!fullscreen)
				{
					if (mainWindow->SetFullscreen())
						fullscreen = true;
				}
			}
			break;

		default: ;
		}
	}
	event.Skip();
}


void CViewerFrame::SetFullscreen()
{
	if (mainWindow->SetFullscreenMode())
	{
		fullscreen = true;
         //this->ShowFullScreen(true);//, wxFULLSCREEN_NOTOOLBAR | wxFULLSCREEN_NOSTATUSBAR |wxFULLSCREEN_NOBORDER);
#ifdef __APPLE__
        int top = 0, left = 0, width = 0, height = 0;
        CToggleScreen toggle;
        toggle.ToggleFullscreen(this);
        
#else
		this->ShowFullScreen(true);
#endif

	}
}

void CViewerFrame::SetScreen()
{
	fullscreen = false;
	
#ifdef __APPLE__
    CToggleScreen toggle;
    toggle.ToggleFullscreen(this);
#else
    this->ShowFullScreen(false);
#endif
}

void CViewerFrame::SetWindowTitle(const wxString& libelle)
{
	SetLabel(libelle);
}

CViewerFrame::~CViewerFrame()
{
	if (mainWindow != nullptr)
		mainWindow->SaveParameter();

	if (exitTimer->IsRunning())
		exitTimer->Stop();

	delete exitTimer;

	if (loadPictureTimer->IsRunning())
		loadPictureTimer->Stop();

	delete(loadPictureTimer);



	if (endLoadPictureTimer->IsRunning())
		endLoadPictureTimer->Stop();
	delete(endLoadPictureTimer);

	if (mainWindow != nullptr)
		delete(mainWindow);

	if (mainWindowWaiting != nullptr)
		delete(mainWindowWaiting);

	if (viewerTheme != nullptr)
		delete(viewerTheme);

	viewerParam->SaveFile();


	if (viewerParam != nullptr)
		delete(viewerParam);


	if (!onExit)
		CViewerFrame::Exit();
}

void CViewerFrame::SetText(const int& numPos, const wxString& libelle)
{
	if (mainWindow != nullptr)
		mainWindow->SetText(numPos, libelle);
}

void CViewerFrame::SetRangeProgressBar(const int& range)
{
	if (mainWindow != nullptr)
		mainWindow->SetRangeProgressBar(range);
}

void CViewerFrame::SetPosProgressBar(const int& position)
{
	if (mainWindow != nullptr)
		mainWindow->SetPosProgressBar(position);
}

void CViewerFrame::OnConfiguration(wxCommandEvent& event)
{
	auto regardsParam = CParamInit::getInstance();
	ConfigRegards configFile(this);
	configFile.ShowModal();
	configFile.IsOk();
}



void CViewerFrame::OnIconSizeLess(wxCommandEvent& event)
{
	CRegardsConfigParam* config = CParamInit::getInstance();
	if (config != nullptr)
	{
		float ratio = config->GetIconSizeRatio();
		if (ratio > 1.0)
			ratio = ratio - 0.25;
		config->SetIconSizeRatio(ratio);

		mainWindow->UpdateScreenRatio();
		//Refresh();
	}
}

void CViewerFrame::OnIconSizeMore(wxCommandEvent& event)
{
	CRegardsConfigParam* config = CParamInit::getInstance();
	if (config != nullptr)
	{
		float ratio = config->GetIconSizeRatio();
		if (ratio < 2.0)
			ratio = ratio + 0.25;
		config->SetIconSizeRatio(ratio);

		mainWindow->UpdateScreenRatio();
		//Refresh();
	}
}

void CViewerFrame::OnExit(wxCommandEvent& event)
{
	//onExit = true;
	//mainInterface->Close();
	Exit();
}

void CViewerFrame::OnAbout(wxCommandEvent& event)
{
	mainInterface->ShowAbout();
}

void CViewerFrame::OnHello(wxCommandEvent& event)
{
	wxLogMessage("Hello world from wxWidgets!");
}

void CViewerFrame::PrintImagePreview(CImageLoadingFormat* imageToPrint)
{
	// Pass two printout objects: for preview, and possible printing.
	wxPrintData* g_printData = CPrintEngine::GetPrintData();
	wxPrintDialogData printDialogData(*g_printData);
	const auto bitmapPreview = new CImageLoadingFormat();
	*bitmapPreview = *imageToPrint;
	const auto preview = new wxPrintPreview(new CBitmapPrintout(imageToPrint), new CBitmapPrintout(bitmapPreview),
	                                        &printDialogData);
	if (!preview->IsOk())
	{
		delete preview;
		wxLogError(wxT("There was a problem previewing.\nPerhaps your current printer is not set correctly?"));
		return;
	}

	const wxString picture_print_label = CLibResource::LoadStringFromResource(L"PicturePrintPreview", 1);
	auto frame =
		new wxPreviewFrame(preview, this, picture_print_label, wxPoint(100, 100), wxSize(600, 650));
	frame->Centre(wxBOTH);
	frame->InitializeWithModality(m_previewModality);
	frame->Show();

	//delete imageToPrint;
}


void CViewerFrame::PrintPreview(CImageLoadingFormat* imageToPrint)
{
	// Pass two printout objects: for preview, and possible printing.
	wxPrintData* g_printData = CPrintEngine::GetPrintData();
	wxPrintDialogData print_dialog_data(*g_printData);
	cv::Mat bitmap_preview = imageToPrint->GetMatrix().getMat();


	const auto preview = new wxPrintPreview(new CBitmapPrintout(imageToPrint), new CBitmapPrintout(bitmap_preview),
	                                        &print_dialog_data);
	if (!preview->IsOk())
	{
		delete preview;
		wxLogError(wxT("There was a problem previewing.\nPerhaps your current printer is not set correctly?"));
		return;
	}

	const wxString picture_print_label = CLibResource::LoadStringFromResource(L"PicturePrintPreview", 1);
	auto frame =
		new wxPreviewFrame(preview, this, picture_print_label, wxPoint(100, 100), wxSize(600, 650));
	frame->Centre(wxBOTH);
	frame->InitializeWithModality(m_previewModality);
	frame->Show();

	//delete imageToPrint;
}

void CViewerFrame::OnEraseDatabase(wxCommandEvent& event)
{
	const wxString erasedatabase = CLibResource::LoadStringFromResource(L"EraseDatabase", 1);
	const wxString informations = CLibResource::LoadStringFromResource(L"labelInformations", 1);
	if (wxMessageBox(erasedatabase, informations, wxYES_NO | wxICON_WARNING) == wxYES)
	{
		CSQLRemoveData::DeleteCatalog(1);
		if (mainWindow != nullptr)
		{
			wxCommandEvent evt(wxEVENT_REFRESHFOLDER);
			mainWindow->GetEventHandler()->AddPendingEvent(evt);
		}
	}
}


void CViewerFrame::OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
	wxPrintData* g_printData = CPrintEngine::GetPrintData();
	wxPageSetupDialogData* g_pageSetupData = CPrintEngine::GetPageSetupDialogData();
	(*g_pageSetupData) = *g_printData;

	wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
	pageSetupDialog.ShowModal();

	(*g_printData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
	(*g_pageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}

#ifdef __WXMAC__
void CViewerFrame::OnPageMargins(wxCommandEvent& WXUNUSED(event))
{
    wxPrintData * g_printData = CPrintEngine::GetPrintData();
    wxPageSetupDialogData * g_pageSetupData = CPrintEngine::GetPageSetupDialogData();
    (*g_pageSetupData) = *g_printData;

	wxMacPageMarginsDialog pageMarginsDialog(this, g_pageSetupData);
	pageMarginsDialog.ShowModal();

	(*g_printData) = pageMarginsDialog.GetPageSetupDialogData().GetPrintData();
	(*g_pageSetupData) = pageMarginsDialog.GetPageSetupDialogData();
}
#endif

/*

			ID_WINDOWFACE = 17,
			ID_WINDOWFOLDER = 18,
			ID_WINDOWVIEWER = 19,
			ID_WINDOWPICTURE = 20,
*/

wxBEGIN_EVENT_TABLE(CViewerFrame, wxFrame)
		EVT_MENU(ID_Hello, CViewerFrame::OnHello)
		EVT_MENU(wxID_HELP, CViewerFrame::OnHelp)
		EVT_MENU(ID_Configuration, CViewerFrame::OnConfiguration)
		EVT_MENU(ID_SIZEICONLESS, CViewerFrame::OnIconSizeLess)
		EVT_MENU(ID_SIZEICONMORE, CViewerFrame::OnIconSizeMore)
		EVT_MENU(ID_ERASEDATABASE, CViewerFrame::OnEraseDatabase)
		//EVT_MENU(ID_INTERPOLATIONFILTER, CViewerFrame::OnInterpolationFilter)
		EVT_MENU(wxID_ABOUT, CViewerFrame::OnAbout)
		EVT_MENU(WXPRINT_PAGE_SETUP, CViewerFrame::OnPageSetup)
		EVT_MENU(wxID_EXIT, CViewerFrame::OnExit)
#ifdef __WXMAC__
	EVT_MENU(WXPRINT_PAGE_MARGINS, CViewerFrame::OnPageMargins)
#endif
wxEND_EVENT_TABLE()
