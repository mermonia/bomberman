#include <iostream>
#include <GL/glew.h>
#include "Enemy.h"

#define FALL_STEP 4
#define SPEED 1

enum EnemyAnims
{
	MOVE_LEFT, MOVE_RIGHT
};

Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
	direction = MOVE_DIR_RIGHT;
}

Enemy::~Enemy()
{
	if(sprite != NULL)
		delete sprite;
}

void Enemy::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/enemy.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 253.f;
	float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 632.f;

	// Cada sprite individual d'enemy.png és de 16x16 píxels, representat en un quad de 32x32
	// per mantenir l'escala dels blocs i del personatge (Player).
	glm::vec2 sizeInSpritesheet(16.f / texW, 16.f / texH);
	sprite = Sprite::createSprite(glm::vec2(32, 32), sizeInSpritesheet, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);
	
	// Animació cap a l'esquerra (fotogrames 0 i 1)
	sprite->setAnimationSpeed(MOVE_LEFT, 6);
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f * 16.f / texW, 0.f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(1.f * 16.f / texW, 0.f));
	
	// Animació cap a la dreta (fotogrames 2 i 3)
	sprite->setAnimationSpeed(MOVE_RIGHT, 6);
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.f * 16.f / texW, 0.f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(1.f * 16.f / texW, 0.f));
	
	direction = MOVE_DIR_RIGHT;
	sprite->changeAnimation(MOVE_RIGHT);
	
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::update(int deltaTime)
{
	sprite->update(deltaTime);

	// Gravetat: cau fins a tocar el terra
	posEnemy.y += FALL_STEP;
	bool onGround = map->collisionMoveDown(posEnemy, glm::ivec2(32, 32), &posEnemy.y);

	if(onGround)
	{
		if(direction == MOVE_DIR_RIGHT)
		{
			posEnemy.x += SPEED;
			
			// Comprovar col·lisió amb paret a la dreta
			bool hitWall = map->collisionMoveRight(posEnemy, glm::ivec2(32, 32));
			
			// Comprovar si hi ha precipici a la dreta (el tile sota el peu dret)
			int xTileAhead = (posEnemy.x + 32) / map->getTileSize();
			int yTileUnder = (posEnemy.y + 32) / map->getTileSize();
			bool cliffAhead = !map->isGround(xTileAhead, yTileUnder);

			if(hitWall || cliffAhead)
			{
				posEnemy.x -= SPEED;
				direction = MOVE_DIR_LEFT;
				if(sprite->animation() != MOVE_LEFT)
					sprite->changeAnimation(MOVE_LEFT);
			}
			else
			{
				if(sprite->animation() != MOVE_RIGHT)
					sprite->changeAnimation(MOVE_RIGHT);
			}
		}
		else if(direction == MOVE_DIR_LEFT)
		{
			posEnemy.x -= SPEED;
			
			// Comprovar col·lisió amb paret a l'esquerra
			bool hitWall = map->collisionMoveLeft(posEnemy, glm::ivec2(32, 32));
			
			// Comprovar si hi ha precipici a l'esquerra (el tile sota el peu esquerre)
			int xTileAhead = (posEnemy.x - 1) / map->getTileSize();
			int yTileUnder = (posEnemy.y + 32) / map->getTileSize();
			bool cliffAhead = !map->isGround(xTileAhead, yTileUnder);

			if(hitWall || cliffAhead)
			{
				posEnemy.x += SPEED;
				direction = MOVE_DIR_RIGHT;
				if(sprite->animation() != MOVE_RIGHT)
					sprite->changeAnimation(MOVE_RIGHT);
			}
			else
			{
				if(sprite->animation() != MOVE_LEFT)
					sprite->changeAnimation(MOVE_LEFT);
			}
		}
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
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
