#include "header.h"
#include "PictureArray.h"
#include <opencv2/core/opengl.hpp>
using namespace Regards::Picture;

CPictureArray::CPictureArray(const cv::_InputArray::KindFlag& type)
{
	kind = type;
}

CPictureArray::CPictureArray(cv::Mat& m)
{
	mat = m;
	kind = cv::_InputArray::KindFlag::MAT;
}

CPictureArray::CPictureArray(cv::cuda::GpuMat& d_mat)
{
	gpuMat = d_mat;
	kind = cv::_InputArray::KindFlag::CUDA_GPU_MAT;
}
void CPictureArray::SetArray(cv::Mat& m)
{
  	mat = m;
	kind = cv::_InputArray::KindFlag::MAT;  
}

void CPictureArray::SetArray(cv::cuda::GpuMat& d_mat)
{
 	d_mat = gpuMat;
	kind = cv::_InputArray::KindFlag::CUDA_GPU_MAT;   
}

void CPictureArray::SetArray(cv::UMat& m)
{
  	umat = m;
	kind = cv::_InputArray::KindFlag::UMAT;  
}


CPictureArray::CPictureArray(cv::UMat& m)
{
	umat = m;
	kind = cv::_InputArray::KindFlag::UMAT;
}

int CPictureArray::getWidth()
{
	return getSize().width;
}

int CPictureArray::getHeight()
{
	return getSize().height;
}

cv::Size CPictureArray::getSize()
{
	switch (kind)
	{
	case cv::_InputArray::KindFlag::CUDA_GPU_MAT:
		return gpuMat.size();
	case cv::_InputArray::KindFlag::MAT:
		return mat.size();
	case cv::_InputArray::KindFlag::UMAT:
		return umat.size();
	default:
		throw std::runtime_error("Unsupported kind in CPictureArray::getSize");
	}
}

void CPictureArray::CopyFrom(cv::ogl::Texture2D* tex)
{
	try
	{
		switch (kind)
		{
		case cv::_InputArray::KindFlag::CUDA_GPU_MAT:
			tex->copyFrom(gpuMat, true);
			break;
		case cv::_InputArray::KindFlag::MAT:
			tex->copyFrom(mat, true);
			break;
		case cv::_InputArray::KindFlag::UMAT:
			tex->copyFrom(umat, true);
			break;
		default:
			throw std::runtime_error("Unsupported kind in CPictureArray::CopyFrom");
		}
	}
	catch (const cv::Exception& e)
	{
		std::cerr << "OpenCV exception in CPictureArray::CopyFrom: " << e.what() << std::endl;
		std::cerr << "Falling back to cv::Mat." << std::endl;
		tex->copyFrom(getMat(), true);
	}
}

cv::_InputArray::KindFlag CPictureArray::Kind()
{
	return kind;
}

cv::UMat& CPictureArray::getUMat()
{
	if(kind == cv::_InputArray::KindFlag::CUDA_GPU_MAT)
	{
		cv::Mat local;
		gpuMat.download(local);
		local.copyTo(umat);
		return umat;
	}
	else if(kind == cv::_InputArray::KindFlag::MAT)
	{
		mat.copyTo(umat);
		return umat;
	}
	return umat;
}

cv::Mat& CPictureArray::getMat()
{
	if (kind == cv::_InputArray::KindFlag::CUDA_GPU_MAT)
	{
		gpuMat.download(mat);
		return mat;
	}
	else if (kind == cv::_InputArray::KindFlag::UMAT)
	{
		umat.copyTo(mat);
		return mat;
	}
	return mat;
}

cv::cuda::GpuMat& CPictureArray::getGpuMat()
{
	if (kind == cv::_InputArray::KindFlag::MAT)
	{
		gpuMat.upload(mat);
		return gpuMat;
	}
	else if (kind == cv::_InputArray::KindFlag::UMAT)
	{
		cv::Mat local;
		umat.copyTo(local);
		gpuMat.upload(local);
		return gpuMat;
	}
	return gpuMat;
}


void CPictureArray::copyTo(cv::Mat& m)
{
	cv::Mat mat = getMat();
	mat.copyTo(m);
}

void CPictureArray::copyTo(cv::cuda::GpuMat& d_mat)
{
	cv::cuda::GpuMat mat = getGpuMat();
	mat.copyTo(d_mat);
}

void CPictureArray::copyTo(cv::UMat& m)
{
	cv::UMat mat = getUMat();
	mat.copyTo(m);
}