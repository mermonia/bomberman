#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"
#include "ShaderProgram.h"
#include "AudioManager.h"

enum BombState
{
	BOMB_TICKING,
	BOMB_EXPLODING,
	BOMB_FINISHED
};

const int BOMB_TICK_TIME = 2200;
const int BOMB_EXPLOSION_DURATION = 250;
const int BOMB_EXPLOSION_FPS = 12;

class Bomb
{
public:
	Bomb(const glm::ivec2 &tilePos, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram *program, TileMap *tileMap, int flameRadius = 1)
	{
		state = BOMB_TICKING;
		timer = 0;
		bSolid = false;
		tileCoords = tilePos;
		tileMapDispl = tileMapPos;
		map = tileMap;
		radius = flameRadius;

		int tileSize = map != NULL ? map->getTileSize() : 16;
		pos = glm::vec2(float(tileCoords.x * tileSize), float(tileCoords.y * tileSize));

		float texW = spritesheet->width() > 0 ? float(spritesheet->width()) : 128.f;
		float texH = spritesheet->height() > 0 ? float(spritesheet->height()) : 128.f;

		float stepX = 16.f / texW;
		float stepY = 16.f / texH;

		spriteBomb = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), spritesheet, program);
		spriteBomb->setNumberAnimations(1);
		spriteBomb->setAnimationSpeed(0, 4);
		spriteBomb->addKeyframe(0, glm::vec2(0.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(1.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(2.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(1.f * stepX, 0.f * stepY));
		spriteBomb->changeAnimation(0);
		spriteBomb->setPosition(glm::vec2(float(tileMapDispl.x + pos.x), float(tileMapDispl.y + pos.y)));

		spriteExplosionCenter = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), spritesheet, program);
		spriteExplosionCenter->setNumberAnimations(1);
		spriteExplosionCenter->setAnimationSpeed(0, BOMB_EXPLOSION_FPS);
		spriteExplosionCenter->addKeyframe(0, glm::vec2(3.f * stepX, 1.f * stepY));
		spriteExplosionCenter->addKeyframe(0, glm::vec2(4.f * stepX, 1.f * stepY));
		spriteExplosionCenter->addKeyframe(0, glm::vec2(5.f * stepX, 1.f * stepY));
		spriteExplosionCenter->changeAnimation(0);

		spriteExplosionBody = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), spritesheet, program);
		spriteExplosionBody->setNumberAnimations(1);
		spriteExplosionBody->setAnimationSpeed(0, BOMB_EXPLOSION_FPS);
		spriteExplosionBody->addKeyframe(0, glm::vec2(1.f * stepX, 1.f * stepY));
		spriteExplosionBody->addKeyframe(0, glm::vec2(0.f * stepX, 1.f * stepY));
		spriteExplosionBody->addKeyframe(0, glm::vec2(0.f * stepX, 2.f * stepY));
		spriteExplosionBody->changeAnimation(0);

		spriteExplosionTip = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), spritesheet, program);
		spriteExplosionTip->setNumberAnimations(1);
		spriteExplosionTip->setAnimationSpeed(0, BOMB_EXPLOSION_FPS);
		spriteExplosionTip->addKeyframe(0, glm::vec2(3.f * stepX, 0.f * stepY));
		spriteExplosionTip->addKeyframe(0, glm::vec2(4.f * stepX, 0.f * stepY));
		spriteExplosionTip->addKeyframe(0, glm::vec2(5.f * stepX, 0.f * stepY));
		spriteExplosionTip->changeAnimation(0);

		bExplodeCenter = true;
		bExplodeLeft = false;
		bExplodeRight = false;
		bExplodeUp = false;
		bExplodeDown = false;
	}

	~Bomb()
	{
		if(spriteBomb != NULL)
			delete spriteBomb;
		if(spriteExplosionCenter != NULL)
			delete spriteExplosionCenter;
		if(spriteExplosionBody != NULL)
			delete spriteExplosionBody;
		if(spriteExplosionTip != NULL)
			delete spriteExplosionTip;
	}

	void update(int deltaTime)
	{
		if(state == BOMB_TICKING)
		{
			spriteBomb->update(deltaTime);
			timer += deltaTime;
			if(timer >= BOMB_TICK_TIME)
			{
				state = BOMB_EXPLODING;
				timer = 0;
				AudioManager::instance().playSound(SFX_EXPLOSION);
				if(spriteExplosionCenter != NULL)
					spriteExplosionCenter->changeAnimation(0);
				if(spriteExplosionBody != NULL)
					spriteExplosionBody->changeAnimation(0);
				if(spriteExplosionTip != NULL)
					spriteExplosionTip->changeAnimation(0);

				bExplodeCenter = true;
				bExplodeLeft = (tileCoords.x > 0);
				if(bExplodeLeft && map != NULL && map->isGround(tileCoords.x - 1, tileCoords.y))
					bExplodeLeft = false;
				bExplodeRight = (map == NULL || tileCoords.x < map->getMapWidth() - 1);
				if(bExplodeRight && map != NULL && map->isGround(tileCoords.x + 1, tileCoords.y))
					bExplodeRight = false;
				bExplodeUp = (tileCoords.y > 0);
				if(bExplodeUp && map != NULL && map->isGround(tileCoords.x, tileCoords.y - 1))
					bExplodeUp = false;
				bExplodeDown = (map == NULL || tileCoords.y < map->getMapHeight() - 1);
				if(bExplodeDown && map != NULL && map->isGround(tileCoords.x, tileCoords.y + 1))
					bExplodeDown = false;
			}
		}
		else if(state == BOMB_EXPLODING)
		{
			if(spriteExplosionCenter != NULL)
				spriteExplosionCenter->update(deltaTime);
			if(spriteExplosionBody != NULL)
				spriteExplosionBody->update(deltaTime);
			if(spriteExplosionTip != NULL)
				spriteExplosionTip->update(deltaTime);
			timer += deltaTime;
			if(timer >= BOMB_EXPLOSION_DURATION)
			{
				state = BOMB_FINISHED;
			}
		}
	}

	void render() const
	{
		if(state == BOMB_TICKING)
		{
			spriteBomb->render();
		}
		else if(state == BOMB_EXPLODING)
		{
			float bx = float(tileMapDispl.x + pos.x);
			float by = float(tileMapDispl.y + pos.y);

			int maxR_Up = 0;
			if(bExplodeUp)
			{
				for(int r = 1; r <= radius; r++)
				{
					if(tileCoords.y - r < 0) break;
					if(map != NULL && map->isGround(tileCoords.x, tileCoords.y - r)) break;
					maxR_Up = r;
					if(map != NULL && map->isBush(tileCoords.x, tileCoords.y - r)) break;
				}
			}

			int maxR_Down = 0;
			if(bExplodeDown)
			{
				for(int r = 1; r <= radius; r++)
				{
					if(map != NULL && tileCoords.y + r >= map->getMapHeight()) break;
					if(map != NULL && map->isGround(tileCoords.x, tileCoords.y + r)) break;
					maxR_Down = r;
					if(map != NULL && map->isBush(tileCoords.x, tileCoords.y + r)) break;
				}
			}

			int maxR_Right = 0;
			if(bExplodeRight)
			{
				for(int r = 1; r <= radius; r++)
				{
					if(map != NULL && tileCoords.x + r >= map->getMapWidth()) break;
					if(map != NULL && map->isGround(tileCoords.x + r, tileCoords.y)) break;
					maxR_Right = r;
					if(map != NULL && map->isBush(tileCoords.x + r, tileCoords.y)) break;
				}
			}

			int maxR_Left = 0;
			if(bExplodeLeft)
			{
				for(int r = 1; r <= radius; r++)
				{
					if(tileCoords.x - r < 0) break;
					if(map != NULL && map->isGround(tileCoords.x - r, tileCoords.y)) break;
					maxR_Left = r;
					if(map != NULL && map->isBush(tileCoords.x - r, tileCoords.y)) break;
				}
			}

			for(int r = 1; r <= maxR_Up; r++)
			{
				glm::vec2 tilePos(bx, by - float(r * 16));
				if(r < maxR_Up && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(tilePos);
					spriteExplosionBody->setAngle(0.f);
					spriteExplosionBody->render();
				}
				else if(r == maxR_Up && spriteExplosionTip != NULL)
				{
					spriteExplosionTip->setPosition(tilePos);
					spriteExplosionTip->setAngle(0.f);
					spriteExplosionTip->render();
				}
			}

			for(int r = 1; r <= maxR_Down; r++)
			{
				glm::vec2 tilePos(bx, by + float(r * 16));
				if(r < maxR_Down && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(tilePos);
					spriteExplosionBody->setAngle(0.f);
					spriteExplosionBody->render();
				}
				else if(r == maxR_Down && spriteExplosionTip != NULL)
				{
					spriteExplosionTip->setPosition(tilePos);
					spriteExplosionTip->setAngle(180.f);
					spriteExplosionTip->render();
				}
			}

			for(int r = 1; r <= maxR_Right; r++)
			{
				glm::vec2 tilePos(bx + float(r * 16), by);
				if(r < maxR_Right && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(tilePos);
					spriteExplosionBody->setAngle(90.f);
					spriteExplosionBody->render();
				}
				else if(r == maxR_Right && spriteExplosionTip != NULL)
				{
					spriteExplosionTip->setPosition(tilePos);
					spriteExplosionTip->setAngle(90.f);
					spriteExplosionTip->render();
				}
			}

			for(int r = 1; r <= maxR_Left; r++)
			{
				glm::vec2 tilePos(bx - float(r * 16), by);
				if(r < maxR_Left && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(tilePos);
					spriteExplosionBody->setAngle(90.f);
					spriteExplosionBody->render();
				}
				else if(r == maxR_Left && spriteExplosionTip != NULL)
				{
					spriteExplosionTip->setPosition(tilePos);
					spriteExplosionTip->setAngle(270.f);
					spriteExplosionTip->render();
				}
			}

			if(bExplodeCenter)
			{
				glm::vec2 centerPos(bx, by);
				bool bHorizontal = (maxR_Left > 0 || maxR_Right > 0);
				bool bVertical = (maxR_Up > 0 || maxR_Down > 0);

				if((bHorizontal || !bVertical) && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(centerPos);
					spriteExplosionBody->setAngle(90.f);
					spriteExplosionBody->render();
				}
				if((bVertical || !bHorizontal) && spriteExplosionBody != NULL)
				{
					spriteExplosionBody->setPosition(centerPos);
					spriteExplosionBody->setAngle(0.f);
					spriteExplosionBody->render();
				}
				if(spriteExplosionCenter != NULL)
				{
					spriteExplosionCenter->setPosition(centerPos);
					spriteExplosionCenter->setAngle(0.f);
					spriteExplosionCenter->render();
				}
			}
		}
	}

	BombState getState() const { return state; }
	bool isExploding() const { return state == BOMB_EXPLODING; }
	bool isSolid() const { return bSolid; }
	void setSolid(bool solid) { bSolid = solid; }
	bool isFinished() const { return state == BOMB_FINISHED; }
	glm::ivec2 getPosition() const { return glm::ivec2(int(pos.x), int(pos.y)); }
	glm::ivec2 getTileCoords() const { return tileCoords; }

	std::vector<glm::ivec2> getExplosionTiles() const
	{
		std::vector<glm::ivec2> tiles;
		if(state != BOMB_EXPLODING)
			return tiles;

		if(bExplodeCenter)
			tiles.push_back(glm::ivec2(int(pos.x), int(pos.y)));

		if(bExplodeUp)
		{
			for(int r = 1; r <= radius; r++)
			{
				if(tileCoords.y - r < 0) break;
				if(map != NULL && map->isGround(tileCoords.x, tileCoords.y - r)) break;
				tiles.push_back(glm::ivec2(int(pos.x), int(pos.y - r * 16)));
				if(map != NULL && map->isBush(tileCoords.x, tileCoords.y - r)) break;
			}
		}
		if(bExplodeDown)
		{
			for(int r = 1; r <= radius; r++)
			{
				if(map != NULL && tileCoords.y + r >= map->getMapHeight()) break;
				if(map != NULL && map->isGround(tileCoords.x, tileCoords.y + r)) break;
				tiles.push_back(glm::ivec2(int(pos.x), int(pos.y + r * 16)));
				if(map != NULL && map->isBush(tileCoords.x, tileCoords.y + r)) break;
			}
		}
		if(bExplodeRight)
		{
			for(int r = 1; r <= radius; r++)
			{
				if(map != NULL && tileCoords.x + r >= map->getMapWidth()) break;
				if(map != NULL && map->isGround(tileCoords.x + r, tileCoords.y)) break;
				tiles.push_back(glm::ivec2(int(pos.x + r * 16), int(pos.y)));
				if(map != NULL && map->isBush(tileCoords.x + r, tileCoords.y)) break;
			}
		}
		if(bExplodeLeft)
		{
			for(int r = 1; r <= radius; r++)
			{
				if(tileCoords.x - r < 0) break;
				if(map != NULL && map->isGround(tileCoords.x - r, tileCoords.y)) break;
				tiles.push_back(glm::ivec2(int(pos.x - r * 16), int(pos.y)));
				if(map != NULL && map->isBush(tileCoords.x - r, tileCoords.y)) break;
			}
		}

		return tiles;
	}

private:
	BombState state;
	int timer;
	int radius;
	bool bSolid;
	glm::ivec2 tileCoords;
	glm::ivec2 tileMapDispl;
	glm::vec2 pos;
	TileMap *map;

	Sprite *spriteBomb;
	Sprite *spriteExplosionCenter;
	Sprite *spriteExplosionBody;
	Sprite *spriteExplosionTip;

	bool bExplodeCenter;
	bool bExplodeLeft;
	bool bExplodeRight;
	bool bExplodeUp;
	bool bExplodeDown;
};

#endif
