#include "../include/Logger.hpp"
#include "../include/Particle.hpp"


void Particle::update(float dt){
	mLife -= dt;
	if (mLife < 0.f){
		return;
	}

	float ddt = dt;// dt * 0.1f;
	//only gravity at the moment
	mSpeed.y -= 9.81f * ddt * .5f;


	mPosition.x += mSpeed.x * ddt;
	mPosition.y += mSpeed.y * ddt;
	mPosition.z += mSpeed.z * ddt;

	//mPosition.y = fmaxf(0.f, mPosition.y);

	//LINFO(mSpeed.y);
}

Particle::Particle(const glm::vec3 &pos, const glm::vec3 &speed, const glm::vec3 &color, float lifespan) : Particle(){
	mPosition.x = pos.x;
	mPosition.y = pos.y;
	mPosition.z = pos.z;

	mSpeed.x = speed.x;
	mSpeed.y = speed.y;
	mSpeed.z = speed.z;

	mColor.x = mColor.x;
	mColor.y = mColor.y;
	mColor.z = mColor.z;

	mLife = lifespan;
}

glm::vec3 & Particle::position(){
	return mPosition;
}

glm::vec3 & Particle::speed(){
	return mSpeed;
}

const bool Particle::isAlive() const{
	return mLife > 0.f;
}

glm::vec3 & Particle::color(){
	return mColor;
}

float & Particle::life(){
	return mLife;
}