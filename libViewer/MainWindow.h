#pragma once
#include <Photos.h>
#include <WindowMain.h>
#include <ToolbarInterface.h>
#include "IconeList.h"
#include <wx/fswatcher.h>
#include <FolderCatalog.h>
#include <InfosSeparationBar.h>
using namespace Regards::Window;

class CRegardsBitmap;
class CImageLoadingFormat;
class CPictureCategorieLoadData;
class CPictureCategorie;
class IStatusBarInterface;
class CFFmpegTranscoding;
class CompressionAudioVideoOption;
class CThreadPhotoLoading;

namespace Regards::Viewer
{
	class CMainWindow;
	class CImageList;
	class CExportDiaporama;
	class CToolbar;
	class CMainParam;
	class CCentralWindow;
	class CFolderProcess;




	class CMainWindow : public CWindowMain
	{
	public:
		CMainWindow(wxWindow* parent, wxWindowID id, IStatusBarInterface* statusbar, const bool& openFirstFile, const wxString& fileToOpen);
		~CMainWindow() override;
		void UpdateScreenRatio() override;

		bool SetFullscreen();
		bool SetFullscreenMode();
		bool SetScreen();
		void TransitionEnd();

		bool OpenFolder(const wxString& path);
		bool IsFullscreen();
		void ShowToolbar();
		bool IsVideo();

		void SetText(const int& numPos, const wxString& libelle);
		void SetRangeProgressBar(const int& range);
		void SetPosProgressBar(const int& position);
		void OpenFile(const wxString& fileToOpen);

		bool GetProcessEnd() override;
		void OnOpenFileOrFolder(wxCommandEvent& event);
		wxString GetFilename();

		void SaveParameter() override;

		bool GetInit()
		{
			return init;
		}

	private:

		void StartOpening();
		void UpdateFolderStatic();
		static void OpenExternalFile(void* param);
		static void NewVersionAvailable(void* param);
		static void CheckFile(void* param);

		void SetDataToStatusBar(void* thumbnailMessage, const wxString& message);

		bool FindNextValidFile();
		bool FindPreviousValidFile();

		void OnEndDecompressFile(wxCommandEvent& event);
		void OnEditFile(wxCommandEvent& event);

		void OnUpdateInfos(wxCommandEvent& event);
		void OnShowToolbar(wxCommandEvent& event);
		void OnStatusSetText(wxCommandEvent& event);
		void OnEndCheckFile(wxCommandEvent& event);
		void OnSetRangeProgressBar(wxCommandEvent& event);
		void OnSetValueProgressBar(wxCommandEvent& event);

		void OnRefreshPicture(wxCommandEvent& event);

		void OnTimereventFileSysTimer(wxTimerEvent& event);
		bool RemoveFSEntry(const wxString& dirPath);
		bool AddFSEntry(const wxString& dirPath);
		void OnFileSystemModified(wxFileSystemWatcherEvent& event);
		bool CheckDatabase(FolderCatalogVector& folderList);
		void OnVersionUpdate(wxCommandEvent& event);
		void UpdateStatusBarMessage(wxCommandEvent& event);

		void OnExit(wxCommandEvent& event);
		void InitPictures(wxCommandEvent& event);
		void PictureVideoClick(wxCommandEvent& event);


		void OnPrint(wxCommandEvent& event);
		void OnPictureClick(wxCommandEvent& event);

		void PrintPreview(wxCommandEvent& event);


		void OnExportFile(wxCommandEvent& event);
		void OnUpdateExifThumbnail(wxCommandEvent& event);

		void SetScreenEvent(wxCommandEvent& event);
		void OnExportDiaporama(wxCommandEvent& event);
		void RefreshFolderList(wxCommandEvent& event);

		void OnUpdateFolder(wxCommandEvent& event);
		void OnRefreshThumbnail(wxCommandEvent& event);
		void OnProcessThumbnail(wxCommandEvent& event);
		void OnEndOpenExternalFile(wxCommandEvent& event);
		void OnRemoveFileFromCheckIn(wxCommandEvent& event);
		void OnCheckInUpdateStatus(wxCommandEvent& event);

		void OnRemoveFSEntry(wxCommandEvent& event);
		void OnAddFSEntry(wxCommandEvent& event);

		void Resize() override;
		void ExportVideo(const wxString& filename);
		void ProcessIdle() override;
		void OnIdle(wxIdleEvent& evt) override;


		void PhotoProcess(CPhotos* photo);

		wxString AddFolder(const wxString& folder, wxString* file);
		void RemoveFolder(const wxString& folder);

		//------------------------------------------------------
		void UpdateMessage(wxCommandEvent& event);
		void ProcessThumbnail();
		static void LoadPicture(void* param);
		void ProcessThumbnail(wxString filename, int type, long longWindow);
		void UpdateThumbnailIcone(wxCommandEvent& event);
		int nbElementInIconeList = 0;
		int nbPhotoElement = 0;
		int nbElement = 0;
		int nbProcess = 0;
		bool stopToGetNbElement = false;
		int thumbnailPos = 0;
		//std::map<wxString, bool> listFile;

		wxString fileToOpen = "";
		bool isCheckNewVersion = false;
		wxString tempVideoFile = "";
		wxString tempAudioVideoFile = "";
		bool fullscreen;
		wxGauge* progressBar;
		wxStatusBar* statusBar;
		//CToolbar* toolbar;
		CCentralWindow* centralWnd;
		std::thread* versionUpdate = nullptr;
		std::thread* openExternalFileThread = nullptr;
		bool isCheckingFile = false;
		IStatusBarInterface* statusBarViewer;
		wxRect posWindow;

		wxString localFilename;

		std::atomic<int> endApplication;
		bool showToolbar;
		CMainParam* viewerParam;
		bool multithread;
		bool needToReload;
		int typeAffichage;
		//std::atomic<bool> updateCriteria;
		bool updateFolder = false;
		bool refreshFolder;
		std::atomic<int> numElementTraitement;
		bool start;
		bool criteriaSendMessage;
		bool checkVersion;
		bool changeFolder = false;

		bool isThumbnailProcess = false;
		CExportDiaporama* exportDiaporama = nullptr;
		wxString firstFileToShow = "";
		wxString oldRequest = "";
		bool init = true;
		vector<wxString> photoList;
		CFolderProcess* folderProcess = nullptr;
		std::map<wxString, bool> listFile;
		wxFileSystemWatcher* m_watcher;
		wxTimer* eventFileSysTimer = nullptr;
	};
}