#include <header.h>
#include "avif.h"
#include "avif/avif.h"
using namespace Regards::Picture;

CAvif::CAvif() {}

CAvif::~CAvif() {}

cv::Mat DecodeAvifImage(avifDecoder* decoder, avifImage* decoded, bool scale = false, int width = 0, int height = 0)
{
    cv::Mat out;

    if (!decoder || !decoded) {
        std::cerr << "Invalid decoder or image.\n";
        return out;
    }

    if (scale) {
        avifBool scaleResult = avifImageScale(decoded, width, height, decoder->imageSizeLimit, decoder->imageDimensionLimit, &decoder->diag);
        if (scaleResult != AVIF_TRUE) {
            std::cerr << "Failed to scale image.\n";
            return out;
        }
    }

    avifRGBImage dstRGB;
    avifRGBImageSetDefaults(&dstRGB, decoded);
    dstRGB.format = AVIF_RGB_FORMAT_BGRA;
    dstRGB.depth = 8;
    avifRGBImageAllocatePixels(&dstRGB);

    if (avifImageYUVToRGB(decoded, &dstRGB) == AVIF_RESULT_OK) {
        out.create(dstRGB.height, dstRGB.width, CV_8UC4);
        memcpy(out.data, dstRGB.pixels, dstRGB.height * dstRGB.width * 4);
    }
    else {
        std::cerr << "Failed to convert YUV to RGB.\n";
    }

    avifRGBImageFreePixels(&dstRGB);
    return out;
}

cv::Mat ProcessAvifFile(const char* filename, avifDecoder* decoder, bool scale = false, int width = 0, int height = 0)
{
    cv::Mat out;

    if (!decoder) {
        std::cerr << "Decoder is null.\n";
        return out;
    }

    avifImage* decoded = avifImageCreateEmpty();
    if (!decoded) {
        std::cerr << "Failed to create empty AVIF image.\n";
        return out;
    }

    avifResult result = avifDecoderSetIOFile(decoder, filename);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Cannot open file for read: " << filename << "\n";
        avifImageDestroy(decoded);
        return out;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Failed to parse image: " << avifResultToString(result) << "\n";
        avifImageDestroy(decoded);
        return out;
    }

    result = avifDecoderRead(decoder, decoded);
    if (result == AVIF_RESULT_OK) {
        out = DecodeAvifImage(decoder, decoded, scale, width, height);
    }
    else {
        std::cerr << "Failed to decode image: " << avifResultToString(result) << "\n";
    }

    avifImageDestroy(decoded);
    return out;
}

cv::Mat CAvif::GetPicture(const char* filename)
{
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) {
        std::cerr << "Failed to create AVIF decoder.\n";
        return cv::Mat();
    }

#if defined(__APPLE_) || defined(__ARM64__)
    decoder->codecChoice = AVIF_CODEC_CHOICE_AUTO;
#else
    decoder->codecChoice = AVIF_CODEC_CHOICE_DAV1D;
#endif

    cv::Mat out = ProcessAvifFile(filename, decoder);
    avifDecoderDestroy(decoder);
    return out;
}

cv::Mat CAvif::GetPictureThumb(const char* filename, const int& width, const int& height)
{
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) {
        std::cerr << "Failed to create AVIF decoder.\n";
        return cv::Mat();
    }

#if defined(__APPLE_) || defined(__ARM64__)
    decoder->codecChoice = AVIF_CODEC_CHOICE_AUTO;
#else
    decoder->codecChoice = AVIF_CODEC_CHOICE_DAV1D;
#endif

    cv::Mat out = ProcessAvifFile(filename, decoder, true, width, height);
    avifDecoderDestroy(decoder);
    return out;
}

bool CAvif::IsOccupied()
{
    return false;
}