#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2

#define INIT_ENEMY_X_TILES 10
#define INIT_ENEMY_Y_TILES 2


Scene::Scene()
{
	map = NULL;
	player = NULL;
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
}


void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/level4.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	
	addEnemy(INIT_ENEMY_X_TILES, INIT_ENEMY_Y_TILES);

	cameraWidth = CAMERA_WIDTH;
	cameraHeight = CAMERA_HEIGHT;
	cameraX = 0.f;
	cameraY = 0.f;

	updateCamera();
	currentTime = 0.0f;
}

void Scene::addEnemy(int tileX, int tileY)
{
	Enemy *newEnemy = new Enemy();
	newEnemy->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemy->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemy->setTileMap(map);
	enemies.push_back(newEnemy);
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL)
			enemies[i]->update(deltaTime);
	}

	checkCollisions();
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
		if(enemies[i] == NULL)
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

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
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



