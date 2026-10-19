#ifndef _ENEMY2_INCLUDE
#define _ENEMY2_INCLUDE

#include <cmath>
#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"

enum Enemy2Anims
{
	ENEMY2_WALK,
	ENEMY2_CHASE,
	ENEMY2_DIE
};

class Enemy2 : public Enemy
{
public:
	Enemy2() : Enemy()
	{
		player = NULL;
		bChasing = false;
		chaseCooldown = 0;
	}

	virtual ~Enemy2()
	{
	}

	void setPlayer(Player *p)
	{
		player = p;
	}

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
	{
		spritesheet.loadFromFile("images/enemy2.png", TEXTURE_PIXEL_FORMAT_RGBA);
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

		// Animació de patrulla normal (Fila 0: columnes 0, 1 i 2)
		sprite->setAnimationSpeed(ENEMY2_WALK, 5);
		sprite->addKeyframe(ENEMY2_WALK, glm::vec2(0.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY2_WALK, glm::vec2(1.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY2_WALK, glm::vec2(2.f * stepX, 0.f * stepY));

		// Animació de persecució ràpida (Fila 1: columnes 0, 1 i 2)
		sprite->setAnimationSpeed(ENEMY2_CHASE, 10);
		sprite->addKeyframe(ENEMY2_CHASE, glm::vec2(0.f * stepX, 1.f * stepY));
		sprite->addKeyframe(ENEMY2_CHASE, glm::vec2(1.f * stepX, 1.f * stepY));
		sprite->addKeyframe(ENEMY2_CHASE, glm::vec2(2.f * stepX, 1.f * stepY));

		// Animació de mort (Columna 3: fila 0 i fila 1)
		sprite->setAnimationSpeed(ENEMY2_DIE, 4);
		sprite->addKeyframe(ENEMY2_DIE, glm::vec2(3.f * stepX, 0.f * stepY));
		sprite->addKeyframe(ENEMY2_DIE, glm::vec2(3.f * stepX, 1.f * stepY));

		direction = MOVE_DIR_RIGHT;
		bChasing = false;
		chaseCooldown = 0;
		sprite->changeAnimation(ENEMY2_WALK);

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

		// Detectar si el jugador és dins d'un radi de 3 blocs (3 * 16 = 48 px)
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
				// Només s'activa si no està en cooldown, dins del radi de 3 blocs i alçada similar
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
				// En persecució:
				// 1. Si el jugador s'ha allunyat més enllà del radi de 3 blocs, deixa de perseguir
				if(dist > radius)
				{
					bChasing = false;
				}
				// 2. Si el jugador ha saltat o esquivat l'enemic i ara és al darrere:
				// no es gira immediatament, sinó que continua el seu recorregut cap endavant!
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

		// Canviar animació segons si persegueix o patrulla
		int targetAnim = bChasing ? ENEMY2_CHASE : ENEMY2_WALK;
		if(sprite->animation() != targetAnim)
			sprite->changeAnimation(targetAnim);

		// Gravetat
		posEnemy.y += 4;
		bool onGround = (map != NULL) && map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y);

		if(onGround && map != NULL)
		{
			// Velocitats:
			// En persecució va més ràpid (1 píxel cada 12 ms, ~83 px/segon)
			// En patrulla va més lent (1 píxel cada 26 ms, ~38 px/segon)
			int stepTime = bChasing ? 12 : 26;
			moveTimer += deltaTime;

			while(moveTimer >= stepTime)
			{
				moveTimer -= stepTime;

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
							// Si topa amb una paret o precipici i no pot matar el jugador,
							// abandona la persecució i continua el seu recorregut en la direcció contrària
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
							// Si topa amb una paret o precipici i no pot matar el jugador,
							// abandona la persecució i continua el seu recorregut en la direcció contrària
							bChasing = false;
							chaseCooldown = 1500;
						}
						break;
					}
				}
			}
		}

		// Orientació visual del sprite:
		// Els fotogrames de patrulla (fila 0 de enemy2.png) miren cap a l'esquerra per defecte.
		// Els fotogrames de persecució (fila 1 de enemy2.png) miren cap a la dreta per defecte.
		// Per tant, ajustem el flip per mirar sempre cap a on avança:
		bool bFlip = bChasing ? (direction == MOVE_DIR_LEFT) : (direction == MOVE_DIR_RIGHT);
		sprite->setFlipX(bFlip);

		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
	}

	virtual void die()
	{
		if(bDying)
			return;
		bDying = true;
		bDead = false;
		dyingTime = 0;
		if(sprite != NULL)
			sprite->setFlipX(false);
		sprite->changeAnimation(ENEMY2_DIE);
	}

private:
	Player *player;
	bool bChasing;
	int chaseCooldown;
};

#endif // _ENEMY2_INCLUDE
