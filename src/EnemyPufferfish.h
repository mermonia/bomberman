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
	void drift(int deltaTime);
	void bob(int deltaTime);

private:
	Player *player;
	bool bPuffed;
	bool bBaseYSet;
	int baseY;
	int bobTimer;
};

#endif // _ENEMYPUFFERFISH_INCLUDE
