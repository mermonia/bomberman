#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "EnemyChaser.h"
#include "EnemyParachute.h"
#include "EnemyPufferfish.h"
#include "EnemyBoss.h"
#include "Bomb.h"
#include "PowerUp.h"
#include "HUD.h"

using std::vector;


#define CAMERA_WIDTH 240.0f
#define CAMERA_HEIGHT 160.0f


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render();

	void loadLevel(int levelNumber);
	void nextLevel();
	bool hasPendingLevel() const { return pendingLevel != 0; }
	void applyPendingLevel();
	int getCurrentLevel() const { return currentLevel; }
	bool isGameOver() const { return bGameOver; }
	bool isGameWon() const { return bGameWon; }
	void resetGameOver() { bGameOver = false; bWaitingGameOver = false; bWaitingRestartLevel = false; }

	void addEnemy(int tileX, int tileY);
	void addEnemyChaser(int tileX, int tileY);
	void addEnemyParachute(int tileX, int tileY);
	void addEnemyPufferfish(int tileX, int tileY);
	void addEnemyBoss(int tileX, int tileY);
	void addEnemyOfType(const std::string &type, int tileX, int tileY);
	void dropBomb();

	void spawnPowerUp(int tileX, int tileY);
	void applyPowerUp(PowerUpType type);

private:
	void loadResources();
	void initShaders();
	void updateCamera();
	void checkCollisions();
	void checkBombCollisions();
	void updateBombSolids();
	bool isTileOccupiedByEntity(const glm::ivec2 &tilePixelPos) const;
	void handlePlayerDeath(bool byBomb = false);
	void toggleGodMode();

private:
	TileMap *map;
	Player *player;
	vector<Enemy *> enemies;
	vector<Bomb *> bombs;
	vector<PowerUp *> powerUps;
	Texture spritesheetBomb;
	Texture spritesheetPowerUps;
	Texture spritesheetDoor;
	Sprite *doorSprite;
	bool bResourcesLoaded;
	bool bDoorOpen;
	bool bBombKeyPressed;
	bool bKKeyPressed;
	bool bGKeyPressed;
	bool bNumKeyPressed;
	bool bLevelCleared;
	int levelClearTimer;
	bool bExitUnlocked;
	bool bGodMode;
	bool bGameOver;
	bool bGameWon;
	bool bWaitingGameOver;
	bool bWaitingRestartLevel;
	int pendingLevel;
	int currentLevel;
	int maxSimultaneousBombs;
	int bombFlameRadius;
	int levelTimeRemaining;
	int levelTimerMs;
	int playerLives;
	HUD hud;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;

	float cameraX, cameraY;
	float cameraWidth, cameraHeight;

};


#endif // _SCENE_INCLUDE

