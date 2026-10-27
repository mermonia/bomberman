#ifndef _ENEMYBOSS_INCLUDE
#define _ENEMYBOSS_INCLUDE

#include <glm/glm.hpp>
#include "Enemy.h"
#include "Player.h"


#define BOSS_MAX_BULLETS 8
#define BOSS_TELEPORT_POINTS 5
#define BOSS_DEATH_BLASTS 6


enum BossState
{
	BOSS_DRIFT, BOSS_TELEPORT_OUT, BOSS_TELEPORT_IN
};

struct BossBullet
{
	glm::ivec2 pos;
	glm::ivec2 dir;
	int age;
	int moveTimer;
	bool bAlive;
};


class EnemyBoss : public Enemy
{
public:
	EnemyBoss();
	virtual ~EnemyBoss();

	void setPlayer(Player *p);

	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	virtual void update(int deltaTime);
	virtual void render();
	virtual void die();
	virtual void hit();
	virtual bool hitsPlayer(const glm::vec2 &playerPos, const glm::vec2 &playerSize) const;
	virtual glm::ivec2 getSize() const { return glm::ivec2(32, 32); }

private:
	void updateDrift(int deltaTime);
	void updateTeleport(int deltaTime);
	void updateBullets(int deltaTime);
	void startTeleport();
	void moveToNextPoint();
	void fireBurst();
	void clearBullets();
	bool stepHorizontal(int dx);
	bool stepVertical(int dy);
	bool blockedAt(const glm::ivec2 &pos) const;
	int targetHeight() const;
	float fadeAlpha() const;
	bool isFlashBright() const;
	void renderDeathBlasts();

private:
	Player *player;
	ShaderProgram *program;
	Texture blastSheet;
	Sprite *bulletSprite;
	Sprite *blastSprite;
	BossState state;
	BossBullet bullets[BOSS_MAX_BULLETS];
	glm::ivec2 spawnPos;
	bool bSpawnKnown;
	bool bTeleportQueued;
	int hp;
	int flashTimer;
	int teleportTimer;
	int fadeTimer;
	int heightTimer;
	int driftTimer;
	int verticalTimer;
	int nextPoint;

};


#endif // _ENEMYBOSS_INCLUDE
