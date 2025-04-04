#include <header.h>
#include "SqlInsertFile.h"
#include <libPicture.h>
#include <wx/dir.h>
#include "SqlResult.h"
#include <algorithm>
#include <mutex>
using namespace Regards::Picture;
using namespace Regards::Sqlite;


CSqlInsertFile::CSqlInsertFile()
	: CSqlExecuteRequest(L"RegardsDB"), m_photosVector(nullptr), listPathFile(nullptr), listPhoto(nullptr)
{
	numPhoto = 0;
	type = 0;
}


CSqlInsertFile::~CSqlInsertFile()
{
}

void CSqlInsertFile::UpdatePhotoProcess(const int& numPhoto)
{
	ExecuteRequestWithNoResult("Update PHOTOS Set Process = 1 where NumPhoto = " + to_string(numPhoto));
}

CPhotos CSqlInsertFile::GetPhotoToProcess()
{
	type = 4;
	this->numPhoto = 0;
	photoLocal.SetId(-1);
	ExecuteRequest("SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS where CriteriaInsert = 0 and Process = 0 LIMIT 1");
	return photoLocal;
}

void CSqlInsertFile::GetPhotoToProcessList(PhotosVector* photosVector)
{
 	type = 0;
    m_photosVector = photosVector;
	ExecuteRequest("SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS where CriteriaInsert = 0 and Process = 0"); 
}

CPhotos CSqlInsertFile::GetPhoto(const int& numPhoto)
{
	type = 4;
	this->numPhoto = numPhoto;
	photoLocal.SetId(-1);
	ExecuteRequest("SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS where CriteriaInsert = 0");
	return photoLocal;
}

int CSqlInsertFile::GetNbPhotosToProcess()
{
	type = 3;
	numPhoto = 0;
	ExecuteRequest("SELECT count(*) as nbphoto FROM PHOTOS where CriteriaInsert = 0 and Process = 0");
	return numPhoto;
}

int CSqlInsertFile::ReinitPhotosToProcess()
{
	numPhoto = 0;
	ExecuteRequest("Update PHOTOS SET CriteriaInsert = 0, Process = 0;");
	return 0;
}


int CSqlInsertFile::GetNbPhotos()
{
	type = 3;
	numPhoto = 0;
	ExecuteRequest("SELECT count(*) as nbphoto FROM PHOTOS where CriteriaInsert = 0");
	return numPhoto;
}

void CSqlInsertFile::ImportFileFromFolder(const vector<wxString>& listFile, const int& idFolder)
{
	BeginTransaction();
	CLibPicture libPicture;

	for (wxString filename : listFile)
	{
		if (GetNumPhoto(filename) == 0)
		{
			bool isValid = true;
            int multifile = 0;
			int extensionId = libPicture.TestImageFormat(filename);
			filename.Replace("'", "''");

            /*
			if (libPicture.TestIsVideo(filename) || libPicture.TestIsPDF(filename) || libPicture.
				TestIsAnimation(filename))
			{
				multifile = 1;
			}

			if (libPicture.TestIsVideo(filename))
			{
				isValid = libPicture.TestIsVideoValid(filename);
			}
            */
			if (extensionId > 0)
			{
				ExecuteRequestWithNoResult(
					"INSERT INTO PHOTOS (NumFolderCatalog, FullPath, CriteriaInsert, Process, ExtensionId, Multifiles) VALUES ("
					+ to_string(idFolder) + ", '" + filename + "', 0, 0, " + to_string(extensionId) + "," +
					to_string(multifile) + ")");
			}
		}
	}
	//ExecuteRequestWithNoResult("INSERT INTO PHOTOSSEARCHCRITERIA (NumPhoto,FullPath) SELECT NumPhoto, FullPath FROM PHOTOS WHERE NumFolderCatalog = " + to_string(idFolder) + " and NumPhoto not in (SELECT NumPhoto FROM PHOTOSSEARCHCRITERIA)");

	CommitTransection();
}


void CSqlInsertFile::InsertPhotoFolderToRefresh(const wxString& folder)
{
	CLibPicture libPicture;
	BeginTransaction();

	ExecuteRequestWithNoResult("DELETE FROM PHOTOFOLDER");

	wxArrayString files;

	wxDir::GetAllFiles(folder, &files, wxEmptyString, wxDIR_FILES);
	if (files.size() > 0)
		sort(files.begin(), files.end());

	for (wxString file : files)
	{
		if (libPicture.TestImageFormat(file) != 0)
		{
			file.Replace("'", "''");
			ExecuteRequestWithNoResult("INSERT INTO PHOTOFOLDER (FullPath) VALUES ('" + file + "')");
		}
	}
	CommitTransection();
}

bool CSqlInsertFile::GetPhotoToAdd(vector<wxString>* listFile)
{
	type = 1;
	listPathFile = listFile;
	return (ExecuteRequest("SELECT FullPath FROM PHOTOFOLDER WHERE FullPath not in (Select FullPath From PHOTOS)") != -
		       1)
		       ? true
		       : false;
}

int CSqlInsertFile::GetNumPhoto(const wxString& filepath)
{
	numPhoto = 0;
	type = 3;
	wxString filename = filepath;
	filename.Replace("'", "''");
	ExecuteRequest("SELECT NumPhoto FROM PHOTOS WHERE FullPath = '" + filename + "'");
	return numPhoto;
}


bool CSqlInsertFile::GetPhotoToRemove(vector<int>* listFile, const int& idFolder)
{
	type = 2;
	listPhoto = listFile;
	return (ExecuteRequest(
		       "SELECT NumPhoto FROM PHOTOS WHERE NumFolderCatalog = " + to_string(idFolder) +
		       " and FullPath not in(Select FullPath From PHOTOFOLDER)") != -1)
		       ? true
		       : false;
}

int CSqlInsertFile::AddFileFromFolder(wxWindow* parent, wxProgressDialog* dialog, wxArrayString& files,
                                      const wxString& folder, const int& idFolder, wxString& firstFile)
{
	if (files.size() > 0)
	{
		BeginTransaction();
		auto s_mutex = new std::mutex();

		tbb::parallel_for(0, static_cast<int>(files.size()), 1, [=](int i)
		{
			wxString file = files[i];
			CLibPicture libPicture;
            int extensionId = libPicture.TestImageFormat(file);
			if (extensionId != 0 && GetNumPhoto(file) == 0)
			{
				ExecuteRequestWithNoResult(
					"INSERT INTO PHOTOS (NumFolderCatalog, FullPath, CriteriaInsert, Process, ExtensionId) VALUES (" +
						to_string(idFolder) + ",'" + file + "', 0, 0, " + to_string(extensionId) + ")");
			}

			if (dialog != nullptr)
			{
				s_mutex->lock();
				wxString message = "In progress : " + to_string(i) + "/" + to_string(files.Count());
				dialog->Update(i, message);
				s_mutex->unlock();
			}
		});

		CommitTransection();

		delete s_mutex;

		CLibPicture libPicture;
		bool first = true;
		for (size_t i = 0; i < files.size(); i++)
		{
			wxString file = files[i];
			if (libPicture.TestImageFormat(file) != 0)
			{
				if (first)
				{
					firstFile = file;
					break;
				}
			}
		}
	}
	return files.size();
}


int CSqlInsertFile::ImportFileFromFolder(const wxString& folder, const int& idFolder, wxString& firstFile)
{
	CLibPicture libPicture;
	BeginTransaction();

	int i = 0;
	wxArrayString files;

	wxDir::GetAllFiles(folder, &files, wxEmptyString, wxDIR_FILES);

	if (files.size() > 0)
		sort(files.begin(), files.end());

	//
	for (wxString file : files)
	{
		if (libPicture.TestImageFormat(file) != 0 && GetNumPhoto(file) == 0)
		{
			bool isValid = true;
			int extensionId = libPicture.TestImageFormat(file);
			if (i == 0)
				firstFile = file;
			file.Replace("'", "''");
            /*
			if (libPicture.TestIsVideo(file))
			{
				isValid = libPicture.TestIsVideoValid(file);
			}
            */
			if (extensionId > 0)
			{
				ExecuteRequestWithNoResult(
					"INSERT INTO PHOTOS (NumFolderCatalog, FullPath, CriteriaInsert, Process, ExtensionId) VALUES (" +
					to_string(idFolder) + ",'" + file + "', 0, 0, " + to_string(extensionId) + ")");
			}

			i++;
		}
	}
	CommitTransection();
	return i;
}

bool CSqlInsertFile::GetPhotos(PhotosVector* photosVector)
{
	m_photosVector = photosVector;
	return (ExecuteRequest(
			       "SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS WHERE CriteriaInsert = 0")
		       != -
		       1)
		       ? true
		       : false;
}

bool CSqlInsertFile::GetAllPhotos(PhotosVector* photosVector)
{
	m_photosVector = photosVector;
	return (ExecuteRequest("SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS") != -1)
		       ? true
		       : false;
}

bool CSqlInsertFile::GetPhotos(PhotosVector* photosVector, const int64_t& numFolder)
{
	m_photosVector = photosVector;
	return (ExecuteRequest(
		       "SELECT NumPhoto, FullPath, NumFolderCatalog, CriteriaInsert FROM PHOTOS WHERE CriteriaInsert = 0 and NumFolderCatalog = "
		       + to_string(numFolder) + "") != -1)
		       ? true
		       : false;
}

int CSqlInsertFile::TraitementResult(CSqlResult* sqlResult)
{
	int nbResult = 0;
	while (sqlResult->Next())
	{
		switch (type)
		{
		case 4:
			{
				if (nbResult == numPhoto)
				{
					for (auto i = 0; i < sqlResult->GetColumnCount(); i++)
					{
						switch (i)
						{
						case 0:
							photoLocal.SetId(sqlResult->ColumnDataInt(i));
							break;
						case 1:
							photoLocal.SetPath(sqlResult->ColumnDataText(i));
							break;
						case 2:
							photoLocal.SetFolderId(sqlResult->ColumnDataInt(i));
							break;
						case 3:
							photoLocal.SetIsCriteriaInsert(sqlResult->ColumnDataInt(i));
							break;
						default: ;
						}
					}
					return nbResult;
				}
			}
			break;

		case 0:
			{
				CPhotos _cPhoto;
				for (auto i = 0; i < sqlResult->GetColumnCount(); i++)
				{
					switch (i)
					{
					case 0:
						_cPhoto.SetId(sqlResult->ColumnDataInt(i));
						break;
					case 1:
						_cPhoto.SetPath(sqlResult->ColumnDataText(i));
						break;
					case 2:
						_cPhoto.SetFolderId(sqlResult->ColumnDataInt(i));
						break;
					case 3:
						_cPhoto.SetIsCriteriaInsert(sqlResult->ColumnDataInt(i));
						break;
					default: ;
					}
				}
				m_photosVector->push_back(_cPhoto);
			}
			break;

		case 1:
			{
				wxString path;
				for (auto i = 0; i < sqlResult->GetColumnCount(); i++)
				{
					switch (i)
					{
					case 0:
						path = sqlResult->ColumnDataText(i);
						break;
					default: ;
					}
				}
				listPathFile->push_back(path);
			}
			break;

		case 2:
			{
				int id;
				for (auto i = 0; i < sqlResult->GetColumnCount(); i++)
				{
					switch (i)
					{
					case 0:
						id = sqlResult->ColumnDataInt(i);
						break;
					default: ;
					}
				}
				listPhoto->push_back(id);
			}
			break;

		case 3:
			{
				for (auto i = 0; i < sqlResult->GetColumnCount(); i++)
				{
					switch (i)
					{
					case 0:
						numPhoto = sqlResult->ColumnDataInt(i);
						break;
					default: ;
					}
				}
			}
			break;
		default: ;
		}
		nbResult++;
	}
	return nbResult;
}
