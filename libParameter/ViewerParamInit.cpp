#include <header.h>
#include "ViewerParamInit.h"
#include "ViewerParam.h"
#include <wx/stdpaths.h>
using namespace Regards::Viewer;

CMainParam* CMainParamInit::_singleton = nullptr;

CMainParamInit::CMainParamInit()
{
}


CMainParamInit::~CMainParamInit()
{
}


CMainParam* CMainParamInit::getInstance()
{
	return _singleton;
}

void CMainParamInit::Initialize(CMainParam* param)
{
	if (nullptr == _singleton)
	{
		wxStandardPathsBase& stdp = wxStandardPaths::Get();
		wxString documentPath = stdp.GetDocumentsDir();


#ifdef WIN32

		documentPath.append("\\RegardsLite\\Regards.viewer.config");
		_singleton = param;
		_singleton->OpenFile(documentPath);
#else


        documentPath.append("/RegardsLite/Regards.viewer.config");
        _singleton = param;
        _singleton->OpenFile(documentPath);
#endif
	}
}
