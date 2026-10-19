#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"

// Enemy represents a patrolling ground enemy based on Player.
// It moves horizontally until it hits a wall or reaches a cliff (precipice),
// at which point it turns around and moves in the opposite direction.

enum EnemyDirection
{
	MOVE_DIR_LEFT, MOVE_DIR_RIGHT
};

class Enemy
{
public:
	Enemy();
	virtual ~Enemy();

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::ivec2 getPosition() const { return posEnemy; }
	glm::ivec2 getSize() const { return glm::ivec2(16, 16); }
	virtual void die();
	bool isDying() const { return bDying; }
	bool isDead() const { return bDead; }

protected:
	glm::ivec2 tileMapDispl, posEnemy;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	EnemyDirection direction;
	bool bDying;
	bool bDead;
	int dyingTime;
	int moveTimer;
};

#endif // _ENEMY_INCLUDE
