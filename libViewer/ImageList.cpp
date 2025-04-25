#include <header.h>
#include "ImageList.h"
using namespace Regards::Viewer;

int CImageList::GetNbElement()
{
    std::lock_guard<std::mutex> lock(muList);
    return photolist.size();
}

CImageList::~CImageList()
{
    std::lock_guard<std::mutex> lock(muList);
    photolist.clear();
}

PhotosVector* CImageList::GetPointer()
{
    return &photolist;
}

PhotosVector CImageList::GetCopy()
{
    std::lock_guard<std::mutex> lock(muList);
    return photolist; // Copie implicite du vecteur
}

void CImageList::Lock()
{
    muList.lock();
}

void CImageList::Unlock()
{
    muList.unlock();
}

void CImageList::SetImageList(const PhotosVector& pictures)
{
    std::lock_guard<std::mutex> lock(muList);
    photolist = pictures; // Copie directe du vecteur
}

CPhotos CImageList::GetElement(const int& numElement, bool& isValid)
{
    std::lock_guard<std::mutex> lock(muList);
    if (numElement >= 0 && numElement < photolist.size())
    {
        isValid = true;
        return photolist[numElement];
    }
    isValid = false;
    return CPhotos(); // Retourne un objet par défaut
}

wxString CImageList::GetFilePath(const int& numElement, bool& isValid)
{
    std::lock_guard<std::mutex> lock(muList);
    if (numElement >= 0 && numElement < photolist.size())
    {
        wxString path = photolist[numElement].GetPath();
        isValid = wxFileExists(path);
        return path;
    }
    isValid = false;
    return ""; // Retourne une chaîne vide
}

int CImageList::FindFileIndex(const wxString& filename)
{
	int numElement = 0;
	muList.lock();

	for (CPhotos file : photolist)
	{
		if (file.GetPath() == filename)
		{
			break;
		}
		numElement++;
	}
	muList.unlock();
	return numElement;
}

int CImageList::FindFileIndex(const int& id)
{
	int numElement = 0;
	muList.lock();

	for (CPhotos file : photolist)
	{
		if (file.GetId() == id)
		{
			break;
		}
		numElement++;
	}
	muList.unlock();
	return numElement;
}
