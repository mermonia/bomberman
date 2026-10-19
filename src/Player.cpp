#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "AssetPath.h"
#include "Game.h"
#include "Bomb.h"


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
	activeBombs = NULL;
	spawnPos = glm::ivec2(0, 0);
	bDying = false;
	dyingTime = 0;
	maxDyingTime = 1000;
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
	spritesheet.loadFromFile(assetPath("images/bub.png"), TEXTURE_PIXEL_FORMAT_RGBA);
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
	spritesheetDeath.loadFromFile(assetPath("images/player_2.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetDeath.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setMinFilter(GL_NEAREST);
	spritesheetDeath.setMagFilter(GL_NEAREST);

	// Cada cel·la és de 16x16 px: 16/64 = 0.25f (X), 16/32 = 0.50f (Y)
	spriteDeath = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(0.25f, 0.50f), &spritesheetDeath, &shaderProgram);
	spriteDeath->setNumberAnimations(2);

	// Animació 0: Mort per enemic (els 8 fotogrames d'abans)
	// Fila 1 (segona fila, y = 0.50f): El personatge es gira i cau d'esquena (4 fotogrames)
	// Fila 0 (primera fila, y = 0.00f): El personatge queda a terra i s'esvaeix en bombolles d'energia (4 fotogrames)
	spriteDeath->setAnimationSpeed(0, 8);
	// Fila 1:
	spriteDeath->addKeyframe(0, glm::vec2(0.00f, 0.0f));
	spriteDeath->addKeyframe(0, glm::vec2(0.75f, 0.50f));

	// Animació 1: Mort per bomba (els fotogrames 2, 3 i 4 de la fila 0)
	spriteDeath->setAnimationSpeed(1, 5);
	spriteDeath->addKeyframe(1, glm::vec2(0.75f, 0.00f)); // Fotograma 2 (cercle vermell)
	spriteDeath->addKeyframe(1, glm::vec2(0.50f, 0.00f)); // Fotograma 3 (esclat vermell)
	spriteDeath->addKeyframe(1, glm::vec2(0.25f, 0.00f)); // Fotograma 4 (dispersió)

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
		// Quan es completa l'animació de mort (maxDyingTime), reapareix al punt d'inici
		if(dyingTime >= maxDyingTime)
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
				bJumping = !collisionDown(&posPlayer.y);
				if(!bJumping)
				{
					if(sprite->animation() == JUMP_LEFT || sprite->animation() == FALL_LEFT)
						sprite->changeAnimation(STAND_LEFT);
					else if(sprite->animation() == JUMP_RIGHT || sprite->animation() == FALL_RIGHT)
						sprite->changeAnimation(STAND_RIGHT);
				}
			}
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(collisionDown(&posPlayer.y))
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

void Player::die(bool byBomb)
{
	if(bDying)
		return;
	bDying = true;
	dyingTime = 0;
	bJumping = false;
	if(byBomb)
	{
		maxDyingTime = 600;
		spriteDeath->changeAnimation(1);
	}
	else
	{
		maxDyingTime = 1000;
		spriteDeath->changeAnimation(0);
	}
	spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

bool Player::collisionDown(int *posY)
{
	if(map != NULL && map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), posY))
		return true;

	if(activeBombs != NULL)
	{
		for(unsigned int i = 0; i < activeBombs->size(); i++)
		{
			Bomb *b = (*activeBombs)[i];
			if(b != NULL && b->getState() == BOMB_TICKING)
			{
				glm::ivec2 bPos = b->getPosition();
				// El jugador està sobre la bomba horitzontalment?
				if(posPlayer.x + 14 > bPos.x && posPlayer.x + 2 < bPos.x + 16)
				{
					// Els peus estan a l'alçada de la part superior de la bomba?
					if(*posY + 16 >= bPos.y && *posY + 16 <= bPos.y + 10)
					{
						*posY = bPos.y - 16;
						return true;
					}
				}
			}
		}
	}
	return false;
}




