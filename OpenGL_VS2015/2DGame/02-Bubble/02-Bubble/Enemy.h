#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

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
	~Enemy();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::ivec2 getPosition() const { return posEnemy; }

private:
	glm::ivec2 tileMapDispl, posEnemy;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	EnemyDirection direction;
};

#endif // _ENEMY_INCLUDE
