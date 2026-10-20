#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "AssetPath.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2

#define INIT_ENEMY_X_TILES 10
#define INIT_ENEMY_Y_TILES 2

#define INIT_ENEMY_CHASER_X_TILES 4
#define INIT_ENEMY_CHASER_Y_TILES 11


Scene::Scene()
{
	map = NULL;
	player = NULL;
	bBombKeyPressed = false;
	cameraX = 0.f;
	cameraY = 0.f;
	cameraWidth = CAMERA_WIDTH;
	cameraHeight = CAMERA_HEIGHT;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL)
			delete enemies[i];
	}
	enemies.clear();
	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i] != NULL)
			delete bombs[i];
	}
	bombs.clear();
}


void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap(assetPath("levels/level4.txt"), glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	glm::ivec2 playerTile(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
	if(map->hasPlayerSpawn())
		playerTile = map->getPlayerSpawn();
	player->setPosition(glm::vec2(playerTile.x * map->getTileSize(), playerTile.y * map->getTileSize()));
	player->setTileMap(map);
	player->setBombs(&bombs);

	const vector<EnemySpawn> &spawns = map->getEnemySpawns();
	if(spawns.empty())
	{
		addEnemy(INIT_ENEMY_X_TILES, INIT_ENEMY_Y_TILES);
		addEnemyChaser(INIT_ENEMY_CHASER_X_TILES, INIT_ENEMY_CHASER_Y_TILES);
	}
	else
	{
		for(unsigned int i = 0; i < spawns.size(); i++)
			addEnemyOfType(spawns[i].type, spawns[i].tile.x, spawns[i].tile.y);
	}

	cameraWidth = CAMERA_WIDTH;
	cameraHeight = CAMERA_HEIGHT;
	cameraX = 0.f;
	cameraY = 0.f;

	updateCamera();
	currentTime = 0.0f;

	// Carregar textura de la bomba (bomba.png)
	spritesheetBomb.loadFromFile(assetPath("images/bomba.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetBomb.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetBomb.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetBomb.setMinFilter(GL_NEAREST);
	spritesheetBomb.setMagFilter(GL_NEAREST);
	bBombKeyPressed = false;
}

void Scene::addEnemy(int tileX, int tileY)
{
	Enemy *newEnemy = new Enemy();
	newEnemy->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemy->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemy->setTileMap(map);
	enemies.push_back(newEnemy);
}

void Scene::addEnemyChaser(int tileX, int tileY)
{
	EnemyChaser *newEnemyChaser = new EnemyChaser();
	newEnemyChaser->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemyChaser->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemyChaser->setTileMap(map);
	newEnemyChaser->setPlayer(player);
	enemies.push_back(newEnemyChaser);
}

void Scene::addEnemyParachute(int tileX, int tileY)
{
	EnemyParachute *newEnemyParachute = new EnemyParachute();
	newEnemyParachute->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemyParachute->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemyParachute->setTileMap(map);
	enemies.push_back(newEnemyParachute);
}

void Scene::addEnemyOfType(const std::string &type, int tileX, int tileY)
{
	if(type == "patrol")
		addEnemy(tileX, tileY);
	else if(type == "chaser")
		addEnemyChaser(tileX, tileY);
	else if(type == "parachute")
		addEnemyParachute(tileX, tileY);
	else
		cout << "Unknown ENEMY type in level file: " << type << endl;
}

void Scene::update(int deltaTime)
{
	// Plantar bomba en prémer la lletra 'a'
	if(Game::instance().getKey(GLFW_KEY_A))
	{
		if(!bBombKeyPressed && player != NULL && !player->isDying())
		{
			bBombKeyPressed = true;
			dropBomb();
		}
	}
	else
	{
		bBombKeyPressed = false;
	}

	currentTime += deltaTime;
	player->update(deltaTime);
	for(int i = int(enemies.size()) - 1; i >= 0; i--)
	{
		if(enemies[i] != NULL)
		{
			enemies[i]->update(deltaTime);
			if(enemies[i]->isDead())
			{
				delete enemies[i];
				enemies.erase(enemies.begin() + i);
			}
		}
	}

	// Actualitzar bombes
	for(int b = int(bombs.size()) - 1; b >= 0; b--)
	{
		if(bombs[b] != NULL)
		{
			bombs[b]->update(deltaTime);
			if(bombs[b]->isFinished())
			{
				delete bombs[b];
				bombs.erase(bombs.begin() + b);
			}
		}
	}

	checkCollisions();
	checkBombCollisions();
	updateCamera();
}

void Scene::checkCollisions()
{
	if(player == NULL || player->isDying())
		return;

	glm::vec2 pPos = player->getPosition();
	glm::vec2 pSize = glm::vec2(player->getSize());

	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] == NULL || enemies[i]->isDying() || enemies[i]->isDead())
			continue;

		glm::vec2 ePos = glm::vec2(enemies[i]->getPosition());
		glm::vec2 eSize = glm::vec2(enemies[i]->getSize());

		// Detecció de col·lisió AABB 2D (Diapositiva 27 de Física per a videojocs)
		bool collisionX = (pPos.x < ePos.x + eSize.x) && (pPos.x + pSize.x > ePos.x);
		bool collisionY = (pPos.y < ePos.y + eSize.y) && (pPos.y + pSize.y > ePos.y);

		if(collisionX && collisionY)
		{
			// El personatge sempre es mor quan toca un enemic
			player->die();
			break;
		}
	}
}

void Scene::dropBomb()
{
	if(player == NULL || map == NULL)
		return;

	// Posició del jugador alineada al bloc més proper de 16x16
	glm::vec2 pPos = player->getPosition();
	int tileX = int((pPos.x + 8.f) / map->getTileSize());
	int tileY = int((pPos.y + 8.f) / map->getTileSize());

	// Evitar plantar dues bombes a la mateixa casella
	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i]->getTileCoords() == glm::ivec2(tileX, tileY))
			return;
	}

	Bomb *newBomb = new Bomb(glm::ivec2(tileX, tileY), glm::ivec2(SCREEN_X, SCREEN_Y), &spritesheetBomb, &texProgram, map);
	bombs.push_back(newBomb);
}

void Scene::checkBombCollisions()
{
	for(unsigned int b = 0; b < bombs.size(); b++)
	{
		if(bombs[b] != NULL && bombs[b]->isExploding())
		{
			std::vector<glm::ivec2> explTiles = bombs[b]->getExplosionTiles();
			for(unsigned int t = 0; t < explTiles.size(); t++)
			{
				glm::vec2 tPos = glm::vec2(explTiles[t]);
				glm::vec2 tSize(16.f, 16.f);

				// 1. Matar el jugador si toca l'explosió
				if(player != NULL && !player->isDying())
				{
					glm::vec2 pPos = player->getPosition();
					glm::vec2 pSize = glm::vec2(player->getSize());
					if((pPos.x < tPos.x + tSize.x) && (pPos.x + pSize.x > tPos.x) &&
					   (pPos.y < tPos.y + tSize.y) && (pPos.y + pSize.y > tPos.y))
					{
						player->die(true);
					}
				}

				// 2. Matar els enemics que toquin l'explosió amb la seva animació de mort
				for(unsigned int e = 0; e < enemies.size(); e++)
				{
					if(enemies[e] == NULL || enemies[e]->isDying() || enemies[e]->isDead())
						continue;
					glm::vec2 ePos = glm::vec2(enemies[e]->getPosition());
					glm::vec2 eSize = glm::vec2(enemies[e]->getSize());
					if((ePos.x < tPos.x + tSize.x) && (ePos.x + eSize.x > tPos.x) &&
					   (ePos.y < tPos.y + tSize.y) && (ePos.y + eSize.y > tPos.y))
					{
						enemies[e]->die();
					}
				}
			}
		}
	}
}

void Scene::updateCamera()
{
	if(player == NULL || map == NULL)
		return;

	glm::vec2 playerPos = player->getPosition();

	// Seguir el jugador amb la regla d'1/3 i 2/3 (Diapositiva 36)
	if((playerPos.x - cameraX) < (cameraWidth / 3.f))
		cameraX = playerPos.x - cameraWidth / 3.f;
	if((playerPos.x - cameraX) > (2.f * cameraWidth / 3.f))
		cameraX = playerPos.x - 2.f * cameraWidth / 3.f;

	if((playerPos.y - cameraY) < (cameraHeight / 3.f))
		cameraY = playerPos.y - cameraHeight / 3.f;
	if((playerPos.y - cameraY) > (2.f * cameraHeight / 3.f))
		cameraY = playerPos.y - 2.f * cameraHeight / 3.f;

	// Limitar la càmera als marges del mapa (Diapositiva 26)
	float mapPixelWidth = float(map->getMapWidth() * map->getTileSize());
	float mapPixelHeight = float(map->getMapHeight() * map->getTileSize());
	float maxCameraX = mapPixelWidth - cameraWidth;
	float maxCameraY = mapPixelHeight - cameraHeight;

	if(maxCameraX > 0.f)
		cameraX = glm::clamp(cameraX, 0.f, maxCameraX);
	else
		cameraX = maxCameraX / 2.f;

	if(maxCameraY > 0.f)
		cameraY = glm::clamp(cameraY, 0.f, maxCameraY);
	else
		cameraY = maxCameraY / 2.f;

	// Actualitzar la matriu de projecció ortogràfica (Diapositiva 30)
	projection = glm::ortho(cameraX, cameraX + cameraWidth, cameraY + cameraHeight, cameraY);
}

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i] != NULL)
			bombs[i]->render();
	}
	player->render();
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL)
			enemies[i]->render();
	}
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, assetPath("shaders/texture.vert"));
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, assetPath("shaders/texture.frag"));
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



