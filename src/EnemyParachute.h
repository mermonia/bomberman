#ifndef _ENEMYPARACHUTE_INCLUDE
#define _ENEMYPARACHUTE_INCLUDE

#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"


enum ParachuteState
{
	PARACHUTE_WALK, PARACHUTE_AIRBORNE, PARACHUTE_GLIDE
};


class EnemyParachute : public Enemy
{
public:
	EnemyParachute();
	virtual ~EnemyParachute();

	void setPlayer(Player *p);

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void die();

private:
	void updateWalk(int deltaTime);
	void updateAirborne(int deltaTime);
	void updateGlide(int deltaTime);
	void jump(float tilesHigh, bool bAttack);
	void land();
	bool onGround() const;
	bool stepVertical(int dy);
	bool stepHorizontal(int dx);
	bool seesPlayerBelow() const;
	bool playerInGlideRange() const;
	int playerCenterX() const;
	void setAnimation(int anim);

private:
	Player *player;
	ParachuteState state;
	bool bAttacking;
	float velocityY;
	float verticalRemainder;
	int hopTimer;
	int attackCooldown;

};


#endif // _ENEMYPARACHUTE_INCLUDE
