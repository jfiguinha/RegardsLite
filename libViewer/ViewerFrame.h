#pragma once
#include <StatusBarInterface.h>
#include "wx/print.h"

#include <MyFrameIntro.h>
#include <MainInterface.h>
#include <FolderCatalog.h>

using namespace Regards::Introduction;

class CImageLoadingFormat;
class CScannerFrame;

namespace Regards::Viewer
{
	// constants:
	enum
	{
		ID_Hello = 1,
		ID_Folder = 2,
		ID_Configuration = 3,
		ID_OpenCL = 4,
		ID_SIZEICONLESS = 5,
		ID_SIZEICONMORE = 6,
		ID_ERASEDATABASE = 7,
		ID_THUMBNAILRIGHT = 8,
		ID_THUMBNAILBOTTOM = 9,
		ID_FACEDETECTION = 10,
		ID_INTERPOLATIONFILTER = 11,
		ID_EXPORT = 12,
		ID_CATEGORYDETECTION = 14,
		ID_ASSOCIATE = 15,
		ID_ExternalProgram = 16,
		ID_OPENFILE = 17,
		ID_OPENFOLDER = 18,
        ID_DIAPORAMA = 21,
        ID_EDIT = 22,

		ID_VIDEO = 1018,
		ID_AUDIO = 1019,
		ID_SUBTITLE = 1020,
		WXSCAN_PAGE,
		WXPRINT_PAGE_SETUP,
		WXPRINT_PAGE_SETUP_PS,
#ifdef __WXMAC__
			WXPRINT_PAGE_MARGINS,
#endif
	};

	class CMainWindow;
	class CWaitingWindow;
	class CMainParam;
	class CMainTheme;

	class CViewerFrame : public wxFrame, public IStatusBarInterface
	{
	public:
		CViewerFrame(const wxString& title, const wxPoint& pos, const wxSize& size, IMainInterface* mainInterface,
		             const wxString& openfile = "");
		~CViewerFrame() override;
		void SetText(const int& numPos, const wxString& libelle) override;
		void SetRangeProgressBar(const int& range) override;
		void SetPosProgressBar(const int& position) override;
		void SetWindowTitle(const wxString& libelle) override;
		void SetFullscreen() override;
		void SetScreen() override;
		void PrintPreview(CImageLoadingFormat* imageToPrint) override;
		void PrintImagePreview(CImageLoadingFormat* imageToPrint) override;
		void Exit() override;

		void ShowViewer() override
		{
		};
		static bool GetViewerMode();
		static void SetViewerMode(const bool& viewerMode);

		int ShowScanner() override;

	private:
		void OnExport(wxCommandEvent& event);
#ifdef WIN32
		void OnAssociate(wxCommandEvent& event);
#endif
		void OnPrint(wxCommandEvent& event);
		void OnHelp(wxCommandEvent& event);
		void OnIconSizeLess(wxCommandEvent& event);
		void OnIconSizeMore(wxCommandEvent& event);

		void OnPictureEndLoading(wxCommandEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		void OnKeyUp(wxKeyEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnHello(wxCommandEvent& event);

		void OnShowInfos(wxCommandEvent& event);
		void OnShowThumbnail(wxCommandEvent& event);
		void OnShowToolbar(wxCommandEvent& event);
		void OnShowThumbnailVideo(wxCommandEvent& event);
		void CheckAllProcessEnd(wxTimerEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnConfiguration(wxCommandEvent& event);
		void OnEraseDatabase(wxCommandEvent& event);

		void OnUpdateWindowShow(wxCommandEvent& event);
        void OnEdit(wxCommandEvent& event);
        void OnExportDiaporama(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);
		void OnPageSetup(wxCommandEvent& event);

		void OnOpenFile(wxCommandEvent& event);
		void OnOpenFolder(wxCommandEvent& event);


		void UpdateMenuCheck();
#ifdef __WXMAC__
			void OnPageMargins(wxCommandEvent& event);
#endif

		void OnTimerLoadPicture(wxTimerEvent& event);
		void OnTimerEndLoadPicture(wxTimerEvent& event);
		bool CheckDatabase(FolderCatalogVector& folderList);

        void OnWindowFullScreen(wxCommandEvent & event);
        void OpenPictureFile();


		wxDECLARE_EVENT_TABLE();


		bool repeatEvent = false;
		int eventToLoop = 0;
		
		IMainInterface* mainInterface;
		CMainParam* viewerParam;
		CMainTheme* viewerTheme;
		CMainWindow* mainWindow;
		wxTimer* exitTimer;
		bool pictureEndLoading = true;
		CWaitingWindow* mainWindowWaiting;
		wxPreviewFrameModalityKind m_previewModality;
		bool fullscreen;
		const wxString& title_;
		const wxPoint& pos_;
		const wxSize& size_;
		IMainInterface* main_interface_;
		const wxString& file_to_open_;
		static bool viewerMode;
		bool onExit;
		bool m_fsWatcher = true;
		wxString fileToOpen ="";
        
        int oldWidth = 0;
        int oldHeight = 0;
		
		wxTimer* endLoadPictureTimer;
		wxTimer* loadPictureTimer;
		
		wxString filenameTimer;
		int nbTime;

		wxMenuItem* toolbarItem;
		wxMenuItem* thumbnailItem;
		wxMenuItem* thumbnailVideoItem;
		wxMenuItem* infosItem;
		
	};
}
