#include "PowerUp.h"

PowerUp::PowerUp(PowerUpType type, const glm::ivec2 &tileCoords, Texture *spritesheet, ShaderProgram *program)
{
	this->type = type;
	this->tileCoords = tileCoords;
	bCollected = false;
	pos = glm::vec2(float(tileCoords.x * 16), float(tileCoords.y * 16));
	timer = 0;

	float tw = (spritesheet != NULL && spritesheet->width() > 0) ? float(spritesheet->width()) : 64.f;
	float th = (spritesheet != NULL && spritesheet->height() > 0) ? float(spritesheet->height()) : 64.f;

	glm::vec2 sizeInSheet(16.f / tw, 16.f / th);
	sprite = Sprite::createSprite(glm::vec2(16, 16), sizeInSheet, spritesheet, program);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 1);

	glm::vec2 frameCoord(0.f);
	switch(type)
	{
	case POWERUP_BOMB_UP:
		frameCoord = glm::vec2(0.f / tw, 0.f / th);
		break;
	case POWERUP_HEART:
		frameCoord = glm::vec2(16.f / tw, 0.f / th);
		break;
	case POWERUP_FIRE_UP:
		frameCoord = glm::vec2(32.f / tw, 0.f / th);
		break;
	case POWERUP_SPEED_UP:
		frameCoord = glm::vec2(0.f / tw, 16.f / th);
		break;
	}

	sprite->addKeyframe(0, frameCoord);
	sprite->changeAnimation(0);
	sprite->setPosition(pos);
}

PowerUp::~PowerUp()
{
	if(sprite != NULL)
		delete sprite;
}

void PowerUp::update(int deltaTime)
{
	timer += deltaTime;
	if(sprite != NULL)
	{
		sprite->update(deltaTime);
		sprite->setPosition(pos);
	}
}

void PowerUp::render() const
{
	if(!bCollected && sprite != NULL)
		sprite->render();
}

bool PowerUp::checkCollision(const glm::vec2 &playerPos, const glm::ivec2 &playerSize) const
{
	if(bCollected)
		return false;

	float pLeft = playerPos.x + 2.f;
	float pRight = playerPos.x + float(playerSize.x) - 2.f;
	float pTop = playerPos.y + 2.f;
	float pBottom = playerPos.y + float(playerSize.y);

	float uLeft = pos.x;
	float uRight = pos.x + 16.f;
	float uTop = pos.y;
	float uBottom = pos.y + 16.f;

	return (pLeft < uRight && pRight > uLeft && pTop < uBottom && pBottom > uTop);
}
