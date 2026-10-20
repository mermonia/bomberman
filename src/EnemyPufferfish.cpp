#include <cmath>
#include <GL/glew.h>
#include "EnemyPufferfish.h"
#include "AssetPath.h"

#define PUFFERFISH_DRIFT_SPEED 1.f
#define PUFFERFISH_BOB_PERIOD 1600
#define PUFFERFISH_BOB_AMPLITUDE 4
#define PUFFERFISH_PUFF_RADIUS 48.f

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
	bBaseYSet = false;
	baseY = 0;
	bobTimer = 0;
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
	bBaseYSet = false;
	bobTimer = 0;
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

void EnemyPufferfish::drift(int deltaTime)
{
	if(map == NULL)
		return;

	int steps = moveStep(deltaTime, PUFFERFISH_DRIFT_SPEED);

	while(steps-- > 0)
	{
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
		else
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

void EnemyPufferfish::bob(int deltaTime)
{
	bobTimer = (bobTimer + deltaTime) % PUFFERFISH_BOB_PERIOD;

	float phase = 2.f * 3.14159265f * float(bobTimer) / float(PUFFERFISH_BOB_PERIOD);
	int target = baseY + int(std::lround(float(PUFFERFISH_BOB_AMPLITUDE) * std::sin(phase)));

	while(posEnemy.y < target)
	{
		if(!stepVertical(1))
			break;
	}
	while(posEnemy.y > target)
	{
		if(!stepVertical(-1))
			break;
	}
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

	if(!bBaseYSet)
	{
		baseY = posEnemy.y;
		bBaseYSet = true;
	}

	bPuffed = playerIsNear();

	int targetAnim = bPuffed ? ENEMY_PUFFERFISH_PUFF : ENEMY_PUFFERFISH_SWIM;
	if(sprite->animation() != targetAnim)
		sprite->changeAnimation(targetAnim);

	if(!bPuffed)
		drift(deltaTime);

	bob(deltaTime);

	sprite->setFlipX(direction == MOVE_DIR_RIGHT);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyPufferfish::die()
{
	if(bDying)
		return;
	bDying = true;
	bDead = false;
	dyingTime = 0;
	if(sprite != NULL)
	{
		sprite->setFlipX(false);
		sprite->changeAnimation(ENEMY_PUFFERFISH_DIE);
	}
}
