#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const { return glm::vec2(posPlayer); }
	glm::ivec2 getSize() const { return glm::ivec2(16, 16); }
	void die(bool byBomb = false);
	bool isDying() const { return bDying; }
	bool hasHeart() const { return bHasHeart; }
	void giveHeart() { bHasHeart = true; }
	void consumeHeart() { bHasHeart = false; }
	bool isInvulnerable() const { return invulnerableTime > 0; }
	void setInvulnerable(int durationMs = 1500) { invulnerableTime = durationMs; }
	void increaseSpeed(float delta = 0.6f) { walkSpeed += delta; }
	float getSpeed() const { return walkSpeed; }
	
private:
	bool collisionDown(int *posY);
	int walkStep(int deltaTime);
	int fallStep(int deltaTime);

private:
	bool bJumping;
	bool bDying;
	bool bHasHeart;
	int invulnerableTime;
	float walkSpeed;
	int dyingTime, maxDyingTime;
	glm::ivec2 tileMapDispl, posPlayer, spawnPos;
	float jumpAngle, walkRemainder, fallRemainder;
	int startY;
	Texture spritesheet;
	Texture spritesheetDeath;
	Sprite *sprite;
	Sprite *spriteDeath;
	TileMap *map;

};


#endif // _PLAYER_INCLUDE


