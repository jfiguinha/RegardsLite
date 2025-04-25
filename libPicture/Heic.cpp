// ReSharper disable All
#include <header.h>
#include "Heic.h"
#ifdef LIBHEIC
#include <cstdint>
#include <libheif/heif.h>

#include "imageinfo.hpp"


using namespace std;
using namespace Regards::Picture;



CHeic::CHeic()
{
}


CHeic::~CHeic()
{
}

static const char kMetadataTypeExif[] = "Exif";

void GetHandleMetadata(heif_image_handle* handle, uint8_t*& buffer, unsigned int& size)
{
	heif_item_id metadata_id;
	int count = heif_image_handle_get_list_of_metadata_block_IDs(handle, kMetadataTypeExif, &metadata_id, 1);

	if (count <= 0) {
		printf("No metadata found.\n");
		return;
	}

	for (int i = 0; i < count; i++) {
		size_t datasize = heif_image_handle_get_metadata_size(handle, metadata_id);

		if (datasize > 0) {
			heif_error error = heif_image_handle_get_metadata(handle, metadata_id, buffer);
			if (error.code == heif_error_Ok) {
				size = static_cast<unsigned int>(datasize);
				printf("size : %u local_data : %p \n", size, buffer);
				return;
			}
		}
	}

	printf("Failed to retrieve metadata.\n");
}
vector<cv::Mat> CHeic::GetAllPicture(const char* filename, int& delay)
{
	vector<cv::Mat> listPicture;
	heif_context* ctx = heif_context_alloc();
	if (!ctx) {
		std::cerr << "Failed to allocate HEIF context.\n";
		return listPicture;
	}

	heif_error err = heif_context_read_from_file(ctx, filename, nullptr);
	if (err.code != heif_error_Ok) {
		std::cerr << "Error reading HEIF file: " << err.message << "\n";
		heif_context_free(ctx);
		return listPicture;
	}

	int numImages = heif_context_get_number_of_top_level_images(ctx);
	if (numImages <= 0) {
		std::cerr << "No images found in the file.\n";
		heif_context_free(ctx);
		return listPicture;
	}

	std::vector<heif_item_id> IDs(numImages);
	heif_context_get_list_of_top_level_image_IDs(ctx, IDs.data(), numImages);

	for (const auto& id : IDs) {
		heif_image_handle* handle = nullptr;
		err = heif_context_get_image_handle(ctx, id, &handle);
		if (err.code != heif_error_Ok) {
			std::cerr << "Error getting image handle: " << err.message << "\n";
			continue;
		}

		heif_image* img = nullptr;
		err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);
		if (err.code != heif_error_Ok) {
			std::cerr << "Error decoding image: " << err.message << "\n";
			heif_image_handle_release(handle);
			continue;
		}

		int width = heif_image_handle_get_width(handle);
		int height = heif_image_handle_get_height(handle);
		int stride = 0;
		const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

		if (data) {
			cv::Mat picture(height, width, CV_8UC3);
			memcpy(picture.data, data, stride * height);
			cv::cvtColor(picture, picture, cv::COLOR_RGB2BGRA);
			listPicture.push_back(std::move(picture));
		}

		heif_image_release(img);
		heif_image_handle_release(handle);
	}

	heif_context_free(ctx);
	return listPicture;
}

void CHeic::SavePicture(const char* filenameOut, const int& format, cv::Mat& source, uint8_t*& data_exif, unsigned int& size,
	const int& compression, const bool& hasExif)
{
	if (source.empty()) {
		std::cerr << "Source image is empty.\n";
		return;
	}

	heif_context* ctx = heif_context_alloc();
	if (!ctx) {
		std::cerr << "Failed to allocate HEIF context.\n";
		return;
	}

	heif_encoder* encoder = nullptr;
	heif_error err;
	if (format == 26) {
		err = heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);
	}
	else if (format == 35) {
		err = heif_context_get_encoder_for_format(ctx, heif_compression_AV1, &encoder);
	}
	else {
		err = heif_context_get_encoder_for_format(ctx, heif_compression_undefined, &encoder);
	}

	if (err.code != heif_error_Ok || !encoder) {
		std::cerr << "Failed to get encoder: " << err.message << "\n";
		heif_context_free(ctx);
		return;
	}

	heif_encoder_set_lossy_quality(encoder, compression);

	heif_image* image = nullptr;
	err = heif_image_create(source.cols, source.rows, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, &image);
	if (err.code != heif_error_Ok || !image) {
		std::cerr << "Failed to create HEIF image: " << err.message << "\n";
		heif_encoder_release(encoder);
		heif_context_free(ctx);
		return;
	}

	heif_image_add_plane(image, heif_channel_interleaved, source.cols, source.rows, 32);

	cv::Mat converted;
	cv::cvtColor(source, converted, cv::COLOR_BGRA2RGBA);
	int stride;
	uint8_t* p = heif_image_get_plane(image, heif_channel_interleaved, &stride);
	memcpy(p, converted.data, converted.total() * converted.elemSize());

	err = heif_context_encode_image(ctx, image, encoder, nullptr, nullptr);
	if (err.code != heif_error_Ok) {
		std::cerr << "Failed to encode image: " << err.message << "\n";
	}

	if (hasExif) {
		heif_image_handle* image_handle = nullptr;
		heif_context_get_primary_image_handle(ctx, &image_handle);
		heif_context_add_exif_metadata(ctx, image_handle, data_exif, size);
		heif_image_handle_release(image_handle);
	}

	heif_context_write_to_file(ctx, filenameOut);

	heif_image_release(image);
	heif_encoder_release(encoder);
	heif_context_free(ctx);
}

cv::Mat CHeic::GetPicture(const char * filename, int& delay, const int& numPicture)
{
    cv::Mat picture;
    
    heif_context* ctx = heif_context_alloc();
    heif_context_read_from_file(ctx, filename, nullptr);
   
    int numImages = heif_context_get_number_of_top_level_images(ctx);
    std::vector<heif_item_id> IDs(numImages);
    heif_context_get_list_of_top_level_image_IDs(ctx, IDs.data(), numImages);

    if(numPicture < numImages){
        struct heif_image_handle* handle;
        struct heif_error err = heif_context_get_image_handle(ctx, IDs[numPicture], &handle);
        if (err.code) {
          std::cerr << err.message << "\n";
        }
        else
        {
            int width = heif_image_handle_get_width(handle);
            int height = heif_image_handle_get_height(handle);



            cv::Mat picture = cv::Mat(height, width, CV_8UC3);
            
            // decode the image and convert colorspace to RGB, saved as 24bit interleaved
            heif_image* img;
            heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);

            int stride = 0;
            const uint8_t * data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
            memcpy(picture.data, data, stride * height);

            heif_image_release(img);
            

            cv::cvtColor(picture, picture, cv::COLOR_RGB2BGRA);

            heif_image_handle_release(handle);
        }
 
    }
    
    heif_context_free(ctx);

	return picture;
}

int CHeic::GetNbFrame(const char * filename)
{
    int nbId = 0;
	try
	{
        heif_context* ctx = heif_context_alloc();
        heif_context_read_from_file(ctx, filename, nullptr);
        nbId = heif_context_get_number_of_top_level_images(ctx);
        heif_context_free(ctx);
    }
    catch(...)
    {
        
    }
    return nbId;
}

cv::Mat CHeic::GetPicture(const char * filename, int& orientation, const bool & isThumbnail)
{
       
    int rotation = 0;
	cv::Mat picture;
	try
	{
		heif_context* ctx = heif_context_alloc();
		heif_context_read_from_file(ctx, filename, nullptr);

		// get a handle to the primary image
		heif_image_handle* handle;
		heif_context_get_primary_image_handle(ctx, &handle);

		// decode the image and convert colorspace to RGB, saved as 24bit interleaved
		heif_image* img;
		heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);

		int width = heif_image_handle_get_width(handle);
		int height = heif_image_handle_get_height(handle);
        
		picture = cv::Mat(height, width, CV_8UC3);

		int stride = 0;
		const uint8_t * data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
		memcpy(picture.data, data, stride * height);

		heif_image_release(img);
		heif_image_handle_release(handle);
		heif_context_free(ctx);


		cv::cvtColor(picture, picture, cv::COLOR_RGB2BGRA);
        

	}
	catch (...) {
		//std::cerr << err.get_message() << "\n";
	}

    printf("Rotation HEIF : %d \n", orientation );

	return picture;
}

void CHeic::GetPictureDimension(const char * filename, int& width, int& height)
{
    auto info = imageinfo::parse<imageinfo::FilePathReader>(filename);
    if (info) {
        width = info.size().width;
        height = info.size().height;
    }
	else
	{
		width = 0;
		height = 0;
	}
}

cv::Mat CHeic::GetThumbnailPicture(const char * filename, int& orientation)
{
    struct heif_error err;
    cv::Mat picture;
    
    heif_context* ctx = heif_context_alloc();
    heif_context_read_from_file(ctx, filename, nullptr);

    // get a handle to the primary image
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

	try
	{

		int nThumbnails = heif_image_handle_get_number_of_thumbnails(handle);
		std::vector<heif_item_id> thumbnailIDs(nThumbnails);

		nThumbnails = heif_image_handle_get_list_of_thumbnail_IDs(handle, thumbnailIDs.data(), nThumbnails);

		for (int thumbnailIdx = 0; thumbnailIdx < nThumbnails; thumbnailIdx++) {
			heif_image_handle* thumbnail_handle;
			err = heif_image_handle_get_thumbnail(handle, thumbnailIDs[thumbnailIdx], &thumbnail_handle);
			if (err.code) {
				std::cerr << err.message << "\n";
				break;
			}

			int th_width = heif_image_handle_get_width(thumbnail_handle);
			int th_height = heif_image_handle_get_height(thumbnail_handle);

			printf("  thumbnail: %dx%d\n", th_width, th_height);


			picture = cv::Mat(th_height, th_width, CV_8UC3);

			// decode the image and convert colorspace to RGB, saved as 24bit interleaved
			heif_image* img;
			heif_decode_image(thumbnail_handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);

			int stride = 0;
			const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
			memcpy(picture.data, data, stride * th_height);

			heif_image_release(img);


			cv::cvtColor(picture, picture, cv::COLOR_RGB2BGRA);

			heif_image_handle_release(thumbnail_handle);


			break;
		}
	}
	catch (...)
	{

	}


	heif_image_handle_release(handle);
    
    heif_context_free(ctx);
    
    return picture;
}


#include <heifreader.h>
using namespace HEIF;
/* raw JPEG image data */
static const unsigned char image_jpg[] = {
	0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01,
	0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43,
	0x00, 0x14, 0x0e, 0x0f, 0x12, 0x0f, 0x0d, 0x14, 0x12, 0x10, 0x12, 0x17,
	0x15, 0x14, 0x18, 0x1e, 0x32, 0x21, 0x1e, 0x1c, 0x1c, 0x1e, 0x3d, 0x2c,
	0x2e, 0x24, 0x32, 0x49, 0x40, 0x4c, 0x4b, 0x47, 0x40, 0x46, 0x45, 0x50,
	0x5a, 0x73, 0x62, 0x50, 0x55, 0x6d, 0x56, 0x45, 0x46, 0x64, 0x88, 0x65,
	0x6d, 0x77, 0x7b, 0x81, 0x82, 0x81, 0x4e, 0x60, 0x8d, 0x97, 0x8c, 0x7d,
	0x96, 0x73, 0x7e, 0x81, 0x7c, 0xff, 0xc0, 0x00, 0x0b, 0x08, 0x00, 0x40,
	0x00, 0x40, 0x01, 0x01, 0x11, 0x00, 0xff, 0xc4, 0x00, 0x1b, 0x00, 0x00,
	0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x04, 0x03, 0x07, 0x02, 0x01, 0xff,
	0xc4, 0x00, 0x2f, 0x10, 0x00, 0x01, 0x03, 0x03, 0x02, 0x05, 0x03, 0x03,
	0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x11,
	0x00, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x81, 0x61, 0x71,
	0x91, 0x13, 0x32, 0xa1, 0x14, 0x22, 0xc1, 0x15, 0x23, 0x52, 0xd1, 0xf0,
	0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3f, 0x00, 0xb3, 0xa2,
	0x8a, 0x28, 0xa2, 0x8a, 0x28, 0xa2, 0x97, 0x64, 0x72, 0xd6, 0x58, 0xd4,
	0x8f, 0xd5, 0x3d, 0xca, 0xa5, 0x7d, 0xa8, 0x4e, 0xaa, 0x3e, 0xb0, 0x3a,
	0x7a, 0x9d, 0x2b, 0x2d, 0x87, 0x13, 0xe3, 0xaf, 0x9f, 0x0c, 0xb6, 0xb5,
	0xb6, 0xe2, 0xb4, 0x48, 0x71, 0x30, 0x14, 0x7b, 0x02, 0x09, 0x14, 0xee,
	0x94, 0x64, 0x38, 0x87, 0x1f, 0x8d, 0x5f, 0xd3, 0x7d, 0xe2, 0xa7, 0x46,
	0xe8, 0x6c, 0x49, 0x1e, 0xfd, 0x07, 0xb1, 0x33, 0x5f, 0xb8, 0xdc, 0xf5,
	0x96, 0x51, 0x7c, 0x96, 0xee, 0x14, 0xba, 0x04, 0xf2, 0x2c, 0x42, 0xa3,
	0xd3, 0x70, 0x7c, 0x1a, 0x6d, 0x45, 0x2c, 0xcd, 0xe5, 0x11, 0x89, 0xb1,
	0x5b, 0xc4, 0x02, 0xe1, 0xd1, 0xb4, 0xcf, 0xdc, 0xa3, 0xfc, 0x0d, 0xcd,
	0x41, 0xde, 0xe3, 0xb2, 0x2e, 0xda, 0x1c, 0xbd, 0xe2, 0x4a, 0x90, 0xe9,
	0x04, 0x95, 0x2b, 0xf7, 0x41, 0xd0, 0x18, 0xe8, 0x36, 0x03, 0xc6, 0x91,
	0x4a, 0x81, 0x20, 0x82, 0x24, 0x10, 0x66, 0x45, 0x5a, 0xdd, 0xf1, 0x32,
	0xff, 0x00, 0xa2, 0x5a, 0x8b, 0x62, 0x57, 0x7d, 0x70, 0x80, 0x93, 0x1a,
	0x94, 0x10, 0x79, 0x4a,
};

/* length of data in image_jpg */
static const unsigned int image_jpg_len = sizeof(image_jpg);

/* dimensions of image */
//static const unsigned int image_jpg_x = 64;
//static const unsigned int image_jpg_y = 64;

/* start of JPEG image data section */
static const unsigned int image_data_offset = 20;
#define image_data_len (image_jpg_len - image_data_offset)

void CHeicExif::GetMetadataHeic(const char * filename, uint8_t*& data, unsigned int& size)
{
	auto* reader = Reader::Create();

	if (reader->initialize(filename) != ErrorCode::OK)
	{
		cout << "Can't find input file: " << filename << ". "
			<< "Please download it from https://github.com/nokiatech/heif_conformance "
			<< "and place it in same directory with the executable." << endl;
		return;
	}

	FileInformation fileInfo{};
	reader->getFileInformation(fileInfo);

	// Find the primary item ID.
	ImageId primaryItemId;
	reader->getPrimaryItem(primaryItemId);

	// Find item(s) referencing to the primary item with "cdsc" (content describes) item reference.
	Array<ImageId> metadataIds;
	reader->getReferencedToItemListByType(primaryItemId, "cdsc", metadataIds);
	if (metadataIds.size > 0)
	{
		ImageId exifItemId = metadataIds[0];

		// Optional: verify the item ID we got is really of "Exif" type.
		FourCC itemType;
		reader->getItemType(exifItemId, itemType);
		if (itemType != "Exif")
		{
			return;
		}

		// Get item size from parsed information. For simplicity, assume it is the first and only non-image item in the
		// file.
		uint64_t itemSize = 1024 * 1024;
		for (ItemInformation itemInfo : fileInfo.rootMetaBoxInformation.itemInformations)
		{
			if (itemInfo.itemId != exifItemId)
			{
				continue;
			}
			itemSize = itemInfo.size;
		}
		// Request item data.
		if (size > 0)
		{
			/* raw EXIF header data */
			static const unsigned char exif_header[] = {
				0xff, 0xd8, 0xff, 0xe1
			};
			/* length of data in exif_header */
			static const unsigned int exif_header_len = sizeof(exif_header);

			auto memoryBuffer = new uint8_t[itemSize];
			reader->getItemData(metadataIds[0], memoryBuffer, itemSize);

			char value = 0;
			int pos = 0;
			memcpy(data, exif_header, exif_header_len);
			pos += exif_header_len;
			value = ((itemSize + 2) >> 8);
			memcpy(data + pos, &value, 1);
			pos++;
			value = (itemSize + 2) & 0xff;
			memcpy(data + pos, &value, 1);
			pos++;
			memcpy(data + pos, memoryBuffer + 4, itemSize - 4);
			pos += itemSize - 4;
			memcpy(data + pos, image_jpg + image_data_offset, image_data_len);





			delete[] memoryBuffer;
		}
		else
			size = itemSize + 512;
	}

	Reader::Destroy(reader);


}



uint32_t CHeic::GetDelay(const char * filename)
{
    int delay = 0;
	auto* reader = Reader::Create();
	Array<uint32_t> itemIds;

	// Input file available from https://github.com/nokiatech/heif_conformance
	if (reader->initialize(filename) == ErrorCode::OK)
	{
		FileInformation info;
		if (reader->getFileInformation(info) == ErrorCode::OK)
		{
			if (info.trackInformation.size > 0)
			{
				// Print information for every track read
				for (const auto& trackProperties : info.trackInformation)
				{
					const auto sequenceId = trackProperties.trackId;
					Array<TimestampIDPair> timestamps;
					reader->getItemTimestamps(sequenceId, timestamps);
					//cout << "Sample timestamps:" << endl;
					for (const auto& timestamp : timestamps)
					{
						delay = timestamp.timeStamp;
						break;
						//cout << " Timestamp=" << timestamp.timeStamp << "ms, sample ID=" << timestamp.itemId << endl;
					}
				}
			}
		}
	}
	else
	{
		cout << "Can't find input file: " << filename << ". "
			<< "Please download it from https://github.com/nokiatech/heif_conformance "
			<< "and place it in same directory with the executable." << endl;
	}

	Reader::Destroy(reader);


	return delay;
}

#endif