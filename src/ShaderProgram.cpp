#include "../include/Logger.hpp"
#include "../include/ShaderProgram.hpp"
#include "../include/Shader.hpp"

#include <vector>

ShaderProgram::~ShaderProgram(){

	for (std::map<GLenum, Shader*>::iterator it = mShaders.begin();
		it != mShaders.end();
		++it){
		if (it->second != nullptr){
			delete it->second;
			it->second = nullptr;
		}
	}

	if (mHandle != GL_FALSE){
		glDeleteProgram(mHandle);
		mHandle = GL_FALSE;
	}

	mShaders.clear();
}

void ShaderProgram::attachShader(Shader* shader){
	if (shader != nullptr){
		mShaders[shader->shaderType()] = shader;
	}
}

void ShaderProgram::compileAndLinkProgram(){

	if (mHandle != GL_FALSE){
		glDeleteProgram(mHandle);
	}

	mHandle = glCreateProgram();

	for (std::map<GLenum, Shader*>::iterator it = mShaders.begin();
		it != mShaders.end();
		++it){
		if (it->second != nullptr){
			if (it->second->compileShader()){
				glAttachShader(mHandle, it->second->handle());
			}
			else{
				LERROR("Failed to compile shader " << it->second->shaderType());
			}
		}
	}

	//check so everything went OK
	GLint result = 0;
	glLinkProgram(mHandle);
	glGetProgramiv(mHandle, GL_LINK_STATUS, &result);

	if (result == GL_FALSE){
		mIsLinked = false;
		GLint logsize = 0;
		glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &logsize);

		if (logsize < 1){
			LERROR("Shader compilation failed but log was empty, aborting...");
			return;
		}

		std::vector<GLchar> log(logsize);
		glGetProgramInfoLog(mHandle, logsize, 0, log.data());

		LERROR("Program linking failed, log:\n" + std::string(log.data()));

		//cleanup
		glDeleteProgram(mHandle);
		mHandle = GL_FALSE;
		return;
	}
}

const GLuint ShaderProgram::handle(){
	return mHandle;
}

const bool ShaderProgram::isLinked(){
	return mIsLinked;
}

const GLuint ShaderProgram::getUniformLocation(const std::string &name){
	return glGetUniformLocation(mHandle, name.c_str());
}