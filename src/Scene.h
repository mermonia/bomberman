#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "EnemyChaser.h"
#include "Bomb.h"

using std::vector;


#define CAMERA_WIDTH 320.0f
#define CAMERA_HEIGHT 240.0f


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

	void addEnemy(int tileX, int tileY);
	void addEnemyChaser(int tileX, int tileY);
	void dropBomb();

private:
	void initShaders();
	void updateCamera();
	void checkCollisions();
	void checkBombCollisions();

private:
	TileMap *map;
	Player *player;
	vector<Enemy *> enemies;
	vector<Bomb *> bombs;
	Texture spritesheetBomb;
	bool bBombKeyPressed;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;

	float cameraX, cameraY;
	float cameraWidth, cameraHeight;

};


#endif // _SCENE_INCLUDE

