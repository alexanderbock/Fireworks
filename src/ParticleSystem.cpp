#include "../include/Logger.hpp"
#include "../include/ParticleSystem.hpp"
#include <glm/gtc/constants.hpp>


ParticleSystem::~ParticleSystem(){
	if (mVertexArray != GL_FALSE){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	if (mPositionBuffer != GL_FALSE){
		glDeleteBuffers(1, &mPositionBuffer);
	}
	if (mColorBuffer != GL_FALSE){
		glDeleteBuffers(1, &mColorBuffer);
	}
}

void ParticleSystem::update(float dt){

	mLife += dt;

	if (mLife > mBurntime + mLaunchtime){
		return;
	}

	if (!mHasExploded && (mParticle.speed().y < 0.f || mParticle.life() < 0.f)){
		mHasExploded = true;
		initExplosion();
	}

	if (mHasExploded){
		std::vector<float> pos;
		std::vector<float> col;
		pos.reserve(mParticles.size() * 3);
		col.reserve(mParticles.size() * 3);
		for (std::vector<Particle>::iterator it = mParticles.begin();
			it != mParticles.end();
			++it){
			it->update(dt);
			pos.insert(pos.end(), mParticle.position().x + it->position().x);
			pos.insert(pos.end(), mParticle.position().y + it->position().y);
			pos.insert(pos.end(), mParticle.position().z + it->position().z);
	
			col.insert(col.end(), it->color().x);
			col.insert(col.end(), it->color().y);
			col.insert(col.end(), it->color().z);
		}

		glBindVertexArray(mVertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
		glBufferData(GL_ARRAY_BUFFER, mParticles.size() * 3 * sizeof(float), pos.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	else{
		mParticle.update(dt);
		std::vector<float> pos;
		
		pos.push_back(mParticle.position().x);
		pos.push_back(mParticle.position().y);
		pos.push_back(mParticle.position().z);

		glBindVertexArray(mVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), pos.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}
}

void ParticleSystem::draw(){
	glBindVertexArray(mVertexArray);
	if (mHasExploded){
		glDrawArrays(GL_POINTS, 0, mParticles.size());
	}
	else{
		glDrawArrays(GL_POINTS, 0, 1);
	}
	glBindVertexArray(0);
}


void ParticleSystem::initExplosion(){
	
	mLife = mLaunchtime;

	mParticles.reserve(mMaxParticles);

	std::vector<float> col;
	float sx, sy, sz, r, g, b, px, py, pz;
	for (int n = 0; n < mMaxParticles; ++n){
		sx = .5f * mSpeedX * (-1.f + 2.f * float(rand()) / float(RAND_MAX));
		sy = .5f * mSpeedY * (-1.f + 2.f * float(rand()) / float(RAND_MAX));
		sz = .5f * mSpeedZ * (-1.f + 2.f * float(rand()) / float(RAND_MAX));

		if (mUseParentColor){
			r = mParticle.color().r;
			g = mParticle.color().g;
			b = mParticle.color().b;
		}
		else{
			r = float(rand()) / float(RAND_MAX);
			g = float(rand()) / float(RAND_MAX);
			b = float(rand()) / float(RAND_MAX);
		}

		col.push_back(r);
		col.push_back(g);
		col.push_back(b);

		mParticles.insert(mParticles.end(), { glm::vec3(0.f), glm::vec3(sx, sy, sz), glm::vec3(r, g, b), static_cast<float>(mBurntime) });
	}

	
	glBindVertexArray(mVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(float), col.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void ParticleSystem::initialize(){

	std::vector<float> col;
	std::vector<float> pos;

	
	float sx, sy, sz, r, g, b, px, py, pz;
	
	sx = mSpeedX * (-1.f + 2.f * float(rand()) / float(RAND_MAX));
	sy = mSpeedY * (1.f + 0.5f * float(rand()) / float(RAND_MAX));
	sz = mSpeedZ * (-1.f + 2.f * float(rand()) / float(RAND_MAX));

	r = float(rand()) / float(RAND_MAX);
	g = float(rand()) / float(RAND_MAX);
	b = float(rand()) / float(RAND_MAX);

	
	//randomize position of launch
	float angle = 2.f * glm::pi<float>() * float(rand()) / float(RAND_MAX);

	px = mLaunchRadius * glm::cos(angle);
	py = 0.f;
	pz = mLaunchRadius * glm::sin(angle);

	//make launch direction towards center of dome
	sx = -static_cast<float>((px < 0) - (0 < px)) * mSpeedX * float(rand()) / float(RAND_MAX);
	sz = -static_cast<float>((pz < 0) - (0 < pz)) * mSpeedZ * float(rand()) / float(RAND_MAX);

    if (pz > 0.f) {
        sz *= -std::fmax(1.f, glm::sin(angle) * 1.5f) ;
    }

	col.push_back(r);
	col.push_back(g);
	col.push_back(b);

	pos.push_back(px);
	pos.push_back(py);
	pos.push_back(pz);

	mParticle.color().r = r;
	mParticle.color().g = g;
	mParticle.color().b = b;

	mParticle.position().x = px;
	mParticle.position().y = py;
	mParticle.position().z = pz;

	mParticle.speed().x = sx;
	mParticle.speed().y = sy;
	mParticle.speed().z = sz;

	mParticle.life() = mLaunchtime;

	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);

	glGenBuffers(1, &mPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), pos.data(), GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
		);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &mColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), col.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
		);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
