#include <sgct.h>

//std includes
#include <iostream>
#include <algorithm>

//project includes
#include "include/ParticleSystem.hpp"
#include "include/Shader.hpp"
#include "include/ShaderProgram.hpp"

//logging
#include "include/Logger.hpp"

//SGCT callbacks
void drawFun();
void preSyncFun();
void postSyncPreDraw();
void initFun();
void encodeFun();
void decodeFun();
void cleanUpFun();
void postDrawCallBack();
void keyCallback(int key, int scancode, int action, int mods);

//SGCT Engine
sgct::Engine * gEngine;

//SGCT cluster variables
sgct::SharedBool gShowInfo(false);
sgct::SharedFloat gDt(0.f);
sgct::SharedBool gShowStats(false);
sgct::SharedBool gAddFireworks(false);

//particle system (aka. fireworks)
std::vector<ParticleSystem*> mFireworks;
ShaderProgram *mParticleShaderProgram;

GLuint mLogoQuadVertexArray;
ShaderProgram* mLogoShaderProgram;

//textures to hold accumulated frames
GLuint mLeftAccumTex = GL_FALSE;
GLuint mRightAccumTex = GL_FALSE;

//screen quad for blending
GLuint mScreenQuadVertexArray;
ShaderProgram *mScreenQuadProgram;

//command-line arguments for controlling appearance + behaviour
float mBESizeScale = 1.f;		//the on-screen size of the generated billboards for particles BEFORE explosion
float mAESizeScale = 1.f;		//the on-screen size of the generated billboards for particles AFTER explosion
bool mUseParentColor = false;	//if a firework will explode in randomized colors or keep its "parent color"
float mSpeedScale = 1.f;		//scaling of speed
float mLaunchTime = 8.f;		//maximum time before a firework explodes
float mBurnTime = 2.f;			//how long a firework burns after exploding

void parseArguments(int argc, char* argv[]){
	std::string tmp;
	for (int n = 0; n < argc; n++){
		tmp = argv[n];
		if (strcmp(argv[n], "-BEsize") == 0 && argc > n + 1)
		{
			mBESizeScale = atof(argv[++n]);
			LINFO("Setting size scale of particles BEFORE explosion to " << mBESizeScale);
		}
		else if (strcmp(argv[n], "-AEsize") == 0 && argc > n + 1)
		{
			mAESizeScale = atof(argv[++n]);
			LINFO("Setting size scale of particles AFTER explosion to " << mBESizeScale);
		}
		else if (strcmp(argv[n], "-upc") == 0 && argc > n + 1)
		{
			mUseParentColor = atoi(argv[++n]) > 0;
			LINFO("Setting use parent color to " << mUseParentColor);
		}
		else if (strcmp(argv[n], "-speed") == 0 && argc > n + 1)
		{
			mSpeedScale = atof(argv[++n]);
			LINFO("Setting speed scale to " << mSpeedScale);
		}
		else if (strcmp(argv[n], "-lt") == 0 && argc > n + 1)
		{
			mLaunchTime = atof(argv[++n]);
			LINFO("Setting launch time to " << mLaunchTime);
		}
		else if (strcmp(argv[n], "-bt") == 0 && argc > n + 1)
		{
			mBurnTime = atof(argv[++n]);
			LINFO("Setting burn time to " << mBurnTime);
		}
		else{
			LINFO("Unrecognized command line argument " << argv[n]);
		}
	}
}

int main(int argc, char* argv[])
{
	gEngine = new sgct::Engine(argc, argv);

	parseArguments(argc, argv);

	gEngine->setDrawFunction(drawFun);
	gEngine->setInitOGLFunction(initFun);
	gEngine->setPreSyncFunction(preSyncFun);
	gEngine->setPostSyncPreDrawFunction(postSyncPreDraw);
	gEngine->setPostDrawFunction(postDrawCallBack);
	gEngine->setCleanUpFunction(cleanUpFun);
	gEngine->setKeyboardCallbackFunction(keyCallback);

	/* Init sgct engine	*/
	if (!gEngine->init(sgct::Engine::OpenGL_4_3_Core_Profile))
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	sgct::SharedData::instance()->setCompression(true);
	sgct::SharedData::instance()->setEncodeFunction(encodeFun);
	sgct::SharedData::instance()->setDecodeFunction(decodeFun);

	//start rendering
	gEngine->render();

	// Clean up engine
	delete gEngine;

	// Exit program
	exit(EXIT_SUCCESS);
}

void initFun()
{
	//init random engine
	//use rand() function to avoid the need for synchronizing fireworks positions etc over network
	srand(1000);

	gEngine->setNearAndFarClippingPlanes(0.01f, 100.f);
	gEngine->setClearColor(0.f, 0.f, 0.f, 0.f);

	mParticleShaderProgram = new ShaderProgram();
	mParticleShaderProgram->attachShader(new Shader(GL_GEOMETRY_SHADER, "../shaders/point.gs"));
	mParticleShaderProgram->attachShader(new Shader(GL_VERTEX_SHADER, "../shaders/point.vs"));
	mParticleShaderProgram->attachShader(new Shader(GL_FRAGMENT_SHADER, "../shaders/point.fs"));
	mParticleShaderProgram->compileAndLinkProgram();

    mLogoShaderProgram = new ShaderProgram;
    mLogoShaderProgram->attachShader(new Shader(GL_VERTEX_SHADER, "../shaders/logo.vs"));
    mLogoShaderProgram->attachShader(new Shader(GL_FRAGMENT_SHADER, "../shaders/logo.fs"));
    mLogoShaderProgram->compileAndLinkProgram();

	sgct::TextureManager::instance()->loadTexure("pointTex", "../resources/halo.png", true);
	sgct::TextureManager::instance()->loadTexure("glareTex", "../resources/glare.png", true);
    sgct::TextureManager::instance()->loadTexure("logoTex", "../resources/logo.png", true);

	//left accumulation texture, also used as mono texture
	glGenTextures(1, &mLeftAccumTex);
	glBindTexture(GL_TEXTURE_2D, mLeftAccumTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gEngine->getWindowPtr(0)->getXFramebufferResolution(), gEngine->getWindowPtr(0)->getYFramebufferResolution(), 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

    //	LDEBUG("Created left eye accumulation texture of resolution " << gEngine->getWindowPtr(0)->getXFramebufferResolution() << "x" << gEngine->getWindowPtr(0)->getYFramebufferResolution());

	//check if we're running stereo
	sgct::SGCTWindow::StereoMode sm = gEngine->getWindowPtr(0)->getStereoMode();
	//if we're running stereo, also create right accumulation texture
	if (sm > sgct::SGCTWindow::StereoMode::No_Stereo){
		glGenTextures(1, &mRightAccumTex);
		glBindTexture(GL_TEXTURE_2D, mRightAccumTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gEngine->getWindowPtr(0)->getXFramebufferResolution(), gEngine->getWindowPtr(0)->getYFramebufferResolution(), 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    // logo quad
    {
        const GLfloat size = 4.f;
        const GLfloat height = 17.f;
        const GLfloat vertexPositionData[] = {
            -size, height, size,
            -size, height, -size,
            size, height, size,
            size, height, -size
        };

        const GLfloat vertexTexCoordData[] = {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        glGenVertexArrays(1, &mLogoQuadVertexArray);
        glBindVertexArray(mLogoQuadVertexArray);

        GLuint vertexPositionBuffer;
        glGenBuffers(1, &vertexPositionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionData), vertexPositionData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            reinterpret_cast<void*>(0) // array buffer offset
            );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLuint vertexTexcoordBuffer;
        glGenBuffers(1, &vertexTexcoordBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexTexcoordBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoordData), vertexTexCoordData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            reinterpret_cast<void*>(0) // array buffer offset
            );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

	//screen quad
	const GLfloat vertexPositionData[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};
	const GLfloat vertexTexCoordData[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	//create a screen quad for accumulation rendering
	glGenVertexArrays(1, &mScreenQuadVertexArray);
	glBindVertexArray(mScreenQuadVertexArray);

	GLuint vertexPositionBuffer;
	glGenBuffers(1, &vertexPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositionData), vertexPositionData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		reinterpret_cast<void*>(0) // array buffer offset
		);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint vertexTexcoordBuffer;
	glGenBuffers(1, &vertexTexcoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTexcoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoordData), vertexTexCoordData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		reinterpret_cast<void*>(0) // array buffer offset
		);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mScreenQuadProgram = new ShaderProgram();
	mScreenQuadProgram->attachShader(new Shader(GL_VERTEX_SHADER, "../shaders/screen.vs"));
	mScreenQuadProgram->attachShader(new Shader(GL_FRAGMENT_SHADER, "../shaders/screen.fs"));
	mScreenQuadProgram->compileAndLinkProgram();

#if defined(_DEBUG)
	gEngine->checkForOGLErrors();
#endif

}

void preSyncFun()
{
	if (gEngine->isMaster())
	{
		gDt.setVal(static_cast<float>(gEngine->getAvgDt()));
	}
}

void encodeFun()
{
	sgct::SharedData::instance()->writeBool(&gShowInfo);
	sgct::SharedData::instance()->writeBool(&gShowStats);
	sgct::SharedData::instance()->writeFloat(&gDt);
    sgct::SharedData::instance()->writeBool(&gAddFireworks);
}

void decodeFun()
{
	sgct::SharedData::instance()->readBool(&gShowInfo);
	sgct::SharedData::instance()->readBool(&gShowStats);
	sgct::SharedData::instance()->readFloat(&gDt);
    sgct::SharedData::instance()->readBool(&gAddFireworks);
}

void postSyncPreDraw()
{
	gEngine->setDisplayInfoVisibility(gShowInfo.getVal());
	gEngine->setStatsGraphVisibility(gShowStats.getVal());

    if (gAddFireworks.getVal()) {
        for (int i = 0; i < 5; ++i) {
            ParticleSystem* p = new ParticleSystem();
            p->setSpeed(5.f * mSpeedScale, 10.f * mSpeedScale, 5.f * mSpeedScale);
            p->setUseParentColor(mUseParentColor);
            p->setLaunchtime(mLaunchTime);
            p->setBurntime(mBurnTime);
            p->initialize();
            mFireworks.push_back(p);
        }
        gAddFireworks.setVal(false);
    }

	//update all fireworks
	for (std::vector<ParticleSystem*>::iterator it = mFireworks.begin();
		it != mFireworks.end();
		++it){
		(*it)->update(gDt.getVal());
	}

	//check if any fireworks has fallen out of scope and remove them if so
	mFireworks.erase(std::remove_if(mFireworks.begin(), mFireworks.end(), [](ParticleSystem* ps){
		return !ps->isAlive();
	}), mFireworks.end());

}

void drawFun()
{
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	//blending for previous frame
	glBlendFunc(GL_ONE, GL_ZERO);

	glUseProgram(mScreenQuadProgram->handle());

	glActiveTexture(GL_TEXTURE0);

	//which eye are we drawing?
	sgct_core::Frustum::FrustumMode fm = gEngine->getCurrentFrustumMode();

	//if right eye
	if (fm == sgct_core::Frustum::StereoRightEye){
		glBindTexture(GL_TEXTURE_2D, mRightAccumTex);
	}
	else{ //otherwise mono or left
		glBindTexture(GL_TEXTURE_2D, mLeftAccumTex);
	}

	glUniform1i(mScreenQuadProgram->getUniformLocation("prevFrame"), 0);
	glBindVertexArray(mScreenQuadVertexArray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);

	/*glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);*/

	//blending for fireworks
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 M, V, P;
	M = gEngine->getModelMatrix();
	V = gEngine->getCurrentViewMatrix();
	P = gEngine->getCurrentProjectionMatrix();

	glUseProgram(mParticleShaderProgram->handle());

	glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("M"), 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("V"), 1, GL_FALSE, glm::value_ptr(V));
	glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("P"), 1, GL_FALSE, glm::value_ptr(P));
	glUniform1i(mParticleShaderProgram->getUniformLocation("Tex"), 0);

	glActiveTexture(GL_TEXTURE0);
	for (std::vector<ParticleSystem*>::iterator it = mFireworks.begin();
		it != mFireworks.end();
		++it){

		if ((*it)->isExploded()){
			glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("glareTex"));
			glUniform1f(mParticleShaderProgram->getUniformLocation("spriteSize"), 0.3f * mAESizeScale);
			glUniform1f(mParticleShaderProgram->getUniformLocation("scale"), (*it)->ttlScale());
		}
		else{
			glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("pointTex"));
			glUniform1f(mParticleShaderProgram->getUniformLocation("spriteSize"), 1.5f * mBESizeScale);
			glUniform1f(mParticleShaderProgram->getUniformLocation("scale"), 1.f);
		}

		(*it)->draw();
	}
	glUseProgram(0);

	glDisable(GL_DEPTH_TEST);

    // Render logo
    //glBlendFunc(GL_ONE, GL_ONE);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(mLogoShaderProgram->handle());
    glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("M"), 1, GL_FALSE, glm::value_ptr(M));
    glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("V"), 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(mParticleShaderProgram->getUniformLocation("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniform1i(mParticleShaderProgram->getUniformLocation("Tex"), 0);
    glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("logoTex"));

    glBindVertexArray(mLogoQuadVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

	//only check OGL errors in debug
#if defined(_DEBUG)
	gEngine->checkForOGLErrors();
#endif

}

void postDrawCallBack(){

	//copy left eye texture (also used as mono texture)
	glCopyImageSubData(
		gEngine->getCurrentWindowPtr()->getFrameBufferTexture(sgct::Engine::TextureIndexes::LeftEye),	//source
		GL_TEXTURE_2D,																					//source target
		0,																								//source level
		0, 0, 0,																						//lower left coordinate to start copy
		mLeftAccumTex,																					//destination
		GL_TEXTURE_2D,																					//destination target
		0,																								//destination level
		0, 0, 0,																						//lower left coordinate of destination
		gEngine->getWindowPtr(0)->getXFramebufferResolution(),											//width of region to copy
		gEngine->getWindowPtr(0)->getYFramebufferResolution(),											//height of region to copy
		1																								//depth of region to copy
		);

    //LDEBUG("Copied left eye texture from texture index " << gEngine->getCurrentWindowPtr()->getFrameBufferTexture(sgct::Engine::TextureIndexes::LeftEye));

	//if we're running stereo, also copy right eye texture
	sgct::SGCTWindow::StereoMode sm = gEngine->getWindowPtr(0)->getStereoMode();
	if (sm > sgct::SGCTWindow::StereoMode::No_Stereo){
		glCopyImageSubData(
			gEngine->getCurrentWindowPtr()->getFrameBufferTexture(sgct::Engine::TextureIndexes::RightEye),	//source
			GL_TEXTURE_2D,																					//source target
			0,																								//source level
			0, 0, 0,																						//lower left coordinate to start copy
			mRightAccumTex,																					//destination
			GL_TEXTURE_2D,																					//destination target
			0,																								//destination level
			0, 0, 0,																						//lower left coordinate of destination
			gEngine->getWindowPtr(0)->getXFramebufferResolution(),											//width of region to copy
			gEngine->getWindowPtr(0)->getYFramebufferResolution(),											//height of region to copy
			1																								//depth of region to copy
			);

        //LDEBUG("Copied right eye texture from texture index " << gEngine->getCurrentWindowPtr()->getFrameBufferTexture(sgct::Engine::TextureIndexes::RightEye));
	}

#if defined(_DEBUG)
	gEngine->checkForOGLErrors();
#endif
}

void cleanUpFun()
{
	delete mParticleShaderProgram;
	delete mScreenQuadProgram;
    delete mLogoShaderProgram;

	if (mLeftAccumTex != GL_FALSE){
		glDeleteTextures(1, &mLeftAccumTex);
	}

	if (mRightAccumTex != GL_FALSE){
		glDeleteTextures(1, &mRightAccumTex);
	}

	if (mScreenQuadVertexArray != GL_FALSE){
		glDeleteVertexArrays(1, &mScreenQuadVertexArray);
	}

    if (mLogoQuadVertexArray != GL_FALSE) {
        glDeleteVertexArrays(1, &mLogoQuadVertexArray);
    }

	for (std::vector<ParticleSystem*>::iterator it = mFireworks.begin();
		it != mFireworks.end();
		++it){
		delete (*it);
	}

	mFireworks.clear();
}

void keyCallback(int key, int scancode, int action, int mods){

	bool shiftKey = ((mods & SGCT_MOD_SHIFT) == SGCT_MOD_SHIFT);
	bool controlKey = ((mods & SGCT_MOD_CONTROL) == SGCT_MOD_CONTROL);
	bool altKey = ((mods & SGCT_MOD_ALT) == SGCT_MOD_ALT);
	bool superKey = ((mods & SGCT_MOD_SUPER) == SGCT_MOD_SUPER);

	switch (key){
	case SGCT_KEY_W:
		if (action == SGCT_PRESS){

		}
		else if (action == SGCT_REPEAT){

		}
		else{	//action == SGCT_RELEASE

		}
		break;
	case SGCT_KEY_I:
		if (action == SGCT_PRESS){
			if (controlKey){
				gShowInfo.toggle();
			}
			else{
				mParticleShaderProgram->compileAndLinkProgram();
				mScreenQuadProgram->compileAndLinkProgram();
			}
		}
		break;
	case SGCT_KEY_S:
		if (action == SGCT_PRESS){
			if (controlKey){
				gShowStats.toggle();
			}
			else{

			}
		}
		else if (action == SGCT_REPEAT){

		}
		else{	//action == SGCT_RELEASE

		}
		break;
	case SGCT_KEY_SPACE:
		if (action == SGCT_PRESS || action == SGCT_REPEAT){
            gAddFireworks.setVal(true);
/*			ParticleSystem* p = new ParticleSystem();
			p->setSpeed(5.f * mSpeedScale, 10.f * mSpeedScale, 5.f * mSpeedScale);
			p->setUseParentColor(mUseParentColor);
			p->setLaunchtime(mLaunchTime);
			p->setBurntime(mBurnTime);
			p->initialize();
			mFireworks.push_back(p);
*/		}

		else{	//action == SGCT_RELEASE

		}
		break;
	default:
		break;
	}
}
