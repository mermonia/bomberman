#include <cmath>
#include <GL/glew.h>
#include "EnemyParachute.h"
#include "AssetPath.h"

#define TILE_SIZE 16
#define PARACHUTE_GRAVITY 30.f
#define PARACHUTE_HOP_HEIGHT 0.5f
#define PARACHUTE_ATTACK_HEIGHT 1.5f
#define PARACHUTE_HOP_INTERVAL 1800
#define PARACHUTE_ATTACK_COOLDOWN 1500
#define PARACHUTE_GLIDE_FALL_SPEED 2.f
#define PARACHUTE_GLIDE_STEER_SPEED 2.5f
#define PARACHUTE_GLIDE_DRIFT_SPEED 0.5f
#define PARACHUTE_SIGHT_X_TILES 5
#define PARACHUTE_SIGHT_BELOW_TILES 8

enum EnemyParachuteAnims
{
	ENEMY_PARACHUTE_FLOAT, ENEMY_PARACHUTE_WALK, ENEMY_PARACHUTE_DIE
};


EnemyParachute::EnemyParachute() : Enemy()
{
	player = NULL;
	state = PARACHUTE_AIRBORNE;
	bAttacking = false;
	velocityY = 0.f;
	verticalRemainder = 0.f;
	hopTimer = 0;
	attackCooldown = 0;
}

EnemyParachute::~EnemyParachute()
{
}

void EnemyParachute::setPlayer(Player *p)
{
	player = p;
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
	state = PARACHUTE_AIRBORNE;
	bAttacking = false;
	velocityY = 0.f;
	verticalRemainder = 0.f;
	hopTimer = 0;
	attackCooldown = 0;
	sprite->changeAnimation(ENEMY_PARACHUTE_WALK);

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

	if(attackCooldown > 0)
		attackCooldown -= deltaTime;

	if(map != NULL)
	{
		if(state == PARACHUTE_WALK)
			updateWalk(deltaTime);
		else if(state == PARACHUTE_AIRBORNE)
			updateAirborne(deltaTime);
		else
			updateGlide(deltaTime);
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyParachute::updateWalk(int deltaTime)
{
	if(!onGround())
	{
		state = PARACHUTE_AIRBORNE;
		velocityY = 0.f;
		return;
	}

	if(attackCooldown <= 0 && seesPlayerBelow())
	{
		direction = (playerCenterX() >= posEnemy.x + 8) ? MOVE_DIR_RIGHT : MOVE_DIR_LEFT;
		jump(PARACHUTE_ATTACK_HEIGHT, true);
		return;
	}

	hopTimer += deltaTime;
	if(hopTimer >= PARACHUTE_HOP_INTERVAL)
	{
		jump(PARACHUTE_HOP_HEIGHT, false);
		return;
	}

	int steps = moveStep(deltaTime, ENEMY_PATROL_SPEED);

	while(steps-- > 0)
	{
		int dx = (direction == MOVE_DIR_RIGHT) ? 1 : -1;
		int xTileAhead = (direction == MOVE_DIR_RIGHT) ? (posEnemy.x + 16) / TILE_SIZE : (posEnemy.x - 1) / TILE_SIZE;
		int yTileUnder = (posEnemy.y + 16) / TILE_SIZE;
		bool cliffAhead = !map->isGround(xTileAhead, yTileUnder);

		if(cliffAhead || !stepHorizontal(dx))
		{
			direction = (direction == MOVE_DIR_RIGHT) ? MOVE_DIR_LEFT : MOVE_DIR_RIGHT;
			break;
		}
	}
}

void EnemyParachute::updateAirborne(int deltaTime)
{
	float dt = deltaTime / 1000.f;
	velocityY += PARACHUTE_GRAVITY * TILE_SIZE * dt;

	if(velocityY >= 0.f)
	{
		state = PARACHUTE_GLIDE;
		verticalRemainder = 0.f;
		moveRemainder = 0.f;
		setAnimation(bAttacking ? ENEMY_PARACHUTE_FLOAT : ENEMY_PARACHUTE_WALK);
		return;
	}

	verticalRemainder += velocityY * dt;
	int dy = int(verticalRemainder);
	verticalRemainder -= float(dy);

	if(!stepVertical(dy))
	{
		velocityY = 0.f;
		verticalRemainder = 0.f;
	}
}

void EnemyParachute::updateGlide(int deltaTime)
{
	int fall = moveStep(deltaTime, PARACHUTE_GLIDE_FALL_SPEED, verticalRemainder);
	if(!stepVertical(fall))
	{
		land();
		return;
	}

	bool bSteering = playerInGlideRange();
	if(bSteering && !bAttacking)
	{
		bAttacking = true;
		setAnimation(ENEMY_PARACHUTE_FLOAT);
	}

	int steps = moveStep(deltaTime, bSteering ? PARACHUTE_GLIDE_STEER_SPEED : PARACHUTE_GLIDE_DRIFT_SPEED);

	while(steps-- > 0)
	{
		if(bSteering)
		{
			int offset = playerCenterX() - (posEnemy.x + 8);
			if(offset == 0)
				break;
			direction = (offset > 0) ? MOVE_DIR_RIGHT : MOVE_DIR_LEFT;
		}

		if(!stepHorizontal((direction == MOVE_DIR_RIGHT) ? 1 : -1))
		{
			if(!bSteering)
				direction = (direction == MOVE_DIR_RIGHT) ? MOVE_DIR_LEFT : MOVE_DIR_RIGHT;
			break;
		}
	}
}

void EnemyParachute::jump(float tilesHigh, bool bAttack)
{
	state = PARACHUTE_AIRBORNE;
	bAttacking = bAttack;
	velocityY = -std::sqrt(2.f * PARACHUTE_GRAVITY * tilesHigh) * TILE_SIZE;
	verticalRemainder = 0.f;
	hopTimer = 0;
	setAnimation(ENEMY_PARACHUTE_WALK);
}

void EnemyParachute::land()
{
	if(bAttacking)
		attackCooldown = PARACHUTE_ATTACK_COOLDOWN;
	state = PARACHUTE_WALK;
	bAttacking = false;
	velocityY = 0.f;
	verticalRemainder = 0.f;
	moveRemainder = 0.f;
	hopTimer = 0;
	setAnimation(ENEMY_PARACHUTE_WALK);
}

bool EnemyParachute::onGround() const
{
	glm::ivec2 probe = posEnemy;
	probe.y += 1;
	int probeY = probe.y;
	return map->collisionMoveDown(probe, glm::ivec2(16, 16), &probeY);
}

bool EnemyParachute::stepVertical(int dy)
{
	int unit = (dy > 0) ? 1 : -1;

	while(dy != 0)
	{
		posEnemy.y += unit;
		bool blocked = (unit > 0) ? map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &posEnemy.y)
		                          : map->collisionMoveUp(posEnemy, glm::ivec2(16, 16), &posEnemy.y);
		if(blocked)
			return false;
		dy -= unit;
	}

	return true;
}

bool EnemyParachute::stepHorizontal(int dx)
{
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

bool EnemyParachute::seesPlayerBelow() const
{
	if(player == NULL || player->isDying())
		return false;

	glm::vec2 pPos = player->getPosition();
	float dx = std::abs((pPos.x + 8.f) - float(posEnemy.x + 8));
	float dy = (pPos.y + 8.f) - float(posEnemy.y + 8);

	return dx <= PARACHUTE_SIGHT_X_TILES * TILE_SIZE && dy >= TILE_SIZE && dy <= PARACHUTE_SIGHT_BELOW_TILES * TILE_SIZE;
}

bool EnemyParachute::playerInGlideRange() const
{
	if(player == NULL || player->isDying())
		return false;

	glm::vec2 pPos = player->getPosition();
	float dx = std::abs((pPos.x + 8.f) - float(posEnemy.x + 8));
	float dy = (pPos.y + 8.f) - float(posEnemy.y + 8);

	return dx <= PARACHUTE_SIGHT_X_TILES * TILE_SIZE && dy >= 0.f && dy <= PARACHUTE_SIGHT_BELOW_TILES * TILE_SIZE;
}

int EnemyParachute::playerCenterX() const
{
	return int(player->getPosition().x) + 8;
}

void EnemyParachute::setAnimation(int anim)
{
	if(sprite->animation() != anim)
		sprite->changeAnimation(anim);
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
