#ifndef _ENEMYPARACHUTE_INCLUDE
#define _ENEMYPARACHUTE_INCLUDE

#include <glm/glm.hpp>
#include "Enemy.h"


class EnemyParachute : public Enemy
{
public:
	EnemyParachute();
	virtual ~EnemyParachute();

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void die();

private:
	void updateDescent(int deltaTime);
	void updatePatrol(int deltaTime);

private:
	bool bLanded;
	int driftTimer;

};


#endif // _ENEMYPARACHUTE_INCLUDE
