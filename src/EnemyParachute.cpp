#include <GL/glew.h>
#include "EnemyParachute.h"
#include "AssetPath.h"

#define FALL_STEP 4
#define PARACHUTE_DESCENT_SPEED 1.5f
#define DRIFT_STEP_TIME 120

enum EnemyParachuteAnims
{
	ENEMY_PARACHUTE_FLOAT, ENEMY_PARACHUTE_WALK, ENEMY_PARACHUTE_DIE
};


EnemyParachute::EnemyParachute() : Enemy()
{
	bLanded = false;
	driftTimer = 0;
}

EnemyParachute::~EnemyParachute()
{
}

void EnemyParachute::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile(assetPath("images/e_parachute.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 64.f;
	float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 64.f;

	float stepX = 16.f / texW;
	float stepY = 16.f / texH;

	glm::vec2 sizeInSpritesheet(stepX, stepY);
	sprite = Sprite::createSprite(glm::vec2(16, 16), sizeInSpritesheet, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(3);

	sprite->setAnimationSpeed(ENEMY_PARACHUTE_FLOAT, 4);
	sprite->addKeyframe(ENEMY_PARACHUTE_FLOAT, glm::vec2(0.f * stepX, 0.f * stepY));
	sprite->addKeyframe(ENEMY_PARACHUTE_FLOAT, glm::vec2(1.f * stepX, 0.f * stepY));
	sprite->addKeyframe(ENEMY_PARACHUTE_FLOAT, glm::vec2(0.f * stepX, 0.f * stepY));
	sprite->addKeyframe(ENEMY_PARACHUTE_FLOAT, glm::vec2(2.f * stepX, 0.f * stepY));

	sprite->setAnimationSpeed(ENEMY_PARACHUTE_WALK, 4);
	sprite->addKeyframe(ENEMY_PARACHUTE_WALK, glm::vec2(3.f * stepX, 0.f * stepY));

	sprite->setAnimationSpeed(ENEMY_PARACHUTE_DIE, 6);
	sprite->addKeyframe(ENEMY_PARACHUTE_DIE, glm::vec2(0.f * stepX, 1.f * stepY));
	sprite->addKeyframe(ENEMY_PARACHUTE_DIE, glm::vec2(1.f * stepX, 1.f * stepY));

	direction = MOVE_DIR_RIGHT;
	bLanded = false;
	driftTimer = 0;
	sprite->changeAnimation(ENEMY_PARACHUTE_FLOAT);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyParachute::update(int deltaTime)
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

	if(map != NULL)
	{
		if(bLanded)
			updatePatrol(deltaTime);
		else
			updateDescent(deltaTime);
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyParachute::updateDescent(int deltaTime)
{
	int steps = moveStep(deltaTime, PARACHUTE_DESCENT_SPEED);

	while(steps-- > 0)
	{
		posEnemy.y += 1;

		if(map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y))
		{
			bLanded = true;
			moveRemainder = 0.f;
			driftTimer = 0;
			sprite->changeAnimation(ENEMY_PARACHUTE_WALK);
			return;
		}
	}

	driftTimer += deltaTime;

	while(driftTimer >= DRIFT_STEP_TIME)
	{
		driftTimer -= DRIFT_STEP_TIME;

		if(direction == MOVE_DIR_RIGHT)
		{
			posEnemy.x += 1;

			if(map->collisionMoveRight(posEnemy, glm::ivec2(16, 16)))
			{
				posEnemy.x -= 1;
				direction = MOVE_DIR_LEFT;
				break;
			}
		}
		else if(direction == MOVE_DIR_LEFT)
		{
			posEnemy.x -= 1;

			if(map->collisionMoveLeft(posEnemy, glm::ivec2(16, 16)))
			{
				posEnemy.x += 1;
				direction = MOVE_DIR_RIGHT;
				break;
			}
		}
	}
}

void EnemyParachute::updatePatrol(int deltaTime)
{
	posEnemy.y += FALL_STEP;
	bool onGround = map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y);

	if(!onGround)
		return;

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
				break;
			}
		}
	}
}

void EnemyParachute::die()
{
	if(bDying)
		return;
	bDying = true;
	bDead = false;
	dyingTime = 0;
	sprite->changeAnimation(ENEMY_PARACHUTE_DIE);
}
