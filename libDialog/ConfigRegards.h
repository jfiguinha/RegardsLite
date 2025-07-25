#ifndef CONFIGREGARDS_H
#define CONFIGREGARDS_H

#ifndef WX_PRECOMP
//(*HeadersPCH(ConfigRegards)
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#endif
//(*Headers(ConfigRegards)
#include <wx/spinctrl.h>
//*)

class ConfigRegards : public wxDialog
{
public:
	ConfigRegards(wxWindow* parent);
	~ConfigRegards() override;

	//(*Declarations(ConfigRegards)
	wxButton* btOk;
	wxStaticBox* sbDiaporama;

	wxTextCtrl* txtMusicDiaporamaPath;
	wxButton* btMusicDiaporamaPath;

	wxSpinCtrl* scTime;
	wxSpinCtrl* scProcessThumbnail;
	wxRadioBox* rdPreviewRender;
	wxRadioBox* rdDxva2Render;

	wxComboBox* rbTransitionEffect;
	wxRadioBox* rbRenderVideo;
	wxRadioBox* rbEffectLibrary;
	wxButton* btCancel;
	wxStaticText* stTime;
	wxStaticBox* sbThumbnail;
	wxRadioBox* rbThumbnailQuality;
	wxRadioBox* rbThumbnailCache;
	wxRadioBox* rbDatabaseInMemory;
	wxRadioBox* rbContrastCorrection;

	wxComboBox* rbTransitionDiaporamaEffect;
	wxComboBox* rbInterpolation;
	wxComboBox* cbHardwareAccelerator;

	wxRadioBox* rbOpenCLOpenGLInterop;
	wxComboBox* rbVideoEncoderHard;
	wxComboBox* rbVideoDecoderHard;
	wxRadioBox* rbInverseVideoRot;
	//*)

	wxComboBox* rbSkin;

	wxTextCtrl* txtPicturePath;
	wxButton* btPicturePath;
	wxTextCtrl* txtVideoPath;
	wxButton* btVideoPath;
	bool IsOk();

protected:
	//(*Identifiers(ConfigRegards)
	//*)

private:
	void OnBtnMusicDiaporamaClick(wxCommandEvent& event);
	void OnbtnPathVideoClick(wxCommandEvent& event);
	void OnBtnPathPictureClick(wxCommandEvent& event);


	//(*Handlers(ConfigRegards)
	void init();
	void OnbtnOkClick(wxCommandEvent& event);
	void OnBtnCancelClick(wxCommandEvent& event);
	//*)
	bool isOk;
	wxString oldAccelerator;
	DECLARE_EVENT_TABLE()
};

#endif
