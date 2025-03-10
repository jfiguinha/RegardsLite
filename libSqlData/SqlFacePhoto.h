#pragma once
#include "SqlExecuteRequest.h"

class CImageLoadingFormat;

namespace Regards
{
	namespace Sqlite
	{
		class CSqlResult;

		class CFaceRecognitionData
		{
		public:
			int numFace;
			int numFaceCompatible;
		};

		class CSqlFacePhoto : public CSqlExecuteRequest
		{
		public:
			CSqlFacePhoto();
			~CSqlFacePhoto() override;
			int InsertFaceTreatment(const wxString& path);
			int InsertFace(const wxString& path, const wxString& gender, const wxString& age, const int& numberface, const int& width, const int& height,
			               const double& pertinence, const uint8_t* zBlob, const int& nBlob);
			int GetNumFace(const wxString& path, const int& numberface);
			int UpdateVideoFace(const int& numFace, const int& videoPosition);
			int GetVideoFacePosition(const int& numFace);
			vector<wxString> GetPhotoList();
			vector<wxString> GetPhotoListTreatment();

			CImageLoadingFormat* GetFacePicture(const int& numFace);
			void DeleteNumFace(const int& numFace);
			void EraseFace(const int& numFace);
			int GetFaceCompatibleRecognition(const int& numFace);
			vector<CFaceRecognitionData> GetAllNumFaceRecognition();
			//vector<int> GetAllNumFaceRecognition(const int& numFace);
			bool DeleteNumFaceMaster(const int& idFaceMaster);
			vector<int> GetAllNumFace();
			vector<int> GetAllThumbnailFace();
			vector<int> GetAllNumFace(const int& numFace);
			cv::Mat GetFace(const int& numFace, bool &isDefault);
			bool DeleteFaceDatabase();
			bool DeletePhotoFaceDatabase(const wxString& path);
			bool DeleteListOfPhoto(const vector<wxString>& listPhoto);
			bool DeleteListOfPhoto(const vector<int>& listNumPhoto);
			bool DeleteFaceTreatmentDatabase();
			void RebuildLink();

		private:
			void DeleteFaceNameAlone();

			int TraitementResult(CSqlResult* sqlResult) override;
			int64_t numFace;
			int type;
			//int width;
			//int height;
			wxString filename;
			vector<wxString> listPhoto;
			vector<int> listFace;
			vector<CFaceRecognitionData> listFaceRecognition;
			//vector<CPictureData *> listFace;
			//CPictureData * facePicture;
			vector<int> listFaceIndex;
			//wxImage bitmap;
		};
	}
}
