#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "AssetPath.h"
#include "AudioManager.h"
#include "Game.h"

#define SCREEN_X 0
#define SCREEN_Y 0

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 2

#define INIT_ENEMY_X_TILES 10
#define INIT_ENEMY_Y_TILES 2

#define INIT_ENEMY_CHASER_X_TILES 4
#define INIT_ENEMY_CHASER_Y_TILES 11

#define GOD_MODE_OFF_GRACE_MS 1500
#define GOD_MODE_OFF_REFILL_SECONDS 30
#define BUSH_FADE_DURATION_MS 400
#define LEVEL_CLEAR_DELAY_MS 2200
#define LEVEL_RESTART_DELAY_MS 2000
#define POWERUP_SPAWN_CHANCE_PERCENT 70

Scene::Scene()
{
	map = NULL;
	player = NULL;
	doorSprite = NULL;
	bResourcesLoaded = false;
	bDoorOpen = false;
	powerUps.clear();
	bBombKeyPressed = false;
	bKKeyPressed = false;
	bGKeyPressed = false;
	bNumKeyPressed = false;
	bGodMode = false;
	bLevelCleared = false;
	levelClearTimer = 0;
	restartLevelTimer = 0;
	bExitUnlocked = false;
	bGameOver = false;
	bGameWon = false;
	bWaitingGameOver = false;
	bWaitingRestartLevel = false;
	pendingLevel = 0;
	currentLevel = 1;
	maxSimultaneousBombs = 1;
	bombFlameRadius = 1;
	levelTimeRemaining = 215;
	levelTimerMs = 0;
	playerLives = 2;
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
	if(doorSprite != NULL)
		delete doorSprite;
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
	for(unsigned int i = 0; i < powerUps.size(); i++)
	{
		if(powerUps[i] != NULL)
			delete powerUps[i];
	}
	powerUps.clear();
}

void Scene::init()
{
	if(!bResourcesLoaded)
	{
		loadResources();
		bResourcesLoaded = true;
	}

	currentTime = 0.0f;
	srand((unsigned int)time(NULL));
	bBombKeyPressed = false;
	bKKeyPressed = false;
	bGKeyPressed = false;
	bNumKeyPressed = false;
	bGodMode = false;
	bGameOver = false;
	bGameWon = false;
	bWaitingGameOver = false;
	bWaitingRestartLevel = false;
	bExitUnlocked = false;
	playerLives = 2;
	maxSimultaneousBombs = 1;
	bombFlameRadius = 1;

	if(player != NULL)
		player->reset();

	loadLevel(1);
}

void Scene::loadResources()
{
	initShaders();

	spritesheetBomb.loadFromFile(assetPath("images/bomba.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetBomb.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetBomb.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetBomb.setMinFilter(GL_NEAREST);
	spritesheetBomb.setMagFilter(GL_NEAREST);

	spritesheetPowerUps.loadFromFile(assetPath("images/power-ups.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetPowerUps.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetPowerUps.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetPowerUps.setMinFilter(GL_NEAREST);
	spritesheetPowerUps.setMagFilter(GL_NEAREST);

	spritesheetDoor.loadFromFile(assetPath("images/door_open.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheetDoor.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheetDoor.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheetDoor.setMinFilter(GL_NEAREST);
	spritesheetDoor.setMagFilter(GL_NEAREST);

	doorSprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(1.f, 1.f), &spritesheetDoor, &texProgram);
	doorSprite->setNumberAnimations(1);
	doorSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	doorSprite->changeAnimation(0);

	hud.init(&texProgram);
}

void Scene::loadLevel(int levelNumber)
{
	currentLevel = levelNumber;
	if(currentLevel < 1) currentLevel = 1;
	if(currentLevel > 5) currentLevel = 5;

	bGameOver = false;
	bWaitingGameOver = false;
	bWaitingRestartLevel = false;
	pendingLevel = 0;
	levelTimeRemaining = 215;
	levelTimerMs = 0;

	for(unsigned int i = 0; i < powerUps.size(); i++)
	{
		if(powerUps[i] != NULL)
			delete powerUps[i];
	}
	powerUps.clear();

	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i] != NULL)
			delete bombs[i];
	}
	bombs.clear();

	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL)
			delete enemies[i];
	}
	enemies.clear();

	if(map != NULL)
	{
		delete map;
		map = NULL;
	}

	char levelFileName[64];
	sprintf(levelFileName, "levels/level%02d.txt", currentLevel);
	map = TileMap::createTileMap(assetPath(levelFileName), glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	if(player == NULL)
	{
		player = new Player();
		player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	}

	glm::ivec2 playerTile(INIT_PLAYER_X_TILES, INIT_PLAYER_Y_TILES);
	if(map->hasPlayerSpawn())
		playerTile = map->getPlayerSpawn();
	player->setPosition(glm::vec2(playerTile.x * map->getTileSize(), playerTile.y * map->getTileSize()));
	player->setTileMap(map);
	player->setInvulnerable(1500);
	player->setGodTint(bGodMode);

	bLevelCleared = false;
	levelClearTimer = 0;
	restartLevelTimer = 0;
	bExitUnlocked = false;
	bDoorOpen = false;
	if(map->hasDoor())
	{
		glm::ivec2 doorTile = map->getDoorTile();
		doorSprite->setPosition(glm::vec2(float(SCREEN_X + doorTile.x * map->getTileSize()), float(SCREEN_Y + doorTile.y * map->getTileSize())));
	}

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
}

void Scene::nextLevel()
{
	if(currentLevel >= 5)
	{
		bGameWon = true;
		maxSimultaneousBombs = 1;
		bombFlameRadius = 1;
		playerLives = 2;
		if(player != NULL)
			player->reset();
		return;
	}
	pendingLevel = currentLevel + 1;
}

void Scene::applyPendingLevel()
{
	if(pendingLevel != 0)
		loadLevel(pendingLevel);
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
	newEnemyParachute->setPlayer(player);
	enemies.push_back(newEnemyParachute);
}

void Scene::addEnemyPufferfish(int tileX, int tileY)
{
	EnemyPufferfish *newEnemyPufferfish = new EnemyPufferfish();
	newEnemyPufferfish->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemyPufferfish->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemyPufferfish->setTileMap(map);
	newEnemyPufferfish->setPlayer(player);
	enemies.push_back(newEnemyPufferfish);
}

void Scene::addEnemyBoss(int tileX, int tileY)
{
	EnemyBoss *newEnemyBoss = new EnemyBoss();
	newEnemyBoss->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	newEnemyBoss->setPosition(glm::vec2(tileX * map->getTileSize(), tileY * map->getTileSize()));
	newEnemyBoss->setTileMap(map);
	newEnemyBoss->setPlayer(player);
	enemies.push_back(newEnemyBoss);
}

void Scene::addEnemyOfType(const std::string &type, int tileX, int tileY)
{
	if(type == "patrol")
		addEnemy(tileX, tileY);
	else if(type == "chaser")
		addEnemyChaser(tileX, tileY);
	else if(type == "parachute")
		addEnemyParachute(tileX, tileY);
	else if(type == "pufferfish")
		addEnemyPufferfish(tileX, tileY);
	else if(type == "boss")
		addEnemyBoss(tileX, tileY);
	else
		cout << "Unknown ENEMY type in level file: " << type << endl;
}

void Scene::spawnPowerUp(int tileX, int tileY)
{
	for(unsigned int i = 0; i < powerUps.size(); i++)
	{
		if(powerUps[i] != NULL && powerUps[i]->getTileCoords() == glm::ivec2(tileX, tileY))
			return;
	}

	bool bHeartPresent = (player != NULL && player->hasHeart());
	if(!bHeartPresent)
	{
		for(unsigned int i = 0; i < powerUps.size(); i++)
		{
			if(powerUps[i] != NULL && powerUps[i]->getType() == POWERUP_HEART)
			{
				bHeartPresent = true;
				break;
			}
		}
	}

	static int nextType = 0;
	PowerUpType type;
	if(bHeartPresent)
	{
		const PowerUpType nonHeartPool[3] = { POWERUP_BOMB_UP, POWERUP_FIRE_UP, POWERUP_SPEED_UP };
		type = nonHeartPool[nextType % 3];
		nextType++;
	}
	else
	{
		type = (PowerUpType)(nextType % 4);
		nextType++;
	}

	PowerUp *newPowerUp = new PowerUp(type, glm::ivec2(tileX, tileY), &spritesheetPowerUps, &texProgram);
	powerUps.push_back(newPowerUp);
}

void Scene::applyPowerUp(PowerUpType type)
{
	AudioManager::instance().playSound(SFX_PICKUP);
	switch(type)
	{
	case POWERUP_BOMB_UP:
		maxSimultaneousBombs++;
		break;
	case POWERUP_HEART:
		if(player != NULL)
			player->giveHeart();
		break;
	case POWERUP_FIRE_UP:
		bombFlameRadius++;
		break;
	case POWERUP_SPEED_UP:
		if(player != NULL)
			player->increaseSpeed(0.6f);
		break;
	}
}

void Scene::update(int deltaTime)
{
	if(pendingLevel != 0)
		return;

	if(Game::instance().getKey(GLFW_KEY_1))
	{
		if(!bNumKeyPressed) { bNumKeyPressed = true; pendingLevel = 1; return; }
	}
	else if(Game::instance().getKey(GLFW_KEY_2))
	{
		if(!bNumKeyPressed) { bNumKeyPressed = true; pendingLevel = 2; return; }
	}
	else if(Game::instance().getKey(GLFW_KEY_3))
	{
		if(!bNumKeyPressed) { bNumKeyPressed = true; pendingLevel = 3; return; }
	}
	else if(Game::instance().getKey(GLFW_KEY_4))
	{
		if(!bNumKeyPressed) { bNumKeyPressed = true; pendingLevel = 4; return; }
	}
	else if(Game::instance().getKey(GLFW_KEY_5))
	{
		if(!bNumKeyPressed) { bNumKeyPressed = true; pendingLevel = 5; return; }
	}
	else
	{
		bNumKeyPressed = false;
	}

	if(Game::instance().getKey(GLFW_KEY_K))
	{
		if(!bKKeyPressed)
		{
			bKKeyPressed = true;
			for(unsigned int i = 0; i < enemies.size(); i++)
			{
				if(enemies[i] != NULL && !enemies[i]->isDead() && !enemies[i]->isDying())
					enemies[i]->die();
			}
			if(!bDoorOpen)
				AudioManager::instance().playSound(SFX_DOOR_OPEN);
			bExitUnlocked = true;
			bDoorOpen = true;
		}
	}
	else
	{
		bKKeyPressed = false;
	}

	if(Game::instance().getKey(GLFW_KEY_G))
	{
		if(!bGKeyPressed && player != NULL && !player->isDying())
		{
			bGKeyPressed = true;
			toggleGodMode();
		}
	}
	else
	{
		bGKeyPressed = false;
	}

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

	bool bClockRunning = !bLevelCleared && player != NULL && !player->isDying();
	if(levelTimeRemaining > 0 && bClockRunning)
	{
		levelTimerMs += deltaTime;
		while(levelTimerMs >= 1000)
		{
			levelTimerMs -= 1000;
			if(levelTimeRemaining > 0)
			{
				levelTimeRemaining--;
				if(levelTimeRemaining == 0 && !bGodMode && player != NULL && !player->isDying())
				{
					handlePlayerDeath(false);
				}
			}
		}
	}

	if(map != NULL)
		map->update(deltaTime);
	updateBombSolids();
	if(player != NULL)
		player->update(deltaTime);

	if(player != NULL && !player->isDying() && map != NULL)
	{
		float bottomLimit = float(map->getMapHeight() * map->getTileSize());
		if(player->getPosition().y >= bottomLimit)
		{
			handlePlayerDeath(false);
		}
	}

	if(bWaitingGameOver)
	{
		restartLevelTimer += deltaTime;
		if(restartLevelTimer >= LEVEL_RESTART_DELAY_MS)
		{
			bWaitingGameOver = false;
			bGameOver = true;
		}
	}
	else if(bWaitingRestartLevel)
	{
		restartLevelTimer += deltaTime;
		if(restartLevelTimer >= LEVEL_RESTART_DELAY_MS)
		{
			bWaitingRestartLevel = false;
			pendingLevel = currentLevel;
			return;
		}
	}

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

	int aliveEnemies = 0;
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL && !enemies[i]->isDead() && !enemies[i]->isDying())
			aliveEnemies++;
	}

	if(aliveEnemies == 0 && !bDoorOpen)
	{
		AudioManager::instance().playSound(SFX_DOOR_OPEN);
		bExitUnlocked = true;
		bDoorOpen = true;
	}

	bool bPlayerAtDoor = false;
	if(map != NULL && map->hasDoor() && player != NULL && !player->isDying())
	{
		glm::ivec2 doorTile = map->getDoorTile();
		glm::vec2 pPos = player->getPosition();
		glm::ivec2 pSize = player->getSize();

		float pCenterX = pPos.x + float(pSize.x) * 0.5f;
		float pCenterY = pPos.y + float(pSize.y) * 0.5f;
		float doorCenterX = float(doorTile.x * map->getTileSize()) + float(map->getTileSize()) * 0.5f;
		float doorCenterY = float(doorTile.y * map->getTileSize()) + float(map->getTileSize()) * 0.5f;

		if(std::abs(pCenterX - doorCenterX) <= 12.0f && std::abs(pCenterY - doorCenterY) <= 14.0f)
		{
			bPlayerAtDoor = true;
		}
	}

	if(bDoorOpen && bPlayerAtDoor && !bLevelCleared && player != NULL && !player->isDying())
	{
		bExitUnlocked = false;
		bLevelCleared = true;
		levelClearTimer = 0;
		player->enterDoor();
		AudioManager::instance().playSound(SFX_DOOR_ENTER);
		AudioManager::instance().playMusic(JINGLE_STAGE_CLEAR);
	}

	if(bLevelCleared)
	{
		levelClearTimer += deltaTime;
		if(levelClearTimer >= LEVEL_CLEAR_DELAY_MS)
		{
			bLevelCleared = false;
			nextLevel();
			return;
		}
	}

	if(player != NULL && !player->isDying())
	{
		glm::vec2 pPos = player->getPosition();
		glm::ivec2 pSize = player->getSize();

		for(int p = int(powerUps.size()) - 1; p >= 0; p--)
		{
			if(powerUps[p] != NULL)
			{
				powerUps[p]->update(deltaTime);
				if(powerUps[p]->checkCollision(pPos, pSize))
				{
					applyPowerUp(powerUps[p]->getType());
					powerUps[p]->collect();
					delete powerUps[p];
					powerUps.erase(powerUps.begin() + p);
				}
			}
		}
	}
	else
	{
		for(unsigned int p = 0; p < powerUps.size(); p++)
		{
			if(powerUps[p] != NULL)
				powerUps[p]->update(deltaTime);
		}
	}

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

void Scene::handlePlayerDeath(bool byBomb)
{
	if(player == NULL || player->isDying() || player->isEnteringDoor())
		return;

	player->die(byBomb);
	AudioManager::instance().playMusic(JINGLE_MISS);

	maxSimultaneousBombs = 1;
	bombFlameRadius = 1;

	if(playerLives > 0)
	{
		playerLives--;
		bWaitingRestartLevel = true;
		restartLevelTimer = 0;
	}
	else
	{
		bWaitingGameOver = true;
		restartLevelTimer = 0;
	}
}

void Scene::toggleGodMode()
{
	bGodMode = !bGodMode;
	player->setGodTint(bGodMode);
	if(bGodMode)
		return;

	player->setInvulnerable(GOD_MODE_OFF_GRACE_MS);
	if(levelTimeRemaining == 0)
	{
		levelTimeRemaining = GOD_MODE_OFF_REFILL_SECONDS;
		levelTimerMs = 0;
	}
}

void Scene::checkCollisions()
{
	if(player == NULL || player->isDying() || player->isInvulnerable() || bGodMode)
		return;

	glm::vec2 pPos = player->getPosition();
	glm::vec2 pSize = glm::vec2(player->getSize());

	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] == NULL || enemies[i]->isDying() || enemies[i]->isDead())
			continue;

		glm::vec2 ePos = glm::vec2(enemies[i]->getPosition());
		glm::vec2 eSize = glm::vec2(enemies[i]->getSize());

		bool collisionX = (pPos.x < ePos.x + eSize.x) && (pPos.x + pSize.x > ePos.x);
		bool collisionY = (pPos.y < ePos.y + eSize.y) && (pPos.y + pSize.y > ePos.y);

		if((collisionX && collisionY) || enemies[i]->hitsPlayer(pPos, pSize))
		{
			if(player->hasHeart())
			{
				player->consumeHeart();
				player->setInvulnerable(1500);
			}
			else
			{
				handlePlayerDeath(false);
			}
			break;
		}
	}
}

void Scene::dropBomb()
{
	if(player == NULL || map == NULL)
		return;

	if(int(bombs.size()) >= maxSimultaneousBombs)
		return;

	glm::vec2 pPos = player->getPosition();
	int tileX = int((pPos.x + 8.f) / map->getTileSize());
	int tileY = int((pPos.y + 8.f) / map->getTileSize());

	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i]->getTileCoords() == glm::ivec2(tileX, tileY))
			return;
	}

	Bomb *newBomb = new Bomb(glm::ivec2(tileX, tileY), glm::ivec2(SCREEN_X, SCREEN_Y), &spritesheetBomb, &texProgram, map, bombFlameRadius);
	bombs.push_back(newBomb);
	AudioManager::instance().playSound(SFX_BOMB_PLACE);
}

void Scene::updateBombSolids()
{
	if(map == NULL)
		return;

	map->clearDynamicSolids();
	for(unsigned int b = 0; b < bombs.size(); b++)
	{
		if(bombs[b] == NULL || bombs[b]->getState() != BOMB_TICKING)
			continue;

		if(!bombs[b]->isSolid() && !isTileOccupiedByEntity(bombs[b]->getPosition()))
			bombs[b]->setSolid(true);

		if(bombs[b]->isSolid())
		{
			glm::ivec2 tile = bombs[b]->getTileCoords();
			map->setDynamicSolid(tile.x, tile.y);
		}
	}
}

bool Scene::isTileOccupiedByEntity(const glm::ivec2 &tilePixelPos) const
{
	const int tileSize = 16;

	if(player != NULL && !player->isDying())
	{
		glm::ivec2 pPos = glm::ivec2(player->getPosition());
		glm::ivec2 pSize = player->getSize();
		if(pPos.x < tilePixelPos.x + tileSize && pPos.x + pSize.x > tilePixelPos.x &&
		   pPos.y < tilePixelPos.y + tileSize && pPos.y + pSize.y > tilePixelPos.y)
			return true;
	}

	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] == NULL || enemies[i]->isDead())
			continue;

		glm::ivec2 ePos = enemies[i]->getPosition();
		glm::ivec2 eSize = enemies[i]->getSize();
		if(ePos.x < tilePixelPos.x + tileSize && ePos.x + eSize.x > tilePixelPos.x &&
		   ePos.y < tilePixelPos.y + tileSize && ePos.y + eSize.y > tilePixelPos.y)
			return true;
	}

	return false;
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

				if(player != NULL && !player->isDying())
				{
					glm::vec2 pPos = player->getPosition();
					glm::vec2 pSize = glm::vec2(player->getSize());
					if((pPos.x < tPos.x + tSize.x) && (pPos.x + pSize.x > tPos.x) &&
					   (pPos.y < tPos.y + tSize.y) && (pPos.y + pSize.y > tPos.y))
					{
						if(player->isInvulnerable() || bGodMode)
						{
						}
						else if(player->hasHeart())
						{
							player->consumeHeart();
							player->setInvulnerable(1500);
						}
						else
						{
							handlePlayerDeath(true);
						}
					}
				}

				for(unsigned int e = 0; e < enemies.size(); e++)
				{
					if(enemies[e] == NULL || enemies[e]->isDying() || enemies[e]->isDead())
						continue;
					glm::vec2 ePos = glm::vec2(enemies[e]->getPosition());
					glm::vec2 eSize = glm::vec2(enemies[e]->getSize());
					if((ePos.x < tPos.x + tSize.x) && (ePos.x + eSize.x > tPos.x) &&
					   (ePos.y < tPos.y + tSize.y) && (ePos.y + eSize.y > tPos.y))
					{
						enemies[e]->hit();
					}
				}
			}

			if(map != NULL)
			{
				glm::ivec2 bombTile = bombs[b]->getTileCoords();

				if(map->isBush(bombTile.x, bombTile.y))
				{
					if(!map->isFadingBush(bombTile.x, bombTile.y))
					{
						if((rand() % 100) < POWERUP_SPAWN_CHANCE_PERCENT)
						{
							map->destroyBush(bombTile.x, bombTile.y);
							spawnPowerUp(bombTile.x, bombTile.y);
						}
						else
						{
							map->destroyBushFade(bombTile.x, bombTile.y, BUSH_FADE_DURATION_MS);
						}
					}
				}
				map->triggerBarrier(bombTile.x, bombTile.y);

				const glm::ivec2 directions[4] = { glm::ivec2(-1, 0), glm::ivec2(1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };
				for(int d = 0; d < 4; d++)
				{
					for(int r = 1; r <= bombFlameRadius; r++)
					{
						int tx = bombTile.x + directions[d].x * r;
						int ty = bombTile.y + directions[d].y * r;

						if(map->isBush(tx, ty))
						{
							if(!map->isFadingBush(tx, ty))
							{
								if((rand() % 100) < POWERUP_SPAWN_CHANCE_PERCENT)
								{
									map->destroyBush(tx, ty);
									spawnPowerUp(tx, ty);
								}
								else
								{
									map->destroyBushFade(tx, ty, BUSH_FADE_DURATION_MS);
								}
							}
							break;
						}
						else if(map->isGround(tx, ty))
						{
							map->triggerBarrier(tx, ty);
							break;
						}
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

	if((playerPos.x - cameraX) < (cameraWidth / 3.f))
		cameraX = playerPos.x - cameraWidth / 3.f;
	if((playerPos.x - cameraX) > (2.f * cameraWidth / 3.f))
		cameraX = playerPos.x - 2.f * cameraWidth / 3.f;

	if((playerPos.y - cameraY) < (cameraHeight / 3.f))
		cameraY = playerPos.y - cameraHeight / 3.f;
	if((playerPos.y - cameraY) > (2.f * cameraHeight / 3.f))
		cameraY = playerPos.y - 2.f * cameraHeight / 3.f;

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

	if(map != NULL)
		map->render();

	if(bDoorOpen && map != NULL && map->hasDoor())
		doorSprite->render();

	for(unsigned int i = 0; i < powerUps.size(); i++)
	{
		if(powerUps[i] != NULL)
			powerUps[i]->render();
	}

	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i] != NULL)
			bombs[i]->render();
	}

	if(player != NULL)
		player->render();

	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL)
			enemies[i]->render();
	}

	int aliveEnemies = 0;
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(enemies[i] != NULL && !enemies[i]->isDead() && !enemies[i]->isDying())
			aliveEnemies++;
	}

	bool bShowExit = bExitUnlocked && !bLevelCleared && (player != NULL && !player->isDying());
	bool bHasHeart = (player != NULL && player->hasHeart());
	hud.render(levelTimeRemaining, playerLives, aliveEnemies, maxSimultaneousBombs, bombFlameRadius, bHasHeart, bShowExit, currentTime, bGodMode);

	texProgram.setUniformMatrix4f("projection", projection);
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
