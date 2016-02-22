#ifndef _SHADER_
#define _SHADER_

#include <string>
#include <GL/glew.h>
class Shader{
public:
	//Constructor with initializer list
	Shader() :
		mHandle(GL_FALSE),
		mType(GL_FALSE),
		mSource(""),
		mFile(""),
		mIsCompiled(false)
	{

	};

	Shader(GLenum type, const std::string source, bool isFile = true) :	Shader()
	{
		setShaderSource(type, source, isFile);
	};

	~Shader();
	void setShaderSource(GLenum shaderType, const std::string &source, bool isFile = true);
	const bool compileShader();
	const GLenum shaderType() const;
	const bool isCompiled() const;
	const GLuint handle() const;
protected:
private:
	GLuint mHandle;
	GLenum mType;
	std::string mFile;
	std::string mSource;
	bool mIsCompiled;
};

#endif