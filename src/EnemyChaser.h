#ifndef _ENEMYCHASER_INCLUDE
#define _ENEMYCHASER_INCLUDE

#include <cmath>
#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"
#include "AssetPath.h"
#include "AudioManager.h"

#define ENEMY_CHASE_SPEED 2.5f

enum EnemyChaserAnims
{
	ENEMY_CHASER_WALK,
	ENEMY_CHASER_CHASE,
	ENEMY_CHASER_DIE
};

class EnemyChaser : public Enemy
{
public:
	EnemyChaser() : Enemy()
	{
		player = NULL;
		bChasing = false;
		chaseCooldown = 0;
	}

	virtual ~EnemyChaser()
	{
	}

	void setPlayer(Player *p)
	{
		player = p;
	}

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
	{
		spritesheet.loadFromFile(assetPath("images/e_chaser.png"), TEXTURE_PIXEL_FORMAT_RGBA);
		spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
		spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
		spritesheet.setMinFilter(GL_NEAREST);
		spritesheet.setMagFilter(GL_NEAREST);

		float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 64.f;
		float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 32.f;

		float stepX = 16.f / texW;
		float stepY = 16.f / texH;

		glm::vec2 sizeInSpritesheet(stepX, stepY);
		sprite = Sprite::createSprite(glm::vec2(16, 16), sizeInSpritesheet, &spritesheet, &shaderProgram);
		sprite->setNumberAnimations(3);

		sprite->setAnimationSpeed(ENEMY_CHASER_WALK, 5);
		sprite->addKeyframe(ENEMY_CHASER_WALK, glm::vec2(0.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY_CHASER_WALK, glm::vec2(1.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY_CHASER_WALK, glm::vec2(2.f * stepX, 0.f * stepY));

		sprite->setAnimationSpeed(ENEMY_CHASER_CHASE, 10);
		sprite->addKeyframe(ENEMY_CHASER_CHASE, glm::vec2(0.f * stepX, 1.f * stepY));
		sprite->addKeyframe(ENEMY_CHASER_CHASE, glm::vec2(1.f * stepX, 1.f * stepY));
		sprite->addKeyframe(ENEMY_CHASER_CHASE, glm::vec2(2.f * stepX, 1.f * stepY));

		sprite->setAnimationSpeed(ENEMY_CHASER_DIE, 4);
		sprite->addKeyframe(ENEMY_CHASER_DIE, glm::vec2(3.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY_CHASER_DIE, glm::vec2(3.f * stepX, 1.f * stepY));

		direction = MOVE_DIR_RIGHT;
		bChasing = false;
		chaseCooldown = 0;
		sprite->changeAnimation(ENEMY_CHASER_WALK);

		tileMapDispl = tileMapPos;
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
	}

	virtual void update(int deltaTime)
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

		if(chaseCooldown > 0)
			chaseCooldown -= deltaTime;

		int blockSize = (map != NULL) ? map->getTileSize() : 16;
		float radius = 3.f * float(blockSize);

		if(player != NULL && !player->isDying())
		{
			glm::vec2 pPos = player->getPosition();
			glm::vec2 eCenter = glm::vec2(float(posEnemy.x + 8), float(posEnemy.y + 8));
			glm::vec2 pCenter = glm::vec2(float(pPos.x + 8), float(pPos.y + 8));

			float dist = glm::distance(eCenter, pCenter);

			if(!bChasing)
			{
				if(chaseCooldown <= 0 && dist <= radius && std::abs(eCenter.y - pCenter.y) <= 24.f)
				{
					bChasing = true;
					if(pPos.x >= posEnemy.x)
						direction = MOVE_DIR_RIGHT;
					else
						direction = MOVE_DIR_LEFT;
				}
			}
			else
			{
				if(dist > radius)
				{
					bChasing = false;
				}
				else if((direction == MOVE_DIR_RIGHT && pPos.x < posEnemy.x - 12.f) ||
				        (direction == MOVE_DIR_LEFT && pPos.x > posEnemy.x + 12.f))
				{
					bChasing = false;
					chaseCooldown = 1500;
				}
			}
		}
		else
		{
			bChasing = false;
		}

		int targetAnim = bChasing ? ENEMY_CHASER_CHASE : ENEMY_CHASER_WALK;
		if(sprite->animation() != targetAnim)
			sprite->changeAnimation(targetAnim);

		posEnemy.y += 4;
		bool onGround = (map != NULL) && map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y);

		if(onGround && map != NULL)
		{
			int steps = moveStep(deltaTime, bChasing ? ENEMY_CHASE_SPEED : ENEMY_PATROL_SPEED);

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
						if(bChasing)
						{
							bChasing = false;
							chaseCooldown = 1500;
						}
						break;
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
						if(bChasing)
						{
							bChasing = false;
							chaseCooldown = 1500;
						}
						break;
					}
				}
			}
		}

		bool bFlip = bChasing ? (direction == MOVE_DIR_LEFT) : (direction == MOVE_DIR_RIGHT);
		sprite->setFlipX(bFlip);

		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
	}

	virtual void die()
	{
		if(bDying)
			return;
		bDying = true;
		AudioManager::instance().playSound(SFX_ENEMY_DIE);
		bDead = false;
		dyingTime = 0;
		if(sprite != NULL)
			sprite->setFlipX(false);
		sprite->changeAnimation(ENEMY_CHASER_DIE);
	}

private:
	Player *player;
	bool bChasing;
	int chaseCooldown;
};

#endif
