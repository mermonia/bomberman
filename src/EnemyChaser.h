#ifndef _ENEMYCHASER_INCLUDE
#define _ENEMYCHASER_INCLUDE

#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"

#define ENEMY_CHASE_SPEED 2.5f

enum EnemyChaserAnims
{
	ENEMY_CHASER_WALK,
	ENEMY_CHASER_CHASE,
	ENEMY_CHASER_DIE
};

class EnemyChaser : public Enemy
{
public:
	EnemyChaser();
	virtual ~EnemyChaser();

	void setPlayer(Player *p);

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void die();

private:
	Player *player;
	bool bChasing;
	int chaseCooldown;
};

#endif
