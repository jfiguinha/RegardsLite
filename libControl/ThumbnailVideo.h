#pragma once
#include "ThumbnailHorizontal.h"

namespace Regards::Control
{
	class CThumbnailVideo : public CThumbnailHorizontal
	{
	public:
		CThumbnailVideo(wxWindow* parent, wxWindowID id, const CThemeThumbnail& themeThumbnail,
		                const bool& testValidity);
		~CThumbnailVideo(void) override;
		void SetFile(const wxString& videoFile, const int& size);
		void SetVideoPosition(const int64_t& videoPos);
		void EraseThumbnail(long value);
		void UpdateVideoThumbnail(const wxString& videoFile);
    
	protected:
		void UpdateVideoThumbnail();
		void ResizeThumbnail() override;
		static bool ItemCompFonct(int x, int y, CIcone * icone, CWindowMain* parent);
		void EraseThumbnail(wxCommandEvent& event) override;
		void UpdateVideoThumbnail(wxCommandEvent& event);
		

		int FindNumItem(const int& videoPos);
		virtual void InitWithDefaultPicture(const wxString& szFileName, const int& size);
		int numItemSelected;
		bool process_end;
		void UpdateThumbnailIcone(wxCommandEvent& event);
		static void LoadVideoThumbnail(void* param);
		void GenerateThumbnail(const wxString& szFileName);
		int64_t oldvideoPos = 0;
		int iFormat = 0;
		int nbProcess = 0;
	};
}
