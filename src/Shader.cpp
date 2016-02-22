#include "../include/Logger.hpp"
#include "../include/Shader.hpp"

#include <fstream>
#include <sstream>
#include <vector>

Shader::~Shader(){
	if (mHandle != GL_FALSE){
		glDeleteShader(mHandle);
		mHandle = GL_FALSE;
	}
}

void Shader::setShaderSource(GLenum shaderType, const std::string &source, bool isFile){
	std::string shader;

	mType = shaderType;

	if (isFile){
		std::ifstream file;
		mFile = source;

		file.open(mFile.c_str());

		if (file.is_open()){
			//create stringstream and read entire file
			std::stringstream ss;
			ss << file.rdbuf();

			//assign shader string
			shader = ss.str();

			file.close();
		}
		else{
			//LERROR("HEJ");
			LERROR("File " + source + " could not be opened...");
			return;
		}
	}
	else{
		mFile.clear();
		shader = source;
	}

	mSource = shader;
}

const bool Shader::compileShader(){
	if (!mFile.empty()){
		setShaderSource(mType, mFile, true);
	}	
	if (mSource.empty()){
		LERROR("Empty shader source, aborting...");
		return false;
	}
	
	//cleanup if theres already a shader compiled
	if (mHandle != GL_FALSE){
		glDeleteShader(mHandle);
	}

	mHandle = glCreateShader(mType);

	//construct source string and length
	const char *str[] = { mSource.c_str() };
	GLint len = static_cast<GLint>(mSource.length());

	glShaderSource(mHandle, 1, str, &len);

	glCompileShader(mHandle);

	//check so that everything went OK
	GLint result = GL_FALSE;
	glGetShaderiv(mHandle, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE){
		mIsCompiled = false;
		GLint logsize = 0;
		glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &logsize);
		
		if (logsize < 1){
			LERROR("Shader compilation failed but log was empty, aborting...");
			return false;
		}

		std::vector<GLchar> log(logsize);
		glGetShaderInfoLog(mHandle, logsize, 0, log.data());

		LINFO("Shader compilation failed, log:\n" + std::string(log.data()));

		//cleanup
		glDeleteShader(mHandle);
		mHandle = GL_FALSE;
		return false;
	}

	mIsCompiled = true;
	return true;
}

const GLenum Shader::shaderType() const{
	return mType;
}

const bool Shader::isCompiled() const{
	return mIsCompiled;
}

const GLuint Shader::handle() const{
	return mHandle;
}