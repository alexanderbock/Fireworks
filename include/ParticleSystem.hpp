#ifndef _PARTICLE_SYSTEM_
#define _PARTICLE_SYSTEM_

#include "Particle.hpp"
#include <GL/glew.h>
#include <vector>

class ParticleSystem{
public:
	//Constructor with initializer list
	ParticleSystem() :
		mNumParticles(0),
		mMaxParticles(100),
		mVertexArray(GL_FALSE),
		mPositionBuffer(GL_FALSE),
		mColorBuffer(GL_FALSE),
		mParticleShader(GL_FALSE),
		mHasExploded(false),
		mBurntime(2.0),
		mLaunchtime(8.0),
		mLife(0.0),
		mSpeedX(5.f),
		mSpeedY(10.f), 
		mSpeedZ(5.f),
		mLaunchRadius(7.4f),
		mUseParentColor(false)
	{
		
	};
	~ParticleSystem();
	void initialize();
	void update(float dt);
	void draw();
	bool isExploded(){ return mHasExploded; };
	void setBurntime(double d){ mBurntime = d; };
	void setLaunchtime(double d){ mLaunchtime = d; };
	bool isAlive(){
		return mLife < (mBurntime + mLaunchtime);
	};
	void setSpeed(float sx, float sy, float sz){
		mSpeedX = sx;
		mSpeedY = sy;
		mSpeedZ = sz;
	};
	float ttlScale(){
		return 1.f - (mLife - mLaunchtime) / mBurntime;
	};
	void setLaunchRadius(float f){ mLaunchRadius = f; };
	void setUseParentColor(bool b){ mUseParentColor = b; };
protected:
private:
	bool mHasExploded;
	void initExplosion();
	Particle mParticle;

	//particles
	int mNumParticles;
	int mMaxParticles;
	std::vector<Particle> mParticles;
	
	bool mUseParentColor;
	double mLaunchtime, mBurntime, mLife;
	float mSpeedX, mSpeedY, mSpeedZ;
	float mLaunchRadius;

	//OpenGL 
	GLuint mVertexArray;
	GLuint mPositionBuffer;
	GLuint mColorBuffer;
	GLuint mParticleShader;
	std::string mVertexShaderSource;
	std::string mFragmentShaderSource;
	std::string mGeometryShaderSource;
	
};

#endif