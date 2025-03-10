#include <header.h>
#include "ViewerThemeInit.h"
#include "ViewerTheme.h"
#include <FileUtility.h>
#include <wx/stdpaths.h>
#include <RegardsConfigParam.h>
#include <ParamInit.h>
#include "ViewerParamInit.h"
#include "ViewerParam.h"
using namespace Regards::Viewer;

CMainTheme* CMainThemeInit::_singleton = nullptr;
wxString CMainThemeInit::documentPath = "";

CMainThemeInit::CMainThemeInit()
{
}


CMainThemeInit::~CMainThemeInit()
{
}


CMainTheme* CMainThemeInit::getInstance()
{
	if (nullptr == _singleton)
	{
		auto viewerTheme = new CMainTheme();
		Initialize(viewerTheme);
		return viewerTheme;
	}
	return _singleton;
}

void CMainThemeInit::Initialize(CMainTheme* param)
{
	if (nullptr == _singleton)
	{
		/*
		wxString filename = wxStandardPaths::Get().GetExecutablePath();
		filename.append(L".viewer.theme");
		_singleton = param;
		_singleton->OpenFile(filename);
		*/
        int skinMode = 0;
        CRegardsConfigParam* regardsParam = CParamInit::getInstance();
        if (regardsParam != nullptr)
            skinMode = regardsParam->GetSkinWindowMode();
        
         wxSystemAppearance systemApp = wxSystemSettings::GetAppearance();
		wxStandardPathsBase& stdp = wxStandardPaths::Get();
		 documentPath = stdp.GetDocumentsDir();
         bool isDarkTheme =  systemApp.IsDark();
         wxString resourceTheme = CFileUtility::GetResourcesFolderPath();
         
#ifdef WIN32

         if (skinMode == 0)
         {
             if (isDarkTheme)
                 documentPath.append("\\RegardsLite\\Regards.dark.theme");
             else
                 documentPath.append("\\RegardsLite\\Regards.light.theme");

             if (isDarkTheme)
                 resourceTheme.append("\\theme\\Regards.viewer.dark.theme");
             else
                 resourceTheme.append("\\theme\\Regards.viewer.light.theme");
         }
         else if (skinMode == 1)
         {
             documentPath.append("\\RegardsLite\\Regards.light.theme");
             resourceTheme.append("\\theme\\Regards.viewer.light.theme");
         }
         else
         {
             documentPath.append("\\RegardsLite\\Regards.dark.theme");
             resourceTheme.append("\\theme\\Regards.viewer.dark.theme");
         }

#else
         if (skinMode == 0)
         {

             if (isDarkTheme)
                 documentPath.append("/RegardsLite/Regards.dark.theme");
             else
                 documentPath.append("/RegardsLite/Regards.light.theme");

             if (isDarkTheme)
                 resourceTheme.append("/theme/Regards.viewer.dark.theme");
             else
                 resourceTheme.append("/theme/Regards.viewer.light.theme");
         }
         else if (skinMode == 1)
         {
             documentPath.append("/RegardsLite/Regards.light.theme");
             resourceTheme.append("/theme/Regards.viewer.light.theme");
         }
         else
         {
             documentPath.append("/RegardsLite/Regards.dark.theme");
             resourceTheme.append("/theme/Regards.viewer.dark.theme");
         }
            
#endif
        if(!wxFileExists(documentPath))
            wxCopyFile(resourceTheme, documentPath);

		_singleton = param;
        _singleton->OpenFile(documentPath);
	}
}

void CMainThemeInit::SaveTheme()
{
	if (nullptr != _singleton)
	{
        if (!wxFileExists(documentPath))
		    _singleton->SaveFile();
	}
}
