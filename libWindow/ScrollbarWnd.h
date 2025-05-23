#pragma once
#include <WindowMain.h>
#include <WindowOpenGLMain.h>
#include "WindowManager.h"

namespace Regards::Window
{
	class CScrollbarHorizontalWnd;
	class CScrollbarVerticalWnd;


	class CControlSize
	{
	public:
		int controlWidth;
		int controlHeight;
		bool useScaleFactor = false;
	};

	class CScrollbarWnd : public CWindowMain
	{
	public:
		CScrollbarWnd(wxWindow* parent, CWindowMain* centralWindow, wxWindowID id,
		              const wxString& windowName = "ScrollBar");
		CScrollbarWnd(wxWindow* parent, CWindowOpenGLMain* centralWindow, wxWindowID id,
		              const wxString& windowName = "ScrollBar");
		~CScrollbarWnd() override;

		void UpdateScreenRatio() override;

		void SetPageSize(const int& pageSize);
		void SetLineSize(const int& lineSize);


		void HideVerticalScroll();
		void HideHorizontalScroll();
		void ShowVerticalScroll();
		void ShowHorizontalScroll();


		int GetShowingScrollV();
		int GetShowingScrollH();

		int GetBarWidth();
		int GetBarHeight();

		void Resize() override;

		int GetHeight() override;
		int GetWidth() override;

		int GetPosLargeur();
		int GetPosHauteur();

	private:
		void DefaultConstructor();

	protected:
		void SetPosition(const int& posX, const int& posY);
		void RefreshData(wxCommandEvent& event);
		void OnLeftPosition(wxCommandEvent& event);
		void OnControlSize(wxCommandEvent& event);
		void OnSetPosition(wxCommandEvent& event);

		void OnMoveLeft(wxCommandEvent& event);
		void OnMoveRight(wxCommandEvent& event);
		void OnMoveTop(wxCommandEvent& event);
		void OnMoveBottom(wxCommandEvent& event);

		void OnScrollMove(wxCommandEvent& event);

		CScrollbarHorizontalWnd* scrollHorizontal;
		CScrollbarVerticalWnd* scrollVertical;
		void OnTopPosition(wxCommandEvent& event);

		bool showV;
		bool showH;
		bool _showV = false;
		bool _showH = false;
		bool _useScaleFactor = false;
		//int posHauteur;
		//int posLargeur;
		int controlHeight;
		int controlWidth;

		int defaultPageSize;
		int defaultLineSize;

		wxTimer* loadingTimer;
		CWindowManager* windowManager;
		CWindowToAdd* centralWindow;
	};
}
