#include <iostream>
#include <GL/glew.h>
#include "Enemy.h"
#include "AssetPath.h"
#include "AudioManager.h"

#define TILE_SIZE 16
#define FALL_STEP 4

enum EnemyAnims
{
	MOVE_LEFT, MOVE_RIGHT, DIE
};

Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
	direction = MOVE_DIR_RIGHT;
	bDying = false;
	bDead = false;
	dyingTime = 0;
	moveRemainder = 0.f;
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
}

void Enemy::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile(assetPath("images/e_basic.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 253.f;
	float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 632.f;

	glm::vec2 sizeInSpritesheet(16.f / texW, 16.f / texH);
	sprite = Sprite::createSprite(glm::vec2(16, 16), sizeInSpritesheet, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(3);

	sprite->setAnimationSpeed(MOVE_LEFT, 5);
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f * 16.f / texW, 0.f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(1.f * 16.f / texW, 0.f));

	sprite->setAnimationSpeed(MOVE_RIGHT, 5);
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.f * 16.f / texW, 0.f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(1.f * 16.f / texW, 0.f));

	sprite->setAnimationSpeed(DIE, 6);
	sprite->addKeyframe(DIE, glm::vec2(4.f * 16.f / texW, 0.f));
	sprite->addKeyframe(DIE, glm::vec2(5.f * 16.f / texW, 0.f));
	sprite->addKeyframe(DIE, glm::vec2(6.f * 16.f / texW, 0.f));

	direction = MOVE_DIR_RIGHT;
	sprite->changeAnimation(MOVE_RIGHT);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::update(int deltaTime)
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

	posEnemy.y += FALL_STEP;
	bool onGround = (map != NULL) && map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y);

	if(onGround && map != NULL)
	{
		int steps = moveStep(deltaTime, ENEMY_PATROL_SPEED);

		while(steps-- > 0)
		{
			if(direction == MOVE_DIR_RIGHT)
			{
				posEnemy.x += 1;

				bool hitWall = map->collisionMoveRight(posEnemy, glm::ivec2(16, 16));
				int xTileAhead = (posEnemy.x + 16) / map->getTileSize();
				int yTileUnder = (posEnemy.y + 16) / map->getTileSize();
				bool cliffAhead = !map->isGround(xTileAhead, yTileUnder);

				if(hitWall || cliffAhead)
				{
					posEnemy.x -= 1;
					direction = MOVE_DIR_LEFT;
					if(sprite->animation() != MOVE_LEFT)
						sprite->changeAnimation(MOVE_LEFT);
					break;
				}
				else
				{
					if(sprite->animation() != MOVE_RIGHT)
						sprite->changeAnimation(MOVE_RIGHT);
				}
			}
			else if(direction == MOVE_DIR_LEFT)
			{
				posEnemy.x -= 1;

				bool hitWall = map->collisionMoveLeft(posEnemy, glm::ivec2(16, 16));
				int xTileAhead = (posEnemy.x - 1) / map->getTileSize();
				int yTileUnder = (posEnemy.y + 16) / map->getTileSize();
				bool cliffAhead = !map->isGround(xTileAhead, yTileUnder);

				if(hitWall || cliffAhead)
				{
					posEnemy.x += 1;
					direction = MOVE_DIR_RIGHT;
					if(sprite->animation() != MOVE_RIGHT)
						sprite->changeAnimation(MOVE_RIGHT);
					break;
				}
				else
				{
					if(sprite->animation() != MOVE_LEFT)
						sprite->changeAnimation(MOVE_LEFT);
				}
			}
		}
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

int Enemy::moveStep(int deltaTime, float tilesPerSecond)
{
	return moveStep(deltaTime, tilesPerSecond, moveRemainder);
}

int Enemy::moveStep(int deltaTime, float tilesPerSecond, float &remainder)
{
	remainder += tilesPerSecond * TILE_SIZE * deltaTime / 1000.f;
	int step = int(remainder);
	remainder -= float(step);

	return step;
}

void Enemy::render()
{
	sprite->render();
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::die()
{
	if(bDying)
		return;
	bDying = true;
	AudioManager::instance().playSound(SFX_ENEMY_DIE);
	bDead = false;
	dyingTime = 0;
	sprite->changeAnimation(DIE);
}

void Enemy::hit()
{
	die();
}

bool Enemy::hitsPlayer(const glm::vec2 &playerPos, const glm::vec2 &playerSize) const
{
	return false;
}
