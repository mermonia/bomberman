#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_LEFT, FALL_LEFT, JUMP_RIGHT, FALL_RIGHT
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	spritesheet.loadFromFile("images/bub.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(0.25f, 1.0f / 3.0f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(8);
	
		sprite->setAnimationSpeed(STAND_LEFT, 8);
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.f, 0.f));
		
		sprite->setAnimationSpeed(STAND_RIGHT, 8);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.75f, 0.f));
		
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 1.f / 3.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.75f, 2.f / 3.f));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.f));
		
		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.50f, 0.f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75f, 0.f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25f, 0.f));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75f, 0.f));
		
		sprite->setAnimationSpeed(JUMP_LEFT, 8);
		sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.25f, 2.f / 3.f));
		sprite->setAnimationSpeed(FALL_LEFT, 8);
		sprite->addKeyframe(FALL_LEFT, glm::vec2(0.f, 2.f / 3.f));
		
		sprite->setAnimationSpeed(JUMP_RIGHT, 8);
		sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.50f, 1.f / 3.f));
		sprite->setAnimationSpeed(FALL_RIGHT, 8);
		sprite->addKeyframe(FALL_RIGHT, glm::vec2(0.25f, 1.f / 3.f));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);
	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if(bJumping)
		{
			if(jumpAngle <= 90)
			{
				if(sprite->animation() != JUMP_LEFT)
					sprite->changeAnimation(JUMP_LEFT);
			}
			else
			{
				if(sprite->animation() != FALL_LEFT)
					sprite->changeAnimation(FALL_LEFT);
			}
		}
		else if(sprite->animation() == FALL_LEFT || sprite->animation() == FALL_RIGHT)
		{
			if(sprite->animation() != FALL_LEFT)
				sprite->changeAnimation(FALL_LEFT);
		}
		else
		{
			if(sprite->animation() != MOVE_LEFT)
				sprite->changeAnimation(MOVE_LEFT);
		}
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x += 2;
			if(!bJumping && sprite->animation() != FALL_LEFT && sprite->animation() != FALL_RIGHT)
				sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if(bJumping)
		{
			if(jumpAngle <= 90)
			{
				if(sprite->animation() != JUMP_RIGHT)
					sprite->changeAnimation(JUMP_RIGHT);
			}
			else
			{
				if(sprite->animation() != FALL_RIGHT)
					sprite->changeAnimation(FALL_RIGHT);
			}
		}
		else if(sprite->animation() == FALL_LEFT || sprite->animation() == FALL_RIGHT)
		{
			if(sprite->animation() != FALL_RIGHT)
				sprite->changeAnimation(FALL_RIGHT);
		}
		else
		{
			if(sprite->animation() != MOVE_RIGHT)
				sprite->changeAnimation(MOVE_RIGHT);
		}
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			if(!bJumping && sprite->animation() != FALL_LEFT && sprite->animation() != FALL_RIGHT)
				sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if(bJumping)
		{
			if(jumpAngle > 90)
			{
				if(sprite->animation() == JUMP_LEFT)
					sprite->changeAnimation(FALL_LEFT);
				else if(sprite->animation() == JUMP_RIGHT)
					sprite->changeAnimation(FALL_RIGHT);
			}
		}
		else if(sprite->animation() != FALL_LEFT && sprite->animation() != FALL_RIGHT)
		{
			if(sprite->animation() == MOVE_LEFT)
				sprite->changeAnimation(STAND_LEFT);
			else if(sprite->animation() == MOVE_RIGHT)
				sprite->changeAnimation(STAND_RIGHT);
		}
	}
	
	if(bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle <= 90)
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(map->collisionMoveUp(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
			{
				jumpAngle = 90;
				startY = posPlayer.y + 96;
				if(sprite->animation() == JUMP_LEFT)
					sprite->changeAnimation(FALL_LEFT);
				else if(sprite->animation() == JUMP_RIGHT)
					sprite->changeAnimation(FALL_RIGHT);
			}
		}
		else
		{
			if(sprite->animation() == JUMP_LEFT)
				sprite->changeAnimation(FALL_LEFT);
			else if(sprite->animation() == JUMP_RIGHT)
				sprite->changeAnimation(FALL_RIGHT);

			if(jumpAngle >= 180)
			{
				bJumping = false;
				posPlayer.y = startY;
			}
			else
			{
				posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
			}
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			if(sprite->animation() == JUMP_LEFT || sprite->animation() == FALL_LEFT)
				sprite->changeAnimation(STAND_LEFT);
			else if(sprite->animation() == JUMP_RIGHT || sprite->animation() == FALL_RIGHT)
				sprite->changeAnimation(STAND_RIGHT);

			if(Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
				if(sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT || sprite->animation() == FALL_LEFT)
					sprite->changeAnimation(JUMP_LEFT);
				else if(sprite->animation() == MOVE_RIGHT || sprite->animation() == STAND_RIGHT || sprite->animation() == FALL_RIGHT)
					sprite->changeAnimation(JUMP_RIGHT);
			}
		}
		else
		{
			if(sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT)
				sprite->changeAnimation(FALL_LEFT);
			else if(sprite->animation() == MOVE_RIGHT || sprite->animation() == STAND_RIGHT)
				sprite->changeAnimation(FALL_RIGHT);
		}
	}
	
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




