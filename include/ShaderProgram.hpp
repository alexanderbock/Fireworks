#ifndef _SHADER_PROGRAM_
#define _SHADER_PROGRAM_

#include <map>
#include <GL/glew.h>

#include "Shader.hpp"


class ShaderProgram{
public:
	//Constructor with initializer list
	ShaderProgram() :
		mHandle(GL_FALSE),
		mIsLinked(false)
	{

	};

	~ShaderProgram();

	void attachShader(Shader* shader);
	void compileAndLinkProgram();
	const GLuint handle();
	const bool isLinked();
	const GLuint getUniformLocation(const std::string &name);

protected:
private:
	bool mIsLinked;
	GLuint mHandle;
	std::map<GLenum, Shader*> mShaders;
};

#endif