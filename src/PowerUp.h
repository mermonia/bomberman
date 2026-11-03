#ifndef _POWERUP_INCLUDE
#define _POWERUP_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "ShaderProgram.h"

enum PowerUpType
{
	POWERUP_BOMB_UP = 0,
	POWERUP_HEART = 1,
	POWERUP_FIRE_UP = 2,
	POWERUP_SPEED_UP = 3
};

class PowerUp
{
public:
	PowerUp(PowerUpType type, const glm::ivec2 &tileCoords, Texture *spritesheet, ShaderProgram *program);
	~PowerUp();
	void update(int deltaTime);
	void render() const;
	bool checkCollision(const glm::vec2 &playerPos, const glm::ivec2 &playerSize) const;

	PowerUpType getType() const { return type; }
	bool isCollected() const { return bCollected; }
	void collect() { bCollected = true; }
	glm::ivec2 getTileCoords() const { return tileCoords; }
	glm::vec2 getPosition() const { return pos; }

private:
	PowerUpType type;
	glm::ivec2 tileCoords;
	glm::vec2 pos;
	bool bCollected;
	int timer;
	Sprite *sprite;
};

#endif
