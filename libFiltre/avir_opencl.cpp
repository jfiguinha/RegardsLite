#pragma once
#include <header.h>
#include <avir_opencl.h>
#include <libResource.h>
#include "OpenCLParameter.h"
#include "OpenCLContext.h"

extern cv::ocl::OpenCLExecutionContext clExecCtx;
using namespace Regards::OpenCL;
using namespace cv;

UMat CAvirFilterOpenCL::ConvertToFloat(cv::UMat& src, const int& width, const int& height, const int& yPosition)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		//cv::Mat  paramDest(height, width, CV_32FC4, dest);
		// Crée un UMat avec le type CV_8UC4
		UMat paramSrc(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));


		vector<COpenCLParameter*> vecParam;

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramyPosition = new COpenCLParameterInt();
		paramyPosition->SetValue(yPosition);
		paramyPosition->SetLibelle("yPosition");
		vecParam.push_back(paramyPosition);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("ConvertToFloatPos", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		// Configuration et exécution du kernel
		size_t global_work_size[2] = { static_cast<size_t>(width), static_cast<size_t>(height) };
		bool success = kernel.run(2, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		//COpenCLContext::GetOutputData(clBuffer, dest, width * height * sizeof(float) * 4, flag);

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		//paramSrc.copyTo(paramDest);
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
		return paramSrc;
	}
}

UMat CAvirFilterOpenCL::ConvertToFloat(cv::UMat& src, const int& width, const int& height)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		//cv::Mat  paramDest(height, width, CV_32FC4, dest);
		// Crée un UMat avec le type CV_8UC4
		UMat paramSrc(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));


		vector<COpenCLParameter*> vecParam;

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("ConvertToFloat", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		// Configuration et exécution du kernel
		size_t global_work_size[2] = { static_cast<size_t>(width), static_cast<size_t>(height) };
		bool success = kernel.run(2, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		//COpenCLContext::GetOutputData(clBuffer, dest, width * height * sizeof(float) * 4, flag);

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		return paramSrc;
	}
}

UMat CAvirFilterOpenCL::ConvertToFloat(const uchar* data, const int& width, const int& height)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		//cv::Mat  paramDest(height, width, CV_32FC4, dest);
		// Crée un UMat avec le type CV_8UC4
		UMat paramSrc(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));


		vector<COpenCLParameter*> vecParam;
		COpenCLParameterByteArray* input = new COpenCLParameterByteArray();
		input->SetLibelle("input");
		input->SetValue((cl_context)clExecCtx.getContext().ptr(), (uint8_t*)data, width * height * 4, flag);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("ConvertToFloat", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		// Configuration et exécution du kernel
		size_t global_work_size[2] = { static_cast<size_t>(width), static_cast<size_t>(height) };
		bool success = kernel.run(2, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		//COpenCLContext::GetOutputData(clBuffer, dest, width * height * sizeof(float) * 4, flag);

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		//paramSrc.copyTo(paramDest);
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
		return paramSrc;
	}
}

UMat CAvirFilterOpenCL::UpSampleUMat(cv::UMat& src, const int& width, const int& height, int widthSrc, int start, int ResampleFactor)
{
	UMat paramSrc(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;


		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));

		vector<COpenCLParameter*> vecParam;

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramWidthSrc = new COpenCLParameterInt();
		paramWidthSrc->SetValue(widthSrc);
		paramWidthSrc->SetLibelle("widthSrc");
		vecParam.push_back(paramWidthSrc);

		auto paramStart = new COpenCLParameterInt();
		paramStart->SetValue(start);
		paramStart->SetLibelle("start");
		vecParam.push_back(paramStart);

		auto paramsrcResampleFactor = new COpenCLParameterInt();
		paramsrcResampleFactor->SetValue(ResampleFactor);
		paramsrcResampleFactor->SetLibelle("ResampleFactor");
		vecParam.push_back(paramsrcResampleFactor);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("UpSample", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

	}
	return paramSrc;
}


UMat CAvirFilterOpenCL::UpSample2D(cv::UMat& src, const int& width, const int& height, int widthSrc, int start, int outLen, int ResampleFactor, int opstep)
{
	UMat paramSrc(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;


		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));

		vector<COpenCLParameter*> vecParam;

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramWidthSrc = new COpenCLParameterInt();
		paramWidthSrc->SetValue(widthSrc);
		paramWidthSrc->SetLibelle("widthSrc");
		vecParam.push_back(paramWidthSrc);

		auto paramStart = new COpenCLParameterInt();
		paramStart->SetValue(start);
		paramStart->SetLibelle("start");
		vecParam.push_back(paramStart);

		auto paramOutLen = new COpenCLParameterInt();
		paramOutLen->SetValue(outLen);
		paramOutLen->SetLibelle("outLen");
		vecParam.push_back(paramOutLen);

		auto paramsrcResampleFactor = new COpenCLParameterInt();
		paramsrcResampleFactor->SetValue(ResampleFactor);
		paramsrcResampleFactor->SetLibelle("ResampleFactor");
		vecParam.push_back(paramsrcResampleFactor);

		auto paramsrcStep = new COpenCLParameterInt();
		paramsrcStep->SetValue(opstep);
		paramsrcStep->SetLibelle("opstep");
		vecParam.push_back(paramsrcStep);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("UpSample2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

	}
	return paramSrc;
}

float* CAvirFilterOpenCL::UpSample(cv::UMat& src, float*& dest, const int& width, const int& height, int widthSrc, int start, int ResampleFactor)
{
	cv::Mat  paramDest(height, width, CV_32FC4, dest);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;


		// Crée un UMat avec le type CV_8UC4
		UMat paramSrc(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramSrc.handle(ACCESS_WRITE));

		vector<COpenCLParameter*> vecParam;

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramWidthSrc = new COpenCLParameterInt();
		paramWidthSrc->SetValue(widthSrc);
		paramWidthSrc->SetLibelle("widthSrc");
		vecParam.push_back(paramWidthSrc);

		auto paramStart = new COpenCLParameterInt();
		paramStart->SetValue(start);
		paramStart->SetLibelle("start");
		vecParam.push_back(paramStart);

		auto paramsrcResampleFactor = new COpenCLParameterInt();
		paramsrcResampleFactor->SetValue(ResampleFactor);
		paramsrcResampleFactor->SetLibelle("ResampleFactor");
		vecParam.push_back(paramsrcResampleFactor);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("UpSample", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		//return paramSrc;
		paramSrc.copyTo(paramDest);
		//return paramDest;
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
	}
	return dest;
}

UMat CAvirFilterOpenCL::doResize2OpenCL(cv::UMat& src, const int& width, const int& height,
	const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen)
{
	UMat paramOutput(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterIntArray* paramrPos = new COpenCLParameterIntArray();
		paramrPos->SetLibelle("PositionTab");
		paramrPos->SetValue((cl_context)clExecCtx.getContext().ptr(), (int*)&PositionTab.at(0), PositionTab.size(), flag);
		vecParam.push_back(paramrPos);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("ftp");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)&ftp.at(0), ftp.size(), flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(IntFltLen);
		paramIntFltLen->SetLibelle("IntFltLen");
		vecParam.push_back(paramIntFltLen);

		auto paramWidthInput = new COpenCLParameterInt();
		paramWidthInput->SetValue(src.size().width);
		paramWidthInput->SetLibelle("inputWidth");
		vecParam.push_back(paramWidthInput);
		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doResize2", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
	return paramOutput;
}


UMat CAvirFilterOpenCL::doResize2OpenCL2D(cv::UMat& src, const int& width, const int& height,
	const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen)
{
	UMat paramOutput(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterIntArray* paramrPos = new COpenCLParameterIntArray();
		paramrPos->SetLibelle("PositionTab");
		paramrPos->SetValue((cl_context)clExecCtx.getContext().ptr(), (int*)&PositionTab.at(0), PositionTab.size(), flag);
		vecParam.push_back(paramrPos);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("ftp");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)&ftp.at(0), ftp.size(), flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(IntFltLen);
		paramIntFltLen->SetLibelle("IntFltLen");
		vecParam.push_back(paramIntFltLen);

		auto paramWidthInput = new COpenCLParameterInt();
		paramWidthInput->SetValue(src.size().width);
		paramWidthInput->SetLibelle("inputWidth");
		vecParam.push_back(paramWidthInput);
		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doResize22D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
	return paramOutput;
}


void CAvirFilterOpenCL::doResize2OpenCL(cv::UMat& src, float*& dest, const int& width, const int& height,
	const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		cv::Mat paramDest(height, width, CV_32FC4, dest);
		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterIntArray* paramrPos = new COpenCLParameterIntArray();
		paramrPos->SetLibelle("PositionTab");
		paramrPos->SetValue((cl_context)clExecCtx.getContext().ptr(), (int*)&PositionTab.at(0), PositionTab.size(), flag);
		vecParam.push_back(paramrPos);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("ftp");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)&ftp.at(0), ftp.size(), flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(IntFltLen);
		paramIntFltLen->SetLibelle("IntFltLen");
		vecParam.push_back(paramIntFltLen);

		auto paramWidthInput = new COpenCLParameterInt();
		paramWidthInput->SetValue(src.size().width);
		paramWidthInput->SetLibelle("inputWidth");
		vecParam.push_back(paramWidthInput);
		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doResize2", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
		//return paramDest;
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
	}
}

UMat CAvirFilterOpenCL::doResizeOpenCL2D(cv::UMat& src, const int& width, const int& height, const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen)
{
	UMat paramOutput(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterIntArray* paramrPos = new COpenCLParameterIntArray();
		paramrPos->SetLibelle("PositionTab");
		paramrPos->SetValue((cl_context)clExecCtx.getContext().ptr(), (int*)&PositionTab.at(0), PositionTab.size(), flag);
		vecParam.push_back(paramrPos);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("ftp");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)&ftp.at(0), ftp.size(), flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(IntFltLen);
		paramIntFltLen->SetLibelle("IntFltLen");
		vecParam.push_back(paramIntFltLen);

		auto paramWidthInput = new COpenCLParameterInt();
		paramWidthInput->SetValue(src.size().width);
		paramWidthInput->SetLibelle("inputWidth");
		vecParam.push_back(paramWidthInput);
		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doResize2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
	return paramOutput;
}


void CAvirFilterOpenCL::doResizeOpenCL(cv::UMat& src, float*& dest, const int& width, const int& height,
	const std::vector<int>& PositionTab, const std::vector<float>& ftp, int IntFltLen)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		cv::Mat paramDest(height, width, CV_32FC4, dest);
		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterIntArray* paramrPos = new COpenCLParameterIntArray();
		paramrPos->SetLibelle("PositionTab");
		paramrPos->SetValue((cl_context)clExecCtx.getContext().ptr(), (int*)&PositionTab.at(0), PositionTab.size(), flag);
		vecParam.push_back(paramrPos);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("ftp");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)&ftp.at(0), ftp.size(), flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(IntFltLen);
		paramIntFltLen->SetLibelle("IntFltLen");
		vecParam.push_back(paramIntFltLen);

		auto paramWidthInput = new COpenCLParameterInt();
		paramWidthInput->SetValue(src.size().width);
		paramWidthInput->SetLibelle("inputWidth");
		vecParam.push_back(paramWidthInput);
		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doResize", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
		//return paramDest;
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
	}
}

void CAvirFilterOpenCL::doFilterOpenCL(cv::UMat& src, float*& dest, const int& width, const int& height,
	const float* f, int flen)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		cv::Mat paramDest(height, width, CV_32FC4, dest);
		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(height, width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("f");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)f, flen, flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(flen);
		paramIntFltLen->SetLibelle("flen");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doFilter", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
		//return paramDest;
		//memcpy(dest, paramDest.data, width * height * 4 * sizeof(float));
	}
}

cv::UMat CAvirFilterOpenCL::GetDataOpenUMat(cv::UMat& src, const int& iPos)
{
	UMat paramOutput(1, src.size().width, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("yPos");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetData", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
	return paramOutput;
}

cv::Mat CAvirFilterOpenCL::GetDataOpenCL(cv::UMat& src, const int& iPos)
{
	cv::Mat paramDest(1, src.size().width, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, src.size().width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetData", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
	return paramDest;
}


void CAvirFilterOpenCL::GetDataOpenCL(cv::UMat& src, float*& dest, const int& iPos)
{
	cv::Mat paramDest(1, src.size().width, CV_32FC4, dest);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, src.size().width, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetData", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
}


cv::Mat CAvirFilterOpenCL::GetDataOpenCLHtoV(cv::UMat& src, const int& iPos)
{
	cv::Mat paramDest(1, src.size().height, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, src.size().height, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().height) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
	return paramDest;
}


cv::UMat CAvirFilterOpenCL::GetDataOpenCLHtoV2D(cv::UMat& src)
{
	UMat paramOutput(src.size().width, src.size().height, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);


		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
	return paramOutput;
}


void CAvirFilterOpenCL::GetDataOpenCLHtoV(cv::UMat& src, float*& dest, const int& iPos)
{
	cv::Mat paramDest(1, src.size().height, CV_32FC4, dest);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, src.size().height, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().height) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
	//return paramDest;
}

UMat CAvirFilterOpenCL::GetDataOpenCLHtoV_dither(cv::UMat& src, int iPos, float gm)
{
	UMat paramOutput(1, src.size().height, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		auto paramGM = new COpenCLParameterFloat();
		paramGM->SetValue(gm);
		paramGM->SetLibelle("gm");
		vecParam.push_back(paramGM);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV_dither", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().height) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

	}
	return paramOutput;
}


UMat CAvirFilterOpenCL::GetDataOpenCLHtoV_dither2D(cv::UMat& src, float gm)
{
	UMat paramOutput(src.size().width, src.size().height, CV_32FC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramGM = new COpenCLParameterFloat();
		paramGM->SetValue(gm);
		paramGM->SetLibelle("gm");
		vecParam.push_back(paramGM);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV_dither2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().height) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

	}
	return paramOutput;
}

void CAvirFilterOpenCL::DitherOpenCL(cv::UMat& src, float*& dest, int srcLen, float PkOut, float TrMul0)
{
	cv::Mat paramDest(1, srcLen, CV_32FC4, dest);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, srcLen, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramPkOut = new COpenCLParameterFloat();
		paramPkOut->SetValue(PkOut);
		paramPkOut->SetLibelle("PkOut");
		vecParam.push_back(paramPkOut);

		auto paramTrMul0 = new COpenCLParameterFloat();
		paramTrMul0->SetValue(TrMul0);
		paramTrMul0->SetLibelle("TrMul0");
		vecParam.push_back(paramTrMul0);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("Dither", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(srcLen) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
	//return paramDest;
}


cv::UMat CAvirFilterOpenCL::DitherOpenCL2D(cv::UMat& src, float PkOut, float TrMul0)
{
	UMat paramOutput(src.size().height, src.size().width, CV_8UC4);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramPkOut = new COpenCLParameterFloat();
		paramPkOut->SetValue(PkOut);
		paramPkOut->SetLibelle("PkOut");
		vecParam.push_back(paramPkOut);

		auto paramTrMul0 = new COpenCLParameterFloat();
		paramTrMul0->SetValue(TrMul0);
		paramTrMul0->SetLibelle("TrMul0");
		vecParam.push_back(paramTrMul0);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("Dither2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

	}
	return paramOutput;
}

void CAvirFilterOpenCL::GetDataOpenCLHtoV_dither(cv::UMat& src, float*& dest, const int& iPos, float gm)
{
	cv::Mat paramDest(1, src.size().height, CV_32FC4, dest);
	cl_mem_flags flag;
	{


		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4
		UMat paramOutput(1, src.size().height, CV_32FC4);
		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);


		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(src.size().height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("xPos");
		vecParam.push_back(paramIntFltLen);

		auto paramGM = new COpenCLParameterFloat();
		paramGM->SetValue(gm);
		paramGM->SetLibelle("gm");
		vecParam.push_back(paramGM);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("GetDataHtoV_dither", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().height) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		paramOutput.copyTo(paramDest);
	}
	//return paramDest;
}

void CAvirFilterOpenCL::doCopyOpenCL_UMat(cv::UMat& output, cv::UMat& src, const int& iPos)
{
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(output.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(src.size().width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(iPos);
		paramIntFltLen->SetLibelle("heightPosition");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doCopy", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(src.size().width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}
	}
}


UMat CAvirFilterOpenCL::doFilterOpenCL_UMat(cv::UMat& src, const int& width, const int& height,
	const float* f, int flen)
{
	UMat paramOutput(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("f");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)f, flen, flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(flen);
		paramIntFltLen->SetLibelle("flen");
		vecParam.push_back(paramIntFltLen);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doFilter", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		return paramOutput;
	}
}


UMat CAvirFilterOpenCL::doFilterOpenCL2D(cv::UMat& src, const int& width, const int& height,
	const float* f, int flen, int step)
{
	UMat paramOutput(height, width, CV_32FC4);
	cl_mem_flags flag;
	{
		bool useMemory = (cv::ocl::Device::getDefault().type() == CL_DEVICE_TYPE_GPU) ? false : true;
		flag = useMemory ? CL_MEM_USE_HOST_PTR : CL_MEM_COPY_HOST_PTR;

		vector<COpenCLParameter*> vecParam;
		// Crée un UMat avec le type CV_8UC4

		auto clBuffer = static_cast<cl_mem>(paramOutput.handle(ACCESS_WRITE));

		auto clInputBuffer = static_cast<cl_mem>(src.handle(ACCESS_READ));
		auto input = new COpenCLParameterClMem(true);
		input->SetValue(clInputBuffer);
		input->SetLibelle("input");
		input->SetNoDelete(true);
		vecParam.push_back(input);

		auto paramWidtSrc = new COpenCLParameterInt();
		paramWidtSrc->SetValue(src.size().width);
		paramWidtSrc->SetLibelle("widthSrc");
		vecParam.push_back(paramWidtSrc);

		auto paramHeightSrc = new COpenCLParameterInt();
		paramHeightSrc->SetValue(src.size().height);
		paramHeightSrc->SetLibelle("heightSrc");
		vecParam.push_back(paramHeightSrc);

		auto paramWidth = new COpenCLParameterInt();
		paramWidth->SetValue(width);
		paramWidth->SetLibelle("width");
		vecParam.push_back(paramWidth);

		auto paramHeight = new COpenCLParameterInt();
		paramHeight->SetValue(height);
		paramHeight->SetLibelle("height");
		vecParam.push_back(paramHeight);

		COpenCLParameterFloatArray* paramrfltBank = new COpenCLParameterFloatArray();
		paramrfltBank->SetLibelle("f");
		paramrfltBank->SetValue((cl_context)clExecCtx.getContext().ptr(), (float*)f, flen, flag);
		vecParam.push_back(paramrfltBank);

		auto paramIntFltLen = new COpenCLParameterInt();
		paramIntFltLen->SetValue(flen);
		paramIntFltLen->SetLibelle("flen");
		vecParam.push_back(paramIntFltLen);

		auto paramIntStep = new COpenCLParameterInt();
		paramIntStep->SetValue(step);
		paramIntStep->SetLibelle("step");
		vecParam.push_back(paramIntStep);

		// Récupération du code source du kernel
		wxString kernelSource = CLibResource::GetOpenCLUcharProgram("IDR_OPENCL_AVIR");
		cv::ocl::ProgramSource programSource(kernelSource);
		ocl::Context context = clExecCtx.getContext();

		// Compilation du kernel
		String errmsg;
		String buildopt = ""; // Options de compilation (vide par défaut)
		ocl::Program program = context.getProg(programSource, buildopt, errmsg);

		ocl::Kernel kernel("doFilter2D", program);

		// Définition du premier argument (outBuffer)
		cl_int err = clSetKernelArg(static_cast<cl_kernel>(kernel.ptr()), 0, sizeof(cl_mem), &clBuffer);
		if (err != CL_SUCCESS)
		{
			throw std::runtime_error("Failed to set kernel argument for outBuffer.");
		}

		// Ajout des autres arguments
		int numArg = 1;
		for (COpenCLParameter* parameter : vecParam)
		{
			parameter->Add(static_cast<cl_kernel>(kernel.ptr()), numArg++);
		}

		size_t global_work_size[1] = { static_cast<size_t>(width) };
		bool success = kernel.run(1, global_work_size, nullptr, true);
		if (!success)
		{
			throw std::runtime_error("Failed to execute OpenCL kernel.");
		}

		for (COpenCLParameter* parameter : vecParam)
		{
			if (!parameter->GetNoDelete())
			{
				delete parameter;
				parameter = nullptr;
			}
		}

		return paramOutput;
	}
}