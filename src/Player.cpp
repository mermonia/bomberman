#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "AssetPath.h"
#include "Game.h"


#define TILE_SIZE 16

#define WALK_SPEED 3.75f
#define FALL_SPEED 15.f
#define JUMP_HEIGHT_TILES 2
#define JUMP_TIME 500

#define JUMP_RISE (JUMP_HEIGHT_TILES * TILE_SIZE)


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
	jumpAngle = 0.f;
	walkRemainder = 0.f;
	fallRemainder = 0.f;
	bDying = false;
	bEnteringDoor = false;
	bHasHeart = false;
	bGodTint = false;
	invulnerableTime = 0;
	walkSpeed = WALK_SPEED;
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
	spritesheet.loadFromFile(assetPath("images/player.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
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

	// Spritesheet de mort (player_death.png - 64x32 px, 4 columnes x 2 files de 16x16)
	spritesheetDeath.loadFromFile(assetPath("images/player_death.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetDeath.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetDeath.setMinFilter(GL_NEAREST);
	spritesheetDeath.setMagFilter(GL_NEAREST);

	// Cada cel·la és de 16x16 px: 16/64 = 0.25f (X), 16/32 = 0.50f (Y)
	spriteDeath = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(0.25f, 0.50f), &spritesheetDeath, &shaderProgram);
	spriteDeath->setNumberAnimations(3);

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

	// Animació 2: Entrar a la porta (segona fila, tercera columna de player_death.png: y = 0.50f, x = 0.50f)
	spriteDeath->setAnimationSpeed(2, 1);
	spriteDeath->addKeyframe(2, glm::vec2(0.50f, 0.50f));

	spriteDeath->changeAnimation(0);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	if(bEnteringDoor)
	{
		if(spriteDeath != NULL)
		{
			spriteDeath->update(deltaTime);
			spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		}
		return;
	}

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

	if(invulnerableTime > 0)
	{
		invulnerableTime -= deltaTime;
		if(invulnerableTime < 0)
			invulnerableTime = 0;
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
		int step = walkStep(deltaTime);
		posPlayer.x -= step;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(16, 16)))
		{
			posPlayer.x += step;
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
		int step = walkStep(deltaTime);
		posPlayer.x += step;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(16, 16)))
		{
			posPlayer.x -= step;
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
		jumpAngle += 180.f * deltaTime / JUMP_TIME;
		if(jumpAngle <= 90)
		{
			posPlayer.y = startY - int(JUMP_RISE * sin(3.14159f * jumpAngle / 180.f) + 0.5f);
			if(map->collisionMoveUp(posPlayer, glm::ivec2(16, 16), &posPlayer.y))
			{
				jumpAngle = 90.f;
				startY = posPlayer.y + JUMP_RISE;
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
				posPlayer.y = startY - int(JUMP_RISE * sin(3.14159f * jumpAngle / 180.f) + 0.5f);
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
		posPlayer.y += fallStep(deltaTime);
		if(collisionDown(&posPlayer.y))
		{
			if(sprite->animation() == JUMP_LEFT || sprite->animation() == FALL_LEFT)
				sprite->changeAnimation(STAND_LEFT);
			else if(sprite->animation() == JUMP_RIGHT || sprite->animation() == FALL_RIGHT)
				sprite->changeAnimation(STAND_RIGHT);

			if(Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0.f;
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
	if(bDying || bEnteringDoor)
	{
		if(spriteDeath != NULL)
			spriteDeath->render();
	}
	else
	{
		if(invulnerableTime <= 0 || (invulnerableTime / 100) % 2 == 0)
			sprite->render();
	}
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	bEnteringDoor = false;
	bDying = false;
	dyingTime = 0;
	bJumping = false;
	posPlayer = pos;
	spawnPos = pos;
	if(sprite != NULL)
	{
		sprite->changeAnimation(STAND_RIGHT);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	}
	if(spriteDeath != NULL)
		spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::enterDoor()
{
	bEnteringDoor = true;
	bJumping = false;
	if(spriteDeath != NULL)
	{
		spriteDeath->changeAnimation(2);
		spriteDeath->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	}
}

void Player::die(bool byBomb)
{
	if(bDying)
		return;
	bEnteringDoor = false;
	bDying = true;
	dyingTime = 0;
	bJumping = false;
	bHasHeart = false;
	invulnerableTime = 0;
	walkSpeed = WALK_SPEED;
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

int Player::walkStep(int deltaTime)
{
	walkRemainder += walkSpeed * TILE_SIZE * deltaTime / 1000.f;
	int step = int(walkRemainder);
	walkRemainder -= float(step);

	return step;
}

int Player::fallStep(int deltaTime)
{
	fallRemainder += FALL_SPEED * TILE_SIZE * deltaTime / 1000.f;
	int step = int(fallRemainder);
	fallRemainder -= float(step);

	return step;
}

bool Player::collisionDown(int *posY)
{
	return map != NULL && map->collisionMoveDown(posPlayer, glm::ivec2(16, 16), posY);
}




