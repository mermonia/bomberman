#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"

#define ENEMY_PATROL_SPEED 1.25f

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
	virtual glm::ivec2 getSize() const { return glm::ivec2(16, 16); }
	virtual void die();
	virtual void hit();
	virtual bool hitsPlayer(const glm::vec2 &playerPos, const glm::vec2 &playerSize) const;
	bool isDying() const { return bDying; }
	bool isDead() const { return bDead; }

protected:
	int moveStep(int deltaTime, float tilesPerSecond);
	int moveStep(int deltaTime, float tilesPerSecond, float &remainder);

protected:
	glm::ivec2 tileMapDispl, posEnemy;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	EnemyDirection direction;
	bool bDying;
	bool bDead;
	int dyingTime;
	float moveRemainder;
};

#endif
