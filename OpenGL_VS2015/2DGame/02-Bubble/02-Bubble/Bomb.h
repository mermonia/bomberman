#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "Sprite.h"
#include "TileMap.h"
#include "ShaderProgram.h"

enum BombState
{
	BOMB_TICKING,
	BOMB_EXPLODING,
	BOMB_FINISHED
};

class Bomb
{
public:
	Bomb(const glm::ivec2 &tilePos, const glm::ivec2 &tileMapPos, Texture *spritesheet, ShaderProgram *program, TileMap *tileMap)
	{
		state = BOMB_TICKING;
		timer = 0;
		tileCoords = tilePos;
		tileMapDispl = tileMapPos;
		map = tileMap;

		int tileSize = map != NULL ? map->getTileSize() : 16;
		pos = glm::vec2(float(tileCoords.x * tileSize), float(tileCoords.y * tileSize));

		float texW = spritesheet->width() > 0 ? float(spritesheet->width()) : 128.f;
		float texH = spritesheet->height() > 0 ? float(spritesheet->height()) : 128.f;

		float stepX = 16.f / texW;
		float stepY = 16.f / texH;

		// Sprite de la bomba (bomba de 16x16)
		spriteBomb = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), spritesheet, program);
		spriteBomb->setNumberAnimations(1);
		spriteBomb->setAnimationSpeed(0, 4); // 4 fps de pulsació
		spriteBomb->addKeyframe(0, glm::vec2(0.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(1.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(2.f * stepX, 0.f * stepY));
		spriteBomb->addKeyframe(0, glm::vec2(1.f * stepX, 0.f * stepY));
		spriteBomb->changeAnimation(0);
		spriteBomb->setPosition(glm::vec2(float(tileMapDispl.x + pos.x), float(tileMapDispl.y + pos.y)));

		// Sprite de l'explosió (16x32: ona expansiva completa de la part encerclada)
		spriteExplosion = Sprite::createSprite(glm::vec2(16, 32), glm::vec2(stepX, 2.f * stepY), spritesheet, program);
		spriteExplosion->setNumberAnimations(1);
		spriteExplosion->setAnimationSpeed(0, 6); // 6 fps (~500ms d'animació per als 3 fotogrames)
		// Fila 0: Columnes 3 (la part encerclada), 4 i 5
		spriteExplosion->addKeyframe(0, glm::vec2(3.f * stepX, 0.f * stepY));
		spriteExplosion->addKeyframe(0, glm::vec2(4.f * stepX, 0.f * stepY));
		spriteExplosion->addKeyframe(0, glm::vec2(5.f * stepX, 0.f * stepY));
		spriteExplosion->changeAnimation(0);

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
		if(spriteExplosion != NULL)
			delete spriteExplosion;
	}

	void update(int deltaTime)
	{
		if(state == BOMB_TICKING)
		{
			spriteBomb->update(deltaTime);
			timer += deltaTime;
			// Després de 2.2 segons la bomba explota
			if(timer >= 2200)
			{
				state = BOMB_EXPLODING;
				timer = 0;
				spriteExplosion->changeAnimation(0);

				// Comprovar quins braços de la creu poden estendre's
				bExplodeCenter = true;
				// Esquerra
				bExplodeLeft = (tileCoords.x > 0);
				if(bExplodeLeft && map != NULL && map->isGround(tileCoords.x - 1, tileCoords.y))
					bExplodeLeft = false;
				// Dreta
				bExplodeRight = (map == NULL || tileCoords.x < map->getMapWidth() - 1);
				if(bExplodeRight && map != NULL && map->isGround(tileCoords.x + 1, tileCoords.y))
					bExplodeRight = false;
				// Dalt
				bExplodeUp = (tileCoords.y > 0);
				if(bExplodeUp && map != NULL && map->isGround(tileCoords.x, tileCoords.y - 1))
					bExplodeUp = false;
				// Baix
				bExplodeDown = (map == NULL || tileCoords.y < map->getMapHeight() - 1);
				if(bExplodeDown && map != NULL && map->isGround(tileCoords.x, tileCoords.y + 1))
					bExplodeDown = false;
			}
		}
		else if(state == BOMB_EXPLODING)
		{
			spriteExplosion->update(deltaTime);
			timer += deltaTime;
			// L'explosió dura 500 ms
			if(timer >= 500)
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

			// Dalt (0 graus: ona expansiva cap amunt)
			if(bExplodeUp)
			{
				spriteExplosion->setPosition(glm::vec2(bx + 2.f, by - 16.f));
				spriteExplosion->setAngle(0.f);
				spriteExplosion->render();
			}
			// Baix (180 graus: ona expansiva cap avall, compensant l'eix per centrar-la amb la de dalt)
			if(bExplodeDown)
			{
				spriteExplosion->setPosition(glm::vec2(bx - 2.f, by));
				spriteExplosion->setAngle(180.f);
				spriteExplosion->render();
			}
			// Dreta (90 graus: ona expansiva cap a la dreta, compensada verticalment)
			if(bExplodeRight)
			{
				spriteExplosion->setPosition(glm::vec2(bx + 8.f, by - 6.f));
				spriteExplosion->setAngle(90.f);
				spriteExplosion->render();
			}
			// Esquerra (270 graus: ona expansiva cap a l'esquerra, compensada verticalment)
			if(bExplodeLeft)
			{
				spriteExplosion->setPosition(glm::vec2(bx - 8.f, by - 9.f));
				spriteExplosion->setAngle(270.f);
				spriteExplosion->render();
			}

			// Si està tancat en 4 parets, dibuixar l'explosió centrada
			if(!bExplodeUp && !bExplodeDown && !bExplodeLeft && !bExplodeRight && bExplodeCenter)
			{
				spriteExplosion->setPosition(glm::vec2(bx, by - 8.f));
				spriteExplosion->setAngle(0.f);
				spriteExplosion->render();
			}
		}
	}

	BombState getState() const { return state; }
	bool isExploding() const { return state == BOMB_EXPLODING; }
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
		if(bExplodeLeft)
			tiles.push_back(glm::ivec2(int(pos.x - 16), int(pos.y)));
		if(bExplodeRight)
			tiles.push_back(glm::ivec2(int(pos.x + 16), int(pos.y)));
		if(bExplodeUp)
			tiles.push_back(glm::ivec2(int(pos.x), int(pos.y - 16)));
		if(bExplodeDown)
			tiles.push_back(glm::ivec2(int(pos.x), int(pos.y + 16)));

		return tiles;
	}

private:
	BombState state;
	int timer;
	glm::ivec2 tileCoords;
	glm::ivec2 tileMapDispl;
	glm::vec2 pos;
	TileMap *map;

	Sprite *spriteBomb;
	Sprite *spriteExplosion;

	bool bExplodeCenter;
	bool bExplodeLeft;
	bool bExplodeRight;
	bool bExplodeUp;
	bool bExplodeDown;
};

#endif // _BOMB_INCLUDE
