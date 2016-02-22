#ifndef _PARTICLE_
#define _PARTICLE_

#include <glm/glm.hpp>

class Particle{
public:
	//Constructor with initializer list
	Particle() :
		mPosition(0.f),
		mSpeed(0.f),
		mColor(0.f),
		mLife(0.f)
	{

	};
	
	Particle(const glm::vec3 &pos, const glm::vec3 &speed, const glm::vec3 &color, float lifespan);

	void update(float dt);
	glm::vec3 & position();
	glm::vec3 & speed();
	glm::vec3 & color();
	float & life();
	const bool isAlive() const;
protected:
private:
	glm::vec3 mPosition;
	glm::vec3 mSpeed;
	glm::vec3 mColor;
	float mLife;
};

#endif