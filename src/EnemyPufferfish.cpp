#include <cmath>
#include <GL/glew.h>
#include "EnemyPufferfish.h"
#include "AssetPath.h"
#include "AudioManager.h"

#define TILE_SIZE 16
#define PUFFERFISH_SWIM_SPEED 1.f
#define PUFFERFISH_RETURN_SPEED 1.5f
#define PUFFERFISH_LUNGE_SPEED 0.9f
#define PUFFERFISH_RANGE_TILES 4
#define PUFFERFISH_BOB_PERIOD 1600
#define PUFFERFISH_BOB_AMPLITUDE 4
#define PUFFERFISH_PUFF_RADIUS 64.f
#define PUFFERFISH_GIVE_UP_RADIUS 112.f
#define PUFFERFISH_PUFF_TIME 2200
#define PUFFERFISH_PUFF_COOLDOWN 1500

enum EnemyPufferfishAnims
{
	ENEMY_PUFFERFISH_SWIM,
	ENEMY_PUFFERFISH_PUFF,
	ENEMY_PUFFERFISH_DIE
};

EnemyPufferfish::EnemyPufferfish() : Enemy()
{
	player = NULL;
	bPuffed = false;
	bHomeSet = false;
	homeX = 0;
	baseY = 0;
	bobTimer = 0;
	puffTimer = 0;
	puffCooldown = 0;
	verticalRemainder = 0.f;
}

EnemyPufferfish::~EnemyPufferfish()
{
}

void EnemyPufferfish::setPlayer(Player *p)
{
	player = p;
}

void EnemyPufferfish::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile(assetPath("images/e_pufferfish.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 64.f;
	float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 64.f;

	float stepX = 16.f / texW;
	float stepY = 16.f / texH;

	sprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(3);

	sprite->setAnimationSpeed(ENEMY_PUFFERFISH_SWIM, 4);
	sprite->addKeyframe(ENEMY_PUFFERFISH_SWIM, glm::vec2(0.f * stepX, 0.f * stepY));
	sprite->addKeyframe(ENEMY_PUFFERFISH_SWIM, glm::vec2(1.f * stepX, 0.f * stepY));

	sprite->setAnimationSpeed(ENEMY_PUFFERFISH_PUFF, 6);
	sprite->addKeyframe(ENEMY_PUFFERFISH_PUFF, glm::vec2(2.f * stepX, 0.f * stepY));
	sprite->addKeyframe(ENEMY_PUFFERFISH_PUFF, glm::vec2(3.f * stepX, 0.f * stepY));

	sprite->setAnimationSpeed(ENEMY_PUFFERFISH_DIE, 4);
	sprite->addKeyframe(ENEMY_PUFFERFISH_DIE, glm::vec2(0.f * stepX, 1.f * stepY));

	direction = MOVE_DIR_LEFT;
	bPuffed = false;
	bHomeSet = false;
	bobTimer = 0;
	puffTimer = 0;
	puffCooldown = 0;
	verticalRemainder = 0.f;
	sprite->changeAnimation(ENEMY_PUFFERFISH_SWIM);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

bool EnemyPufferfish::playerIsNear() const
{
	if(player == NULL || player->isDying())
		return false;

	glm::vec2 pPos = player->getPosition();
	glm::vec2 eCenter = glm::vec2(float(posEnemy.x + 8), float(posEnemy.y + 8));
	glm::vec2 pCenter = glm::vec2(pPos.x + 8.f, pPos.y + 8.f);

	return glm::distance(eCenter, pCenter) <= PUFFERFISH_PUFF_RADIUS;
}

bool EnemyPufferfish::stepVertical(int dy)
{
	if(map == NULL)
		return false;

	glm::ivec2 next = posEnemy;
	next.y += dy;

	int probeY = next.y;
	bool blocked;
	if(dy > 0)
		blocked = map->collisionMoveDown(next, glm::ivec2(16, 16), &probeY);
	else
		blocked = map->collisionMoveUp(next, glm::ivec2(16, 16), &probeY);

	if(blocked)
		return false;

	posEnemy.y = next.y;
	return true;
}

bool EnemyPufferfish::stepHorizontal(int dx)
{
	if(map == NULL)
		return false;

	posEnemy.x += dx;
	bool blocked = (dx > 0) ? map->collisionMoveRight(posEnemy, glm::ivec2(16, 16))
	                        : map->collisionMoveLeft(posEnemy, glm::ivec2(16, 16));
	if(blocked)
	{
		posEnemy.x -= dx;
		return false;
	}

	return true;
}

void EnemyPufferfish::moveVerticallyTowards(int targetY, int deltaTime, float tilesPerSecond)
{
	int steps = moveStep(deltaTime, tilesPerSecond, verticalRemainder);

	while(steps-- > 0 && posEnemy.y != targetY)
	{
		if(!stepVertical(posEnemy.y < targetY ? 1 : -1))
			break;
	}
}

void EnemyPufferfish::swim(int deltaTime)
{
	int leftBound = homeX - PUFFERFISH_RANGE_TILES * TILE_SIZE;
	int rightBound = homeX + PUFFERFISH_RANGE_TILES * TILE_SIZE;
	int steps = moveStep(deltaTime, PUFFERFISH_SWIM_SPEED);

	while(steps-- > 0)
	{
		if(direction == MOVE_DIR_RIGHT && posEnemy.x >= rightBound)
			direction = MOVE_DIR_LEFT;
		else if(direction == MOVE_DIR_LEFT && posEnemy.x <= leftBound)
			direction = MOVE_DIR_RIGHT;

		if(!stepHorizontal(direction == MOVE_DIR_RIGHT ? 1 : -1))
		{
			direction = (direction == MOVE_DIR_RIGHT) ? MOVE_DIR_LEFT : MOVE_DIR_RIGHT;
			break;
		}
	}

	bobTimer = (bobTimer + deltaTime) % PUFFERFISH_BOB_PERIOD;
	float phase = 2.f * 3.14159265f * float(bobTimer) / float(PUFFERFISH_BOB_PERIOD);
	int targetY = baseY + int(std::lround(float(PUFFERFISH_BOB_AMPLITUDE) * std::sin(phase)));
	moveVerticallyTowards(targetY, deltaTime, PUFFERFISH_RETURN_SPEED);
}

void EnemyPufferfish::lunge(int deltaTime)
{
	glm::vec2 pPos = player->getPosition();
	int targetX = int(pPos.x);
	int targetY = int(pPos.y);

	direction = (targetX >= posEnemy.x) ? MOVE_DIR_RIGHT : MOVE_DIR_LEFT;

	int steps = moveStep(deltaTime, PUFFERFISH_LUNGE_SPEED);
	while(steps-- > 0 && posEnemy.x != targetX)
	{
		if(!stepHorizontal(posEnemy.x < targetX ? 1 : -1))
			break;
	}

	moveVerticallyTowards(targetY, deltaTime, PUFFERFISH_LUNGE_SPEED);
}

void EnemyPufferfish::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(bDying)
	{
		dyingTime += deltaTime;
		if(dyingTime >= 500)
		{
			bDead = true;
		}
		return;
	}

	if(!bHomeSet)
	{
		homeX = posEnemy.x;
		baseY = posEnemy.y;
		bHomeSet = true;
	}

	if(puffCooldown > 0)
		puffCooldown -= deltaTime;

	if(bPuffed)
	{
		puffTimer -= deltaTime;
		bool bLostPlayer = true;
		if(player != NULL && !player->isDying())
		{
			glm::vec2 pPos = player->getPosition();
			bLostPlayer = glm::distance(glm::vec2(posEnemy), pPos) > PUFFERFISH_GIVE_UP_RADIUS;
		}

		if(puffTimer <= 0 || bLostPlayer)
		{
			bPuffed = false;
			puffCooldown = PUFFERFISH_PUFF_COOLDOWN;
		}
	}
	else if(puffCooldown <= 0 && playerIsNear())
	{
		bPuffed = true;
		puffTimer = PUFFERFISH_PUFF_TIME;
	}

	int targetAnim = bPuffed ? ENEMY_PUFFERFISH_PUFF : ENEMY_PUFFERFISH_SWIM;
	if(sprite->animation() != targetAnim)
		sprite->changeAnimation(targetAnim);

	if(bPuffed)
		lunge(deltaTime);
	else
		swim(deltaTime);

	sprite->setFlipX(direction == MOVE_DIR_RIGHT);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyPufferfish::die()
{
	if(bDying)
		return;
	bDying = true;
	AudioManager::instance().playSound(SFX_ENEMY_DIE);
	bDead = false;
	dyingTime = 0;
	if(sprite != NULL)
	{
		sprite->setFlipX(false);
		sprite->changeAnimation(ENEMY_PUFFERFISH_DIE);
	}
}
