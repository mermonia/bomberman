#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"
#include "ShaderProgram.h"

enum BombState
{
	BOMB_TICKING,
	BOMB_EXPLODING,
	BOMB_FINISHED
};

const int BOMB_TICK_TIME = 2200;
const int BOMB_EXPLOSION_DURATION = 250;
const int BOMB_EXPLOSION_FPS = 12;

class Bomb
{
public:
	Bomb(const glm::ivec2 &tilePos, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram *program, TileMap *tileMap, int flameRadius = 1);
	~Bomb();
	void update(int deltaTime);
	void render() const;

	BombState getState() const { return state; }
	bool isExploding() const { return state == BOMB_EXPLODING; }
	bool isSolid() const { return bSolid; }
	void setSolid(bool solid) { bSolid = solid; }
	bool isFinished() const { return state == BOMB_FINISHED; }
	glm::ivec2 getPosition() const { return glm::ivec2(int(pos.x), int(pos.y)); }
	glm::ivec2 getTileCoords() const { return tileCoords; }

	std::vector<glm::ivec2> getExplosionTiles() const;

private:
	BombState state;
	int timer;
	int radius;
	bool bSolid;
	glm::ivec2 tileCoords;
	glm::ivec2 tileMapDispl;
	glm::vec2 pos;
	TileMap *map;

	Sprite *spriteBomb;
	Sprite *spriteExplosionCenter;
	Sprite *spriteExplosionBody;
	Sprite *spriteExplosionTip;

	bool bExplodeCenter;
	bool bExplodeLeft;
	bool bExplodeRight;
	bool bExplodeUp;
	bool bExplodeDown;
};

#endif
