#include <header.h>
#include "MainWindow.h"
#include <LibResource.h>
#include "ExportDiaporama.h"
#include "ViewerParamInit.h"
#include "ViewerParam.h"
#include <libPicture.h>
#include "window_mode_id.h"
#include <ImageLoadingFormat.h>
#include "ThumbnailViewerPicture.h"
#include "ThumbnailBuffer.h"
#include "SqlFindPhotos.h"
#include <SqlThumbnail.h>
#include <BitmapWndViewer.h>
#include "Toolbar.h"
#include <StatusBarInterface.h>
#include "CentralWindow.h"
#include "FileUtility.h"
#include <wx/filename.h>
#include <window_id.h>
#include <SQLRemoveData.h>
#include <SqlInsertFile.h>
#include "StatusText.h"
#include <ThumbnailMessage.h>
#include <SqlThumbnailVideo.h>
#include <SqlFindFolderCatalog.h>
#define LIBHEIC
#include <picture_id.h>
#include <ShowElement.h>
#include <wx/filedlg.h>
#include <SqlFaceLabel.h>
#include "SqlFacePhoto.h"
#include <FiltreEffetCPU.h>
#include "CheckVersion.h"
#include <IBitmapWnd.h>
#include <MainTheme.h>
#include <MainThemeInit.h>
#include <SqlPhotosWithoutThumbnail.h>
#include <ParamInit.h>
#include "FolderProcess.h"
#include <wx/busyinfo.h>

#include <ImageVideoThumbnail.h>
#include <ThreadLoadingBitmap.h>
#include "window_mode_id.h"
#include <wx/mimetype.h>
#include <wx/dir.h>
#include "SqlFolderCatalog.h"
using namespace Regards::Picture;
using namespace Regards::Control;
using namespace Regards::Viewer;
using namespace std;
using namespace Regards::Sqlite;

constexpr auto TIMER_EVENTFILEFS = 3;


wxDEFINE_EVENT(wxEVENT_ADDFSENTRY, wxCommandEvent);
wxDEFINE_EVENT(wxEVENT_REMOVEFSENTRY, wxCommandEvent);
wxDEFINE_EVENT(wxEVENT_UPDATECHECKINSTATUS, wxCommandEvent);
wxDEFINE_EVENT(wxEVENT_UPDATECHECKINFOLDER, wxCommandEvent);
wxDEFINE_EVENT(wxVERSION_UPDATE_EVENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVENT_SETSCREEN, wxCommandEvent);

bool firstTime = true;

class CFolderFiles
{
public:
	vector<wxString> pictureFiles;
	wxString folderName;
};

class CThreadVideoData
{
public:
	CThreadVideoData()
	{
		mainWindow = nullptr;
	}

	~CThreadVideoData();

	CMainWindow* mainWindow;
	wxString video;
};

class CThreadCheckFile
{
public:
	CThreadCheckFile()
	{
		mainWindow = nullptr;
	}

	~CThreadCheckFile()
	{
	};


	std::thread* checkFile = nullptr;
	CMainWindow* mainWindow;
	int pictureSize;
	int numFile;
};

void CMainWindow::CheckFile(void* param)
{
	CThreadCheckFile* checkFile = (CThreadCheckFile*)param;
	if (checkFile != nullptr)
	{
		int numElementTraitement = 0;

		PhotosVector* _pictures = new PhotosVector();
		CSqlFindPhotos sqlFindPhotos;
		sqlFindPhotos.SearchPhotosByCriteriaFolder(_pictures);

		wxString nbElement = to_string(_pictures->size());
		for (int i = 0; i < _pictures->size(); i++)
		{
			CPhotos photo = _pictures->at(i);

			int nbProcesseur = 1;

			if (wxFileName::FileExists(photo.GetPath()))
			{
				//Test si thumbnail valide
				CMainParam* config = CMainParamInit::getInstance();
				if (config != nullptr)
				{
					if (config->GetCheckThumbnailValidity())
					{
						CSqlThumbnail sqlThumbnail;
						CSqlThumbnailVideo sqlThumbnailVideo;
						wxFileName file(photo.GetPath());
						wxULongLong sizeFile = file.GetSize();
						wxString md5file = sizeFile.ToString();

						bool result = sqlThumbnail.TestThumbnail(photo.GetPath(), md5file);
						if (!result)
						{
							//Remove thumbnail
							sqlThumbnail.DeleteThumbnail(photo.GetPath());
							sqlThumbnailVideo.DeleteThumbnail(photo.GetPath());
						}

						wxCommandEvent evt(wxEVENT_UPDATECHECKINFOLDER);
						checkFile->mainWindow->GetEventHandler()->AddPendingEvent(evt);
					}
				}
			}
			else
			{
				//Remove file
				CSQLRemoveData::DeletePhoto(photo.GetId());
				wxCommandEvent evt(wxEVENT_UPDATECHECKINFOLDER);
				checkFile->mainWindow->GetEventHandler()->AddPendingEvent(evt);
			}

			numElementTraitement++;

			wxCommandEvent evt(wxEVENT_UPDATECHECKINSTATUS);
			evt.SetInt(numElementTraitement);
			evt.SetString(nbElement);
			checkFile->mainWindow->GetEventHandler()->AddPendingEvent(evt);

			if (checkFile->mainWindow->endApplication)
				break;
			std::this_thread::sleep_for(50ms);
		}

		delete _pictures;
	}


	wxCommandEvent evt(wxEVENT_ENDCHECKFILE);
	evt.SetClientData(checkFile);
	checkFile->mainWindow->GetEventHandler()->AddPendingEvent(evt);
}


void CMainWindow::OnEndCheckFile(wxCommandEvent& event)
{
	CThreadCheckFile* checkFile = (CThreadCheckFile*)event.GetClientData();
	if (checkFile != nullptr)
	{
		if (checkFile->checkFile != nullptr)
		{
			checkFile->checkFile->join();
			delete checkFile->checkFile;
		}

		isCheckingFile = false;
		delete checkFile;
	}
	changeFolder = false;
	processIdle = true;
}

CThreadVideoData::~CThreadVideoData()
{
}


extern wxImage defaultPicture;

void CMainWindow::OnRemoveFileFromCheckIn(wxCommandEvent& event)
{
	UpdateFolderStatic();
	processIdle = true;
}

void CMainWindow::OnCheckInUpdateStatus(wxCommandEvent& event)
{
	int numElementTraitement = event.GetInt();
	wxString nbElement = event.GetString();
	wxString label = CLibResource::LoadStringFromResource(L"LBLFILECHECKING", 1);
	wxString message = label + to_string(numElementTraitement) + L"/" + nbElement;
	if (statusBarViewer != nullptr)
	{
		statusBarViewer->SetText(3, message);
	}
}

CMainWindow::CMainWindow(wxWindow* parent, wxWindowID id, IStatusBarInterface* statusbar, const bool& openFirstFile, const wxString& fileToOpen)
	: CWindowMain("CMainWindow", parent, id)
{
	fullscreen = false;

	showToolbar = true;
	multithread = true;
	needToReload = false;
	typeAffichage = THUMB_SHOW_ALL;

	refreshFolder = false;
	numElementTraitement = 0;
	start = true;
	criteriaSendMessage = false;
	checkVersion = true;

	folderProcess = new CFolderProcess(this);


	CMainTheme* viewerTheme = CMainThemeInit::getInstance();
	viewerParam = CMainParamInit::getInstance();
	eventFileSysTimer = new wxTimer(this, TIMER_EVENTFILEFS);

	if (viewerTheme != nullptr)
	{
		CThemeSplitter theme;
		viewerTheme->GetSplitterTheme(&theme);
		centralWnd = new CCentralWindow(this, CENTRALVIEWERWINDOWID, theme, false);
	}
	this->statusBarViewer = statusbar;

	/*----------------------------------------------------------------------
	 *
	 * Manage Event
	 *
	 ----------------------------------------------------------------------*/

	Connect(wxEVENT_SETSCREEN, wxCommandEventHandler(CMainWindow::SetScreenEvent));
	Connect(wxEVENT_INFOS, wxCommandEventHandler(CMainWindow::OnUpdateInfos));
	Connect(wxEVENT_REFRESHFOLDERLIST, wxCommandEventHandler(CMainWindow::RefreshFolderList));
	Connect(TOOLBAR_UPDATE_ID, wxCommandEventHandler(CMainWindow::OnShowToolbar));
	Connect(wxEVT_IDLE, wxIdleEventHandler(CMainWindow::OnIdle));

	Connect(wxEVENT_ENDCHECKFILE, wxCommandEventHandler(CMainWindow::OnEndCheckFile));

	Connect(wxEVENT_ONPICTURECLICK, wxCommandEventHandler(CMainWindow::OnPictureClick));

	Connect(wxEVENT_PICTUREVIDEOCLICK, wxCommandEventHandler(CMainWindow::PictureVideoClick));
	Connect(wxEVENT_REFRESHFOLDER, wxCommandEventHandler(CMainWindow::InitPictures));
	Connect(wxEVENT_REFRESHPICTURE, wxCommandEventHandler(CMainWindow::OnRefreshPicture));

	Connect(wxEVENT_SETSTATUSTEXT, wxCommandEventHandler(CMainWindow::OnStatusSetText));
	Connect(wxEVT_EXIT, wxCommandEventHandler(CMainWindow::OnExit));
	Connect(wxEVENT_SETRANGEPROGRESSBAR, wxCommandEventHandler(CMainWindow::OnSetRangeProgressBar));
	Connect(wxEVENT_PRINTPICTURE, wxCommandEventHandler(CMainWindow::PrintPreview));

	Connect(wxEVENT_UPDATESTATUSBARMESSAGE, wxCommandEventHandler(CMainWindow::UpdateStatusBarMessage));

	Connect(wxEVENT_PRINT, wxCommandEventHandler(CMainWindow::OnPrint));
	Connect(wxEVENT_SETVALUEPROGRESSBAR, wxCommandEventHandler(CMainWindow::OnSetValueProgressBar));

	Connect(wxEVENT_OPENFILEORFOLDER, wxCommandEventHandler(CMainWindow::OnOpenFileOrFolder));
	Connect(wxEVENT_EDITFILE, wxCommandEventHandler(CMainWindow::OnEditFile));
	Connect(wxEVENT_EXPORTFILE, wxCommandEventHandler(CMainWindow::OnExportFile));
	Connect(wxEVENT_ENDCOMPRESSION, wxCommandEventHandler(CMainWindow::OnEndDecompressFile));
	Connect(wxEVENT_UPDATETHUMBNAILEXIF, wxCommandEventHandler(CMainWindow::OnUpdateExifThumbnail));
	Connect(wxEVENT_EXPORTDIAPORAMA, wxCommandEventHandler(CMainWindow::OnExportDiaporama));

	Connect(wxEVENT_ICONEUPDATE, wxCommandEventHandler(CMainWindow::UpdateThumbnailIcone));
	Connect(wxVERSION_UPDATE_EVENT, wxCommandEventHandler(CMainWindow::OnVersionUpdate));
	Connect(wxEVENT_UPDATEMESSAGE, wxCommandEventHandler(CMainWindow::UpdateMessage));
	Connect(wxEVENT_REFRESHTHUMBNAIL, wxCommandEventHandler(CMainWindow::OnRefreshThumbnail));
	Connect(wxEVENT_ICONETHUMBNAILGENERATION, wxCommandEventHandler(CMainWindow::OnProcessThumbnail));
	Connect(wxEVENT_ENDOPENEXTERNALFILE, wxCommandEventHandler(CMainWindow::OnEndOpenExternalFile));

	Connect(wxEVENT_ADDFSENTRY, wxCommandEventHandler(CMainWindow::OnAddFSEntry));
	Connect(wxEVENT_REMOVEFSENTRY, wxCommandEventHandler(CMainWindow::OnRemoveFSEntry));
	Connect(wxEVENT_UPDATECHECKINSTATUS, wxCommandEventHandler(CMainWindow::OnCheckInUpdateStatus));
	Connect(wxEVENT_UPDATECHECKINFOLDER, wxCommandEventHandler(CMainWindow::OnRemoveFileFromCheckIn));

	Connect(TIMER_EVENTFILEFS, wxEVT_TIMER, wxTimerEventHandler(CMainWindow::OnTimereventFileSysTimer), nullptr, this);
	/*----------------------------------------------------------------------
	 *
	 * Manage Event
	 *
	 ----------------------------------------------------------------------*/

	statusBar = new wxStatusBar(this, wxID_ANY, wxSTB_DEFAULT_STYLE, "wxStatusBar");

	int tabWidth[] = { 100, 300, 300, 300 };
	statusBar->SetFieldsCount(4);
	statusBar->SetStatusWidths(4, tabWidth);

	progressBar = new wxGauge(statusBar, wxID_ANY, 200, wxPoint(1000, 0), wxSize(200, statusBar->GetSize().y),
		wxGA_HORIZONTAL);
	progressBar->SetRange(100);
	progressBar->SetValue(50);
	this->fileToOpen = fileToOpen;
	//updateFolder = true;
	listProcessWindow.push_back(this);
	CMainParam* config = CMainParamInit::getInstance();
	if (config != nullptr)
		firstFileToShow = localFilename = config->GetLastShowPicture();
	if (fileToOpen != "")
		firstFileToShow = localFilename = fileToOpen;



	m_watcher = new wxFileSystemWatcher();
	m_watcher->SetOwner(this);
	Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(CMainWindow::OnFileSystemModified));

	//UpdateFolderStatic();
	CSqlPhotosWithoutThumbnail sqlPhoto;
	sqlPhoto.GetPhotoList(&photoList, 0);
	versionUpdate = new std::thread(NewVersionAvailable, this);
	openExternalFileThread = new std::thread(OpenExternalFile, this);

	isCheckNewVersion = true;

	refreshFolder = true;
	processIdle = true;
	
	endApplication = false;

	bool find = false;

	FolderCatalogVector folderList;
	CSqlFindFolderCatalog folderCatalog;
	folderCatalog.GetFolderCatalog(&folderList, NUMCATALOGID);
	CheckDatabase(folderList);
}

bool CMainWindow::CheckDatabase(FolderCatalogVector& folderList)
{
	wxString libelle = CLibResource::LoadStringFromResource(L"LBLBUSYINFO", 1);
	wxBusyCursor busy;
	//wxBusyInfo wait(libelle);

	bool folderChange = false;

	//Test de la validité des répertoires
	for (CFolderCatalog folderlocal : folderList)
	{
		if (!wxDirExists(folderlocal.GetFolderPath()))
		{
			//Remove Folder
			CSQLRemoveData::DeleteFolder(folderlocal.GetNumFolder());
			folderChange = true;
		}
		else
		{
			AddFSEntry(folderlocal.GetFolderPath());
		}
	}


	//Test de la validité des fichiers
	PhotosVector photoList;
	CSqlThumbnail sqlThumbnail;
	CSqlFindPhotos findphotos;
	findphotos.GetAllPhotos(&photoList);
	for (CPhotos photo : photoList)
	{
		if (!wxFileExists(photo.GetPath()))
		{
			//Remove Folder
			CSQLRemoveData::DeletePhoto(photo.GetId());
			folderChange = true;
		}
	}

	//Thumbnail Photo Verification

	vector<int> listPhoto = sqlThumbnail.GetAllPhotoThumbnail();
	for (int numPhoto : listPhoto)
	{
		wxString thumbnail = CFileUtility::GetThumbnailPath(to_string(numPhoto));
		if (!wxFileExists(thumbnail))
		{
			sqlThumbnail.EraseThumbnail(numPhoto);
		}
	}


	return folderChange;
}

void CMainWindow::OnFileSystemModified(wxFileSystemWatcherEvent& event)
{
	if (eventFileSysTimer != nullptr)
	{
		eventFileSysTimer->Stop();
		eventFileSysTimer->Start(1000);
	}
}

void CMainWindow::OnTimereventFileSysTimer(wxTimerEvent& event)
{
	//printf("OnFileSystemModified \n");
	const wxCommandEvent evt(wxEVENT_REFRESHFOLDER);
	this->GetEventHandler()->AddPendingEvent(evt);
	eventFileSysTimer->Stop();
}


void CMainWindow::OnRemoveFSEntry(wxCommandEvent& event)
{
	wxString* dirPath = (wxString*)event.GetClientData();
	if (dirPath != nullptr)
	{
		if (m_watcher == nullptr)
			return;

		if (wxDirExists(*dirPath) == false)
			return;

		const wxFileName dirname(*dirPath, "");
		m_watcher->Remove(dirname);

		delete dirPath;
	}

}

void CMainWindow::OnAddFSEntry(wxCommandEvent& event)
{
	wxString* dirPath = (wxString*)event.GetClientData();
	if (dirPath != nullptr)
	{
		if (m_watcher == nullptr)
			return;

		if (wxDirExists(*dirPath) == false)
			return;

		const wxFileName dirname(*dirPath, "");
		m_watcher->AddTree(
			dirname, wxFSW_EVENT_CREATE | wxFSW_EVENT_DELETE | wxFSW_EVENT_RENAME | wxFSW_EVENT_MODIFY);

		delete dirPath;
	}
}

bool CMainWindow::RemoveFSEntry(const wxString& dirPath)
{
	wxString* data = new wxString(dirPath);
	auto localevent = new wxCommandEvent(wxEVENT_REMOVEFSENTRY);
	localevent->SetClientData(data);
	wxQueueEvent(this, localevent);
	return true;
}

bool CMainWindow::AddFSEntry(const wxString& dirPath)
{
	wxString* data = new wxString(dirPath);
	auto localevent = new wxCommandEvent(wxEVENT_ADDFSENTRY);
	localevent->SetClientData(data);
	wxQueueEvent(this, localevent);
	return true;
}

void CMainWindow::StartOpening()
{
	if (fileToOpen != "")
	{
		OpenFile(fileToOpen);
	}
	else
	{
		UpdateFolderStatic();

		//Verify the old data

		int pictureSize = CThumbnailBuffer::GetVectorSize();
		CThreadCheckFile* checkFile = new CThreadCheckFile();
		checkFile->mainWindow = this;
		checkFile->pictureSize = pictureSize;
		checkFile->numFile = numElementTraitement;
		checkFile->checkFile = new std::thread(CheckFile, checkFile);
		isCheckingFile = true;
		std::this_thread::sleep_for(100ms);
	}

	//refreshFolder = true;
	
}

void CMainWindow::OpenExternalFile(void* param)
{
	CMainWindow* local = (CMainWindow*)param;
	if (local != nullptr)
		local->StartOpening();

	auto localevent = new wxCommandEvent(wxEVENT_ENDOPENEXTERNALFILE);
	wxQueueEvent(local, localevent);
}

void CMainWindow::OnEndOpenExternalFile(wxCommandEvent& event)
{
	if (openExternalFileThread->joinable())
	{
		openExternalFileThread->join();
		delete openExternalFileThread;
		openExternalFileThread = nullptr;
	}

	processIdle = true;
}

void CMainWindow::OnRefreshThumbnail(wxCommandEvent& event)
{
	nbProcess = 0;
	listFile.clear();
	processIdle = true;
	CSqlPhotosWithoutThumbnail sqlPhoto;
	sqlPhoto.GetPhotoList(&photoList, 0);
}

void CMainWindow::UpdateThumbnailIcone(wxCommandEvent& event)
{
	printf("CMainWindow::UpdateThumbnailIcone \n");


	nbProcess--;
	auto localevent = new wxCommandEvent(wxEVENT_ICONEUPDATE);
	localevent->SetClientData(event.GetClientData());
	wxQueueEvent(centralWnd, localevent);
}



void CMainWindow::OnVersionUpdate(wxCommandEvent& event)
{
	cout << "OnVersionUpdate" << endl;


	int hasUpdate = event.GetInt();

	if (versionUpdate != nullptr)
	{
		versionUpdate->join();
		delete versionUpdate;
		versionUpdate = nullptr;
	}

	isCheckNewVersion = false;


	if (hasUpdate)
	{
		wxString title = CLibResource::LoadStringFromResource("LBLINFOS", 1);
		wxString infos = CLibResource::LoadStringFromResource("LBLNEWVERSIONAVAILABLE", 1);

		wxMessageBox(infos, title, wxOK | wxICON_INFORMATION);
	}


}



void CMainWindow::NewVersionAvailable(void* param)
{
	int hasUpdate = 0;
	CToolbar* toolbar = (CToolbar*)param;
	wxString localVersion = CLibResource::LoadStringFromResource("REGARDSVERSION", 1);
	wxString serverURL = CLibResource::LoadStringFromResource("ADRESSEWEBVERSION", 1);
	CCheckVersion _checkVersion(serverURL);
	wxString serverVersion = _checkVersion.GetLastVersion();
	serverVersion = serverVersion.SubString(0, serverVersion.length() - 2);

	long localValueVersion;
	long localServerVersion;

	localVersion.Replace(".", "");
	serverVersion.Replace(".", "");

	if (!localVersion.ToLong(&localValueVersion)) { /* error! */ }
	if (!serverVersion.ToLong(&localServerVersion)) { /* error! */ }

	printf("serverVersion %d \n", localServerVersion);
	printf("localVersion %d \n", localValueVersion);

	if (serverVersion != "error" && serverVersion != "")
	{
		if (localValueVersion < localServerVersion)
		{
			hasUpdate = 1;
		}
	}

	wxCommandEvent event(wxVERSION_UPDATE_EVENT);
	event.SetInt(hasUpdate);
	wxPostEvent(toolbar, event);
}




bool CMainWindow::IsVideo()
{
	if (centralWnd != nullptr)
		return centralWnd->IsVideo();

	return false;
}


void CMainWindow::OnExportDiaporama(wxCommandEvent& event)
{
	if (exportDiaporama != nullptr)
	{
		exportDiaporama->OnExportDiaporama();
	}
}

void CMainWindow::OnUpdateExifThumbnail(wxCommandEvent& event)
{
	int numPhoto = event.GetInt();
	int rotate = event.GetExtraLong();

	wxString thumbnail = CFileUtility::GetThumbnailPath(to_string(numPhoto));
	CFiltreEffetCPU::LoadAndRotate(thumbnail, rotate);
	wxWindow* window = this->FindWindowById(THUMBNAILVIEWERPICTURE);
	if (window != nullptr)
	{
		wxCommandEvent evt(wxEVENT_REFRESHTHUMBNAIL);
		evt.SetInt(numPhoto);
		window->GetEventHandler()->AddPendingEvent(evt);
	}
}

void CMainWindow::OnEndDecompressFile(wxCommandEvent& event)
{
	if (exportDiaporama != nullptr)
	{
		int ret = event.GetInt();
		exportDiaporama->OnEndDecompressFile(ret);
		delete exportDiaporama;
		exportDiaporama = nullptr;
	}
}

void CMainWindow::ExportVideo(const wxString& filename)
{
	if (!wxFileExists(filename))
		return;

	if (exportDiaporama != nullptr)
		delete exportDiaporama;

	exportDiaporama = new CExportDiaporama(this);
	exportDiaporama->ExportVideo(filename);
}

void CMainWindow::OnExportFile(wxCommandEvent& event)
{
	if (centralWnd->IsVideo())
	{
		wxString pathProgram = "";
#ifdef __APPLE__
		//ExportVideo(this->centralWnd->GetFilename());
		pathProgram = CFileUtility::GetProgramFolderPath() + "/RegardsLite \"" + this->centralWnd->GetFilename() + "\" -p RegardsConverter";
		cout << "Path Program" << pathProgram << endl;
#else
#ifdef __WXMSW__
		pathProgram = "RegardsLite.exe \"" + this->centralWnd->GetFilename() + "\"  -p RegardsConverter";
#else
		pathProgram = CFileUtility::GetProgramFolderPath() + "/RegardsLite \"" + this->centralWnd->GetFilename() + "\" -p RegardsConverter";
		cout << "Path Program" << pathProgram << endl;
#endif

#endif
		wxExecute(pathProgram);

		this->SetFocus();  // focus on my window
		this->Raise();  // bring window to front
		this->Show(true); // show the window

	}
	else
	{
		CBitmapWndViewer* viewer = nullptr;
		auto bitmapWindow = dynamic_cast<IBitmapWnd*>(FindWindowById(BITMAPWINDOWVIEWERID));
		if (bitmapWindow != nullptr)
		{
			viewer = static_cast<CBitmapWndViewer*>(bitmapWindow->GetWndPt());
		}

		//auto bitmapWindow = dynamic_cast<CBitmapWndViewer*>(this->FindWindowById(BITMAPWINDOWVIEWERID));
		if (viewer != nullptr)
			viewer->ExportPicture();
	}
}

void CMainWindow::OnEditFile(wxCommandEvent& event)
{
	CMainParam* config = CMainParamInit::getInstance();
	wxString pathProgram = "";
	wxString title = CLibResource::LoadStringFromResource(L"LBLSELECTPICTUREEDITOR", 1);
	if (centralWnd->IsVideo())
	{
		if (config != nullptr)
			pathProgram = config->GetPathForVideoEdit();

		title = CLibResource::LoadStringFromResource(L"LBLSELECTVIDEOEDITOR", 1);
	}
	else
	{
		if (config != nullptr)
			pathProgram = config->GetPathForPictureEdit();
	}

	if (!wxFileExists(pathProgram))
		pathProgram = "";

	if (pathProgram.empty())
	{

		const wxString allfiles = CLibResource::LoadStringFromResource(L"LBLALLFILES", 1);
		wxFileDialog openFileDialog(nullptr, title, "", "",
			allfiles, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		wxString documentPath = CFileUtility::GetDocumentFolderPath();
		openFileDialog.SetDirectory(documentPath);
		if (openFileDialog.ShowModal() == wxID_OK)
			pathProgram = openFileDialog.GetPath();
	}

	if (centralWnd->IsVideo())
	{
		if (config != nullptr)
			config->SetPathForVideoEdit(pathProgram);
	}
	else
	{
		if (config != nullptr)
			config->SetPathForPictureEdit(pathProgram);
	}


	pathProgram = "\"" + pathProgram + "\" \"" + localFilename + "\"";
	wxExecute(pathProgram);
}


void CMainWindow::OnPrint(wxCommandEvent& event)
{
	bool showPrintPicture = true;
	if (centralWnd->IsVideo())
	{
		auto video = static_cast<CShowElement*>(this->FindWindowById(SHOWBITMAPVIEWERID));
		if (video != nullptr)
		{
			if (video->IsPause())
			{
				cv::Mat image = video->GetVideoBitmap();
				if (!image.empty())
				{
					auto imageLoading = new CImageLoadingFormat();
					imageLoading->SetPicture(image);
					statusBarViewer->PrintImagePreview(imageLoading);
					showPrintPicture = false;
				}
			}
		}
	}
	if (showPrintPicture)
	{
		CLibPicture libPicture;
		CImageLoadingFormat* image = libPicture.LoadPicture(localFilename);
		if (image != nullptr)
		{
			statusBarViewer->PrintPreview(image);
		}
	}
}


void CMainWindow::SetDataToStatusBar(void* thumbMessage, const wxString& picture)
{
	const auto thumbnailMessage = static_cast<CThumbnailMessage*>(thumbMessage);
	const wxString message = picture + to_string(thumbnailMessage->nbPhoto);

	if (thumbnailMessage != nullptr)
	{
		if (statusBarViewer != nullptr)
		{
			statusBarViewer->SetRangeProgressBar(thumbnailMessage->nbElement);
			statusBarViewer->SetText(2, message);
			statusBarViewer->SetPosProgressBar(thumbnailMessage->thumbnailPos + 1);
		}
		delete thumbnailMessage;
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::UpdateStatusBarMessage(wxCommandEvent& event)
{
	const auto thumbnailMessage = static_cast<CThumbnailMessage*>(event.GetClientData());
	if (thumbnailMessage != nullptr)
	{
		const int typeMessage = thumbnailMessage->typeMessage;

		switch (typeMessage)
		{
		case 0:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLCRITERIANBIMAGE", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		case 1:
		{
			if (statusBarViewer != nullptr)
			{
				statusBarViewer->SetRangeProgressBar(thumbnailMessage->nbElement);
				statusBarViewer->SetPosProgressBar(0);
			}
			delete thumbnailMessage;
		}
		break;

		case 2:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLFOLDERPROCESSING", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		case 3:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLPICTURERENDER", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		case 4:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLFACEPROCESS", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		case 5:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLFACERECOGNITIONPROCESS", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		case 6:
		{
			const wxString picture = CLibResource::LoadStringFromResource(L"LBLGEOLOCALISATIONGPS", 1);
			SetDataToStatusBar(event.GetClientData(), picture);
		}
		break;

		default:;
		}
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::PrintPreview(wxCommandEvent& event)
{
	const auto bitmap = static_cast<CImageLoadingFormat*>(event.GetClientData());

	if (bitmap != nullptr)
	{
		statusBarViewer->PrintImagePreview(bitmap);
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::RefreshFolderList(wxCommandEvent& event)
{
	UpdateFolderStatic();
	//processIdle = true;
}



//---------------------------------------------------------------
//
//---------------------------------------------------------------
wxString CMainWindow::GetFilename()
{
	return localFilename;
}


//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::OnShowToolbar(wxCommandEvent& event)
{
	ShowToolbar();
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::UpdateScreenRatio()
{
	//toolbar->UpdateScreenRatio();
	centralWnd->UpdateScreenRatio();
	this->Resize();
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::OnStatusSetText(wxCommandEvent& event)
{
	auto statusText = static_cast<CStatusText*>(event.GetClientData());
	if (statusText != nullptr)
	{
		statusBar->SetStatusText(statusText->text, statusText->position);
		delete statusText;
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::OnSetValueProgressBar(wxCommandEvent& event)
{
	int position = event.GetInt();
	//cout << "OnSetValueProgressBar Pos : " << position << endl;
	if (progressBar != nullptr)
	{
		if (progressBar->GetRange() > 0)
		{
			if (position >= progressBar->GetRange())
				progressBar->SetValue(progressBar->GetRange() - 1);
			else
				progressBar->SetValue(position);

			progressBar->Refresh();
		}
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::OnSetRangeProgressBar(wxCommandEvent& event)
{
	int range = event.GetInt();
	// cout << "OnSetRangeProgressBar Pos : " << range << endl;
	if (progressBar != nullptr)
		progressBar->SetRange(range);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::SetText(const int& numPos, const wxString& libelle)
{
	auto event = new wxCommandEvent(wxEVENT_SETSTATUSTEXT);
	auto statusText = new CStatusText();
	statusText->position = numPos;
	statusText->text = libelle;
	event->SetClientData(statusText);
	wxQueueEvent(this, event);

	//statusBar->SetStatusText(libelle, numPos);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::SetRangeProgressBar(const int& range)
{
	auto event = new wxCommandEvent(wxEVENT_SETRANGEPROGRESSBAR);
	event->SetInt(range);
	wxQueueEvent(this, event);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::SetPosProgressBar(const int& position)
{
	auto event = new wxCommandEvent(wxEVENT_SETVALUEPROGRESSBAR);
	event->SetInt(position);
	wxQueueEvent(this, event);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool CMainWindow::FindNextValidFile()
{
	bool isFound = false;
	wxString lastFile = centralWnd->ImageFin(false);
	do
	{
		isFound = CThumbnailBuffer::FindValidFile(localFilename);

		if (!isFound)
		{
			if (lastFile == localFilename)
				break;

			localFilename = centralWnd->ImageSuivante(false);
		}
	} while (!isFound);

	return isFound;
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool CMainWindow::FindPreviousValidFile()
{
	bool isFound = false;
	wxString firstFile = centralWnd->ImageDebut(false);
	do
	{
		isFound = CThumbnailBuffer::FindValidFile(localFilename);

		if (!isFound)
		{
			if (firstFile == localFilename)
				break;

			localFilename = centralWnd->ImagePrecedente(false);
		}
	} while (!isFound);

	return isFound;
}




void CMainWindow::UpdateFolderStatic()
{
	printf("CMainWindow::UpdateFolderStatic() \n");
	//
	//wxString libelle = CLibResource::LoadStringFromResource(L"LBLBUSYINFO", 1);
	//wxBusyInfo wait(libelle);

	wxBusyCursor busy;
	{
		CSqlFindPhotos sqlFindPhotos;

		int typeAffichage = 0;

		if (firstFileToShow != "")
			localFilename = firstFileToShow;
		else
			localFilename = centralWnd->GetFilename();

		PhotosVector* _pictures = new PhotosVector();
		sqlFindPhotos.GetAllPhotos(_pictures);


		CThumbnailBuffer::InitVectorList(_pictures);

		if (firstFileToShow == "")
		{
			bool isFound = false;

			if (!isFound && CThumbnailBuffer::GetVectorSize() > 0 && localFilename != "")
			{
				isFound = FindNextValidFile();
				if (!isFound)
					isFound = FindPreviousValidFile();
			}

			if (!isFound && CThumbnailBuffer::GetVectorSize() > 0)
				localFilename = CThumbnailBuffer::GetVectorValue(0).GetPath();
		}


		centralWnd->SetListeFile(localFilename, typeAffichage);
		listFile.clear();
		thumbnailPos = 0;
		firstFileToShow = "";
		numElementTraitement = 0;
		nbElementInIconeList = CThumbnailBuffer::GetVectorSize();
		init = true;


	}

}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::PhotoProcess(CPhotos* photo)
{
	int nbProcesseur = 1;

	if (wxFileName::FileExists(photo->GetPath()))
	{
		//Test si thumbnail valide
		CMainParam* config = CMainParamInit::getInstance();
		if (config != nullptr)
		{
			numElementTraitement++;
		}
	}
	else
	{
		//Remove file
		CSQLRemoveData::DeletePhoto(photo->GetId());
		UpdateFolderStatic();
		numElementTraitement++;
		processIdle = true;
	}

	wxString label = CLibResource::LoadStringFromResource(L"LBLFILECHECKING", 1);
	wxString message = label + to_string(numElementTraitement) + L"/" + to_string(CThumbnailBuffer::GetVectorSize());
	if (statusBarViewer != nullptr)
	{
		statusBarViewer->SetText(3, message);
	}
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::ProcessIdle()
{
	bool hasDoneOneThings = false;
	int pictureSize = CThumbnailBuffer::GetVectorSize();

	if (refreshFolder)
	{
		int nbFile = 0;
		bool folderChange = false;
		folderProcess->RefreshFolder(folderChange, nbFile);
		if (folderChange || nbFile > 0)
		{
            printf("UpdateFolderStatic \n");
			UpdateFolderStatic();
			processIdle = true;

			photoList.clear();
			CSqlPhotosWithoutThumbnail sqlPhoto;
			sqlPhoto.GetPhotoList(&photoList, 0);
		}
		refreshFolder = false;
		numElementTraitement = 0;

		hasDoneOneThings = true;
	}
	else if (numElementTraitement < pictureSize)
	{
		/*
		if (!isCheckingFile)
		{
			CThreadCheckFile* checkFile = new CThreadCheckFile();
			checkFile->mainWindow = this;
			checkFile->pictureSize = pictureSize;
			checkFile->numFile = numElementTraitement;
			checkFile->checkFile = new std::thread(CheckFile, checkFile);
			isCheckingFile = true;
			std::this_thread::sleep_for(100ms);
		}
		*/
		//hasDoneOneThings = true;
	}


	ProcessThumbnail();

	if (hasDoneOneThings)
		processIdle = true;

}

void CMainWindow::ProcessThumbnail()
{


	if (nbElementInIconeList == 0)
	{
		return;
	}



	int nbProcesseur = 1;
	if (CRegardsConfigParam* config = CParamInit::getInstance(); config != nullptr)
		nbProcesseur = config->GetThumbnailProcess();

	for (int i = 0; i < photoList.size(); i++)
	{
		if (nbProcess >= nbProcesseur)
			return;

		wxString path = photoList[i];

		auto event = new wxCommandEvent(wxEVENT_UPDATEMESSAGE);
		event->SetExtraLong(photoList.size());
		wxQueueEvent(this, event);

		ProcessThumbnail(path, 0, 0);


		std::map<wxString, bool>::iterator it = listFile.find(path);
		if (it == listFile.end())
		{
			ProcessThumbnail(path, 0, 0);
			listFile[path] = true;
		}

		photoList.erase(photoList.begin() + i);


		i--;

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}



	if (photoList.empty())
	{
		CSqlPhotosWithoutThumbnail sqlPhoto;
		sqlPhoto.GetPhotoList(&photoList, 0);
	}
	if (photoList.empty())
	{
		nbElement = 0;
		processIdle = false;
		needToRefresh = true;


		auto event = new wxCommandEvent(wxEVENT_UPDATEMESSAGE);
		event->SetExtraLong(nbElement);
		wxQueueEvent(this, event);


	}
	else
		processIdle = true;
}

void CMainWindow::UpdateMessage(wxCommandEvent& event)
{
	const int nbPhoto = event.GetExtraLong();
	const auto thumbnailMessage = new CThumbnailMessage();
	if (nbPhoto > 0)
	{
		thumbnailMessage->nbPhoto = nbPhoto;
		thumbnailMessage->thumbnailPos = thumbnailPos;
		thumbnailMessage->nbElement = nbElementInIconeList;
		thumbnailMessage->typeMessage = 3;
		wxWindow* mainWnd = FindWindowById(MAINVIEWERWINDOWID);
		if (mainWnd != nullptr)
		{
			wxCommandEvent eventChange(wxEVENT_UPDATESTATUSBARMESSAGE);
			eventChange.SetClientData(thumbnailMessage);
			eventChange.SetInt(3);
			mainWnd->GetEventHandler()->AddPendingEvent(eventChange);
		}
		thumbnailPos++;
	}


}

void CMainWindow::OnProcessThumbnail(wxCommandEvent& event)
{
	wxString firstFile = "";
	wxString* filename = (wxString*)event.GetClientData();
	wxString localName = wxString(*filename);

	CLibPicture libPicture;
	int iFormat = libPicture.TestImageFormat(localName);

	int nbProcesseur = 1;
	if (CRegardsConfigParam* config = CParamInit::getInstance(); config != nullptr)
		nbProcesseur = config->GetThumbnailProcess() + 1;

	if (nbProcess < nbProcesseur)
	{
		int type = event.GetInt();
		int longWindow = event.GetExtraLong();
		if (type == 1)
		{
			ProcessThumbnail(localName, type, longWindow);
		}
		else
		{
			std::map<wxString, bool>::iterator it = listFile.find(localName);
			if (it == listFile.end())
			{
				std::vector<wxString>::iterator itPhoto = std::find(photoList.begin(), photoList.end(), localName);
				if (itPhoto != photoList.end())
					photoList.erase(itPhoto);

				ProcessThumbnail(localName, type, longWindow);
				listFile[localName] = true;
			}
		}
	}
	else if (photoList.size() > 0)
	{
		firstFile = *filename;

		for (int i = 0; i < photoList.size(); i++)
		{
			wxString oldFile = photoList[i];
			if (*filename == oldFile)
			{
				photoList[i] = firstFile;
				break;
			}
			else
			{
				photoList[i] = firstFile;
				firstFile = oldFile;
			}
		}
		processIdle = true;
	}


	delete filename;
}

void CMainWindow::ProcessThumbnail(wxString filename, int type, long longWindow)
{
	int nbProcesseur = 1;
	if (CRegardsConfigParam* config = CParamInit::getInstance(); config != nullptr)
		nbProcesseur = config->GetThumbnailProcess() + 1;

	if (nbProcess >= nbProcesseur)
		return;

	if (filename != "")
	{
		nbProcess++;
		auto pLoadBitmap = new CThreadLoadingBitmap();
		pLoadBitmap->filename = filename;
		pLoadBitmap->window = this;
		pLoadBitmap->longWindow = longWindow;
		pLoadBitmap->type = type;
		pLoadBitmap->_thread = new thread(LoadPicture, pLoadBitmap);

	}
	else
	{
		printf("error");
	}

}


void CMainWindow::LoadPicture(void* param)
{

	//std::thread* t1 = nullptr;
	CLibPicture libPicture;
	auto threadLoadingBitmap = static_cast<CThreadLoadingBitmap*>(param);
	if (threadLoadingBitmap == nullptr)
		return;

	CImageLoadingFormat* imageLoad = libPicture.LoadThumbnail(threadLoadingBitmap->filename);
	if (imageLoad != nullptr)
	{
		threadLoadingBitmap->bitmapIcone = imageLoad->GetMatrix().getMat();
		delete imageLoad;
	}

	if (!threadLoadingBitmap->bitmapIcone.empty())
	{
		//Enregistrement en base de données
		CSqlThumbnail sqlThumbnail;
		wxFileName file(threadLoadingBitmap->filename);
		wxULongLong sizeFile = file.GetSize();
		wxString hash = sizeFile.ToString();
		wxString localName = sqlThumbnail.InsertThumbnail(threadLoadingBitmap->filename, threadLoadingBitmap->bitmapIcone.size().width, threadLoadingBitmap->bitmapIcone.size().height, hash);
		if (localName != "")
		{
			//threadLoadingBitmap->bitmapIcone.SaveFile(localName, wxBITMAP_TYPE_JPEG);
			cv::imwrite(CConvertUtility::ConvertToStdString(localName), threadLoadingBitmap->bitmapIcone);
		}

	}

	auto event = new wxCommandEvent(wxEVENT_ICONEUPDATE);
	event->SetClientData(threadLoadingBitmap);
	wxQueueEvent(threadLoadingBitmap->window, event);
}


//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::OnIdle(wxIdleEvent& evt)
{
	if (needToRefresh)
	{
		this->Refresh();
		needToRefresh = false;
	}

	StartThread();
}



//---------------------------------------------------------------
//
//---------------------------------------------------------------
CMainWindow::~CMainWindow()
{

	if (eventFileSysTimer->IsRunning())
		eventFileSysTimer->Stop();

	delete(eventFileSysTimer);
	delete(progressBar);
	delete(statusBar);
	delete(centralWnd);
	//delete(toolbar);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void CMainWindow::SaveParameter()
{
	if (centralWnd != nullptr)
		centralWnd->SaveParameter();

}

void CMainWindow::Resize()
{
	if (!fullscreen)
	{
		const wxSize sizeStatusBar = statusBar->GetSize();

		//centralWnd->SetSize(0, toolbar->GetNavigatorHeight() + toolbarViewerMode->GetNavigatorHeight(), GetWindowWidth(), GetWindowHeight() - (toolbarViewerMode->GetNavigatorHeight() + toolbar->GetNavigatorHeight() + sizeStatusBar.y));
		centralWnd->SetSize(0, 0, GetWindowWidth(), GetWindowHeight() - (sizeStatusBar.y));
		centralWnd->Refresh();

		statusBar->SetSize(0, GetWindowHeight() - sizeStatusBar.y, GetWindowWidth(), sizeStatusBar.y);
		statusBar->Refresh();
	}
	else
	{
		centralWnd->SetSize(0, 0, GetWindowWidth(), GetWindowHeight());
		centralWnd->Refresh();
	}
}

void CMainWindow::PictureVideoClick(wxCommandEvent& event)
{
	const long timePosition = event.GetExtraLong();
	if (centralWnd != nullptr)
	{
		centralWnd->SetPosition(timePosition);
	}
}


void CMainWindow::OnPictureClick(wxCommandEvent& event)
{
	const int photoId = event.GetExtraLong();
	wxString filename = CThumbnailBuffer::FindPhotoById(photoId);
	centralWnd->LoadPicture(filename);
}

void CMainWindow::OnUpdateFolder(wxCommandEvent& event)
{
	int typeId = event.GetInt();

	if (typeId == 0)
	{
		const auto newPath = static_cast<wxString*>(event.GetClientData());
		if (newPath != nullptr)
		{
			firstFileToShow = *newPath;
			delete newPath;
		}
	}


	//wxString libelle = CLibResource::LoadStringFromResource(L"LBLBUSYINFO", 1);
	wxBusyCursor busy;
	{
		photoList.clear();
		CSqlPhotosWithoutThumbnail sqlPhoto;
		sqlPhoto.GetPhotoList(&photoList, 0);
	}


	UpdateFolderStatic();
	processIdle = true;
	//this->Show(true);
}



void CMainWindow::TransitionEnd()
{
	centralWnd->TransitionEnd();

	//if (!centralWnd->IsDiaporamaStart())
	//{
	auto showBitmap = static_cast<CShowElement*>(this->FindWindowById(SHOWBITMAPVIEWERID));
	if (showBitmap != nullptr)
	{
		showBitmap->TransitionEnd();
	}
	//}
}


void CMainWindow::OnUpdateInfos(wxCommandEvent& event)
{
#if defined(WIN32) && defined(_DEBUG)
	OutputDebugString(L"CMainWindow::OnUpdateInfos");
	OutputDebugString(L"\n");
#endif

	auto pictureInfos = static_cast<CPictureInfosMessage*>(event.GetClientData());
	if (pictureInfos != nullptr)
	{
		wxString filename = pictureInfos->filename;
		if (filename[0] != '\0')
		{
			statusBarViewer->SetText(1, filename);
		}

		statusBarViewer->SetText(0, pictureInfos->infos);

		wxString label = CLibResource::LoadStringFromResource(L"LBLUPDATEINFOS", 1);
		wxString infos = label + CFileUtility::GetFileName(filename);
		statusBarViewer->SetWindowTitle(infos);

		delete pictureInfos;
	}
}

bool CMainWindow::GetProcessEnd()
{
	endApplication = true;

	if ( nbProcess > 0 || isCheckingFile)
		return false;

	return true;
}


void CMainWindow::OnExit(wxCommandEvent& event)
{
	statusBarViewer->Exit();
}

void CMainWindow::OnOpenFileOrFolder(wxCommandEvent& event)
{
	auto file = static_cast<wxString*>(event.GetClientData());
	if (file != nullptr)
	{
		bool find = false;
		wxFileName filename(*file);
		wxString folder = filename.GetPath();

		//Test if folder is on database
		CSqlFolderCatalog sqlFolderCatalog;
		int64_t idFolder = sqlFolderCatalog.GetFolderCatalogId(NUMCATALOGID, folder);

		cout << "Folder : " << folder << " " << idFolder << endl;

		if (idFolder == -1)
		{

			init = true;
			int type = event.GetInt();
			if (type == 1)
				OpenFile(*file);
			else
				OpenFolder(*file);
		}


		delete file;
	}
}



wxString CMainWindow::AddFolder(const wxString& folder, wxString* file)
{
	auto windowMain = static_cast<CWindowMain*>(this->FindWindowById(MAINVIEWERWINDOWID));
	wxString localFilename = "";
	wxString msg = "In progress ...";
	CSqlFolderCatalog sqlFolderCatalog;

	wxArrayString files;
	wxDir::GetAllFiles(folder, &files, wxEmptyString, wxDIR_FILES);
	if (files.size() > 0)
		sort(files.begin(), files.end());


	int64_t idFolder = sqlFolderCatalog.GetOrInsertFolderCatalog(NUMCATALOGID, folder);
	//Insert la liste des photos dans la base de données.
	CSqlInsertFile sqlInsertFile;
	sqlInsertFile.AddFileFromFolder(this, nullptr, files, folder, idFolder, localFilename);

	if (file != nullptr)
		localFilename = *file;

	return localFilename;
}


void CMainWindow::RemoveFolder(const wxString& folder)
{

	wxString libelle = CLibResource::LoadStringFromResource(L"LBLBUSYINFO", 1);
	//wxBusyInfo wait(libelle, windowMain);
	if (!folder.IsEmpty())
	{
		wxString title = CLibResource::LoadStringFromResource(L"LBLSTOPALLPROCESS", 1);
		wxString message = CLibResource::LoadStringFromResource(L"LBLSTOPPROCESS", 1);
		StopAllProcess(title, message, this);

		//Indication d'imporation des critères 
		CSqlFolderCatalog sqlFolderCatalog;
		int64_t idFolder = sqlFolderCatalog.GetFolderCatalogId(NUMCATALOGID, folder);
		if (idFolder != -1)
		{
			CSQLRemoveData sqlRemoveData;
			sqlRemoveData.DeleteFolder(idFolder);
		}

		SetStopProcess(false);

	}
}

void CMainWindow::OpenFile(const wxString& fileToOpen)
{
	if (isCheckingFile)
	{
		changeFolder = true;
		std::this_thread::sleep_for(50ms);
	}

	bool find = false;
	wxFileName filename(fileToOpen);
	wxString folder = filename.GetPath();
	FolderCatalogVector folderList;
    //Test if folder is on database
    CSqlFolderCatalog sqlFolderCatalog;
    int64_t idFolder = sqlFolderCatalog.GetFolderCatalogId(NUMCATALOGID, folder);
	
    
    cout << "Folder : " << folder << " " << idFolder << endl;
    
    if (idFolder == -1)
    {
		CSqlFindFolderCatalog folderCatalog;
		folderCatalog.GetFolderCatalog(&folderList, NUMCATALOGID);
		for (CFolderCatalog folderlocal : folderList)
		{
			RemoveFSEntry(folderlocal.GetFolderPath());
		}

        CSQLRemoveData::DeleteCatalog(1);
        AddFolder(folder, nullptr);
		AddFSEntry(folder);
    }
	firstFileToShow = fileToOpen;
	UpdateFolderStatic();
	processIdle = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///Gestion des événements du menu
////////////////////////////////////////////////////////////////////////////////////////////////
bool CMainWindow::OpenFolder(const wxString& path)
{
	if (isCheckingFile)
	{
		changeFolder = true;
		std::this_thread::sleep_for(50ms);
	}


	if (wxDirExists(path))
	{

		bool find = false;
		FolderCatalogVector folderList;
        

		wxString folder = path;
        //Test if folder is on database
        CSqlFolderCatalog sqlFolderCatalog;
        int64_t idFolder = sqlFolderCatalog.GetFolderCatalogId(NUMCATALOGID, folder);


        
        cout << "Folder : " << folder << " " << idFolder << endl;
        
        if (idFolder == -1)
        {
			CSqlFindFolderCatalog folderCatalog;
			folderCatalog.GetFolderCatalog(&folderList, NUMCATALOGID);
			for (CFolderCatalog folderlocal : folderList)
			{
				RemoveFSEntry(folderlocal.GetFolderPath());
			}

            CSQLRemoveData::DeleteCatalog(1);
            firstFileToShow = AddFolder(path, nullptr);
			AddFSEntry(path);
        }
		


		UpdateFolderStatic();
		processIdle = true;

	}

	return true;
}

bool CMainWindow::IsFullscreen()
{
	return fullscreen;
}

void CMainWindow::InitPictures(wxCommandEvent& event)
{
	printf("InitPictures \n");
	refreshFolder = true;
	processIdle = true;
}


void CMainWindow::OnRefreshPicture(wxCommandEvent& event)
{
	localFilename = centralWnd->GetFilename();
	centralWnd->LoadPicture(localFilename, true);
}


void CMainWindow::ShowToolbar()
{
	showToolbar = !showToolbar;
	if (centralWnd != nullptr)
	{
		if (!showToolbar)
			centralWnd->HideToolbar();
		else
			centralWnd->ShowToolbar();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
///Affichage en mode plein écran
////////////////////////////////////////////////////////////////////////////////////////////////
bool CMainWindow::SetFullscreen()
{
	const bool work = centralWnd->IsCompatibleFullscreen();
	if (work)
		statusBarViewer->SetFullscreen();
	return work;
}

void CMainWindow::SetScreenEvent(wxCommandEvent& event)
{
	this->Resize();
}

bool CMainWindow::SetFullscreenMode()
{
	bool is_work = false;

	if (!fullscreen)
	{
		if (centralWnd->FullscreenMode())
		{
			is_work = true;
			fullscreen = true;
			statusBar->Show(false);
			wxCommandEvent event(wxEVENT_SETSCREEN);
			wxPostEvent(this, event);
		}
	}
	return is_work;
}

bool CMainWindow::SetScreen()
{
	bool isWork = false;

	if (fullscreen)
	{
		if (centralWnd->ScreenMode())
		{
			statusBarViewer->SetScreen();
			fullscreen = false;
			statusBar->Show(true);
			isWork = true;
			wxCommandEvent event(wxEVENT_SETSCREEN);
			wxPostEvent(this, event);
		}
	}
	return isWork;
}