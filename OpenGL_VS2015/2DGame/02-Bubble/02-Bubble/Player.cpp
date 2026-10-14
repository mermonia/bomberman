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
	spriteDeath = NULL;
	map = NULL;
	spawnPos = glm::ivec2(0, 0);
	bDying = false;
	dyingTime = 0;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
	if (spriteDeath != NULL)
		delete spriteDeath;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	spritesheet.loadFromFile("images/bub.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(0.25f, 1.0f / 3.0f), &spritesheet, &shaderProgram);
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

	// Spritesheet de mort (player_2.png - 64x32 px, 4 columnes x 2 files de 16x16)
	spritesheetDeath.loadFromFile("images/player_2.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetDeath.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setMinFilter(GL_NEAREST);
	spritesheetDeath.setMagFilter(GL_NEAREST);

	// Cada cel·la és de 16x16 px: 16/64 = 0.25f (X), 16/32 = 0.50f (Y)
	spriteDeath = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(0.25f, 0.50f), &spritesheetDeath, &shaderProgram);
	spriteDeath->setNumberAnimations(1);

	// Animació de mort: els 4 fotogrames de la fila 0 (del primer a l'últim)
	spriteDeath->setAnimationSpeed(0, 5);
	spriteDeath->addKeyframe(0, glm::vec2(0.00f, 0.0f));
	spriteDeath->addKeyframe(0, glm::vec2(0.75f, 0.50f));
	spriteDeath->changeAnimation(0);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	if(bDying)
	{
		spriteDeath->update(deltaTime);
		dyingTime += deltaTime;
		// Quan es completa l'animació dels 4 fotogrames (800ms), reapareix al punt d'inici
		if(dyingTime >= 800)
		{
			bDying = false;
			posPlayer = spawnPos;
			bJumping = false;
			sprite->changeAnimation(STAND_RIGHT);
			sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
			spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		}
		return;
	}

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
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(16, 16)))
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
		if(map->collisionMoveRight(posPlayer, glm::ivec2(16, 16)))
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
			posPlayer.y = int(startY - 64 * sin(3.14159f * jumpAngle / 180.f));
			if(map->collisionMoveUp(posPlayer, glm::ivec2(16, 16), &posPlayer.y))
			{
				jumpAngle = 90;
				startY = posPlayer.y + 64;
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
				posPlayer.y = int(startY - 64 * sin(3.14159f * jumpAngle / 180.f));
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y);
			}
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), &posPlayer.y))
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
	if(bDying)
		spriteDeath->render();
	else
		sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	spawnPos = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	if(spriteDeath != NULL)
		spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::die()
{
	if(bDying)
		return;
	bDying = true;
	dyingTime = 0;
	bJumping = false;
	spriteDeath->changeAnimation(0);
	spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




