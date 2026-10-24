#ifndef _ENEMYPUFFERFISH_INCLUDE
#define _ENEMYPUFFERFISH_INCLUDE

#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"

class EnemyPufferfish : public Enemy
{
public:
	EnemyPufferfish();
	virtual ~EnemyPufferfish();

	void setPlayer(Player *p);

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void die();

private:
	bool playerIsNear() const;
	bool stepVertical(int dy);
	bool stepHorizontal(int dx);
	void swim(int deltaTime);
	void lunge(int deltaTime);
	void moveVerticallyTowards(int targetY, int deltaTime, float tilesPerSecond);

private:
	Player *player;
	bool bPuffed;
	bool bHomeSet;
	int homeX;
	int baseY;
	int bobTimer;
	int puffTimer;
	int puffCooldown;
	float verticalRemainder;
};

#endif // _ENEMYPUFFERFISH_INCLUDE
