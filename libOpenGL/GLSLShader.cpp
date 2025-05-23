#include <header.h>
#include "GLSLShader.h"
#include <LibResource.h>
#include <ConvertUtility.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
using namespace Regards::OpenGL;

GLSLShader::GLSLShader(void): m_hVertexHandle(0), m_hComputeHandle(0)
{
	m_hProgramObject = 0;
	m_hShaderHandle = 0;
}

GLSLShader::~GLSLShader(void)
{
	DeleteShader();
}

// helper to check and display for shader compiler errors
bool GLSLShader::check_shader_compile_status(GLuint obj)
{
	GLint status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		char cBuffer[2400];
		GLint length;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
		//cBuffer = new char[length + 1];
		glGetShaderInfoLog(obj, length, &length, cBuffer);

		printf("Error %s \n", cBuffer);
		return false;
	}
	printf("check_shader_compile_status is OK \n");
	return true;
}


// helper to check and display for shader linker error
bool GLSLShader::check_program_link_status(GLuint obj)
{
	GLint status;
	glGetProgramiv(obj, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		char cBuffer[800];
		GLint length;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
		//cBuffer = new char[length + 1];
		glGetShaderInfoLog(obj, length, &length, cBuffer);

		printf("Error %s \n", cBuffer);
		/*
		int nOutCount = 0;
		WCHAR *pWideChar = new WCHAR[length + 1];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cBuffer, length, pWideChar, nOutCount);
		for (auto nStr = 0; nStr < length; nStr++)
		{
			short s = cBuffer[nStr];
			pWideChar[nStr] = s;
		}
		pWideChar[length] = 0;
#ifdef _DEBUG
		OutputDebugString(pWideChar);
#endif
		delete[] pWideChar;
		*/
		return false;
	}
	return true;
}

bool GLSLShader::CreateProgram(const wxString& nProgramID_i, GLenum glSlShaderType_i)
{
	if (m_hProgramObject == 0)
		m_hProgramObject = glCreateProgram();

	if (glSlShaderType_i == GL_VERTEX_SHADER)
	{
		return CreateVertexProgram(nProgramID_i);
	}
	if (glSlShaderType_i == GL_COMPUTE_SHADER)
		return CreateComputeProgram(nProgramID_i);

	return CreateShaderProgram(nProgramID_i);
}

bool GLSLShader::CreateShaderProgram(const wxString& nProgramID_i)
{
	m_hShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);

	if (0 == m_hShaderHandle)
	{
		// Set Error messages.
		//AppError::SetError( CString(L"0 == m_hShaderHandle GL_FRAGMENT_PROGRAM_ARB shader creation failed") );
		return false;
	}

	wxString dataProgram = CLibResource::GetOpenGLShaderProgram(nProgramID_i);

	int length = static_cast<int>(dataProgram.size()) + 1;
	auto src = new char[length];
	strcpy(src, CConvertUtility::ConvertToUTF8(dataProgram));
	printf("Opengl shader : \n");
	printf(src);
	printf("Opengl end shader : \n");
	//printf(src);

	glShaderSource(m_hShaderHandle, 1, (const GLcharARB**)&src, &length);

	//glShaderSourceARB( m_hShaderHandle, strlen( pBufferResData ), (const GLcharARB**)&pbyShaderData, &nLENGTH );
	glCompileShader(m_hShaderHandle);

	if (!check_shader_compile_status(m_hShaderHandle))
	{
		GLint maxLength = 0;
		glGetShaderiv(m_hShaderHandle, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(m_hShaderHandle, maxLength, &maxLength, &errorLog[0]);
		std::string s(begin(errorLog), end(errorLog));
		cout << s.c_str() << endl;
		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(m_hShaderHandle); // Don't leak the shader.
		return false;
	}
	glAttachShader(m_hProgramObject, m_hShaderHandle);
	delete[] src;
	return true;
}

bool GLSLShader::CreateComputeProgram(const wxString& nProgramID_i)
{
	m_hComputeHandle = glCreateShader(GL_COMPUTE_SHADER);

	if (0 == m_hVertexHandle)
	{
		return false;
	}

	wxString dataProgram = CLibResource::GetOpenGLShaderProgram(nProgramID_i);

	int length = static_cast<int>(dataProgram.size()) + 1;
	auto data = new char[length];
	strcpy(data, CConvertUtility::ConvertToUTF8(dataProgram));
	glShaderSource(m_hComputeHandle, 1, (const GLcharARB**)&data, &length);

	glCompileShader(m_hComputeHandle);

	if (!check_shader_compile_status(m_hComputeHandle))
		return false;

	glAttachShader(m_hProgramObject, m_hComputeHandle);
	delete[] data;
	return true;
}


bool GLSLShader::CreateVertexProgram(const wxString& nProgramID_i)
{
	m_hVertexHandle = glCreateShader(GL_VERTEX_SHADER);

	if (0 == m_hVertexHandle)
	{
		return false;
	}

	wxString dataProgram = CLibResource::GetOpenGLShaderProgram(nProgramID_i);

	int length = static_cast<int>(dataProgram.size()) + 1;
	auto data = new char[length];
	strcpy(data, CConvertUtility::ConvertToUTF8(dataProgram));
	glShaderSource(m_hVertexHandle, 1, (const GLcharARB**)&data, &length);

	glCompileShader(m_hVertexHandle);

	if (!check_shader_compile_status(m_hVertexHandle))
		return false;

	glAttachShader(m_hProgramObject, m_hVertexHandle);
	delete[] data;
	return true;
}

bool GLSLShader::DeleteShader()
{
	if (m_hVertexHandle)
	{
		glDetachShader(m_hProgramObject, m_hVertexHandle);
		glDeleteShader(m_hVertexHandle);
		m_hVertexHandle = 0;
	}
	if (m_hComputeHandle)
	{
		glDetachShader(m_hProgramObject, m_hComputeHandle);
		glDeleteShader(m_hComputeHandle);
		m_hComputeHandle = 0;
	}
	if (m_hShaderHandle)
	{
		glDetachShader(m_hProgramObject, m_hShaderHandle);
		glDeleteShader(m_hShaderHandle);
		m_hShaderHandle = 0;
	}
	if (m_hProgramObject)
	{
		glDeleteProgram(m_hProgramObject);
		m_hProgramObject = 0;
	}
	return true;
}

bool GLSLShader::EnableShader()
{
	glLinkProgram(m_hProgramObject);

	if (!check_program_link_status(m_hProgramObject))
		return false;

	glUseProgram(m_hProgramObject);
	return true;
}

bool GLSLShader::DisableShader()
{
	// int nErr = 0;
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}


bool GLSLShader::SetTexture(const char* pParamName_i, const int nTextureID_i)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + nTextureID_i);
	glBindTexture(GL_TEXTURE_2D, nTextureID_i);
	glUniform1i(nParamObj, nTextureID_i);
	GLenum glErr = glGetError();
	//printf("SetTexture glError %p \n", gluErrorString(glErr));
	return (GL_NO_ERROR == glErr);
}

bool GLSLShader::SetParam(const char* pParamName_i, const float fValue_i)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	glUniform1f(nParamObj, fValue_i);
	GLenum glErr = glGetError();
	//printf("SetParam glError %p\n", gluErrorString(glErr));
	return (GL_NO_ERROR == glErr);
}

bool GLSLShader::SetIntegerParam(const char* pParamName_i, const int iValue_i)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	glUniform1i(nParamObj, iValue_i);
	return (GL_NO_ERROR == glGetError());
}

bool GLSLShader::SetFloatVectorParam(const char* pParamName_i, const int size, float* tabVecs)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	if (tabVecs != nullptr)
		glUniform1fv(nParamObj, size, tabVecs);

	return (GL_NO_ERROR == glGetError());
}

bool GLSLShader::SetIntVectorParam(const char* pParamName_i, const int size, int* tabVecs)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	if (tabVecs != nullptr)
		glUniform1iv(nParamObj, size, tabVecs);

	return (GL_NO_ERROR == glGetError());
}

bool GLSLShader::SetVec3Param(const char* pParamName_i, vec3f iValue_i)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

    glUniform3f(nParamObj, iValue_i.x, iValue_i.y, iValue_i.z);

	return (GL_NO_ERROR == glGetError());
}

bool GLSLShader::SetMatrixParam(const char* pParamName_i, float* tabVecs)
{
	GLint nParamObj = glGetUniformLocation(m_hProgramObject, pParamName_i);
	if (-1 == nParamObj)
	{
		return false;
	}

	// send our projection matrix to the shader
	if (tabVecs != nullptr)
		glUniformMatrix4fv(nParamObj, 1, GL_FALSE, tabVecs);

	return (GL_NO_ERROR == glGetError());
}
