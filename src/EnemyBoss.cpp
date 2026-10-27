#include <cmath>
#include <GL/glew.h>
#include "EnemyBoss.h"
#include "AssetPath.h"

#define TILE_SIZE 16
#define BOSS_SIZE 32
#define BOSS_HP 6
#define BOSS_HIT_FLASH_MS 1000
#define BOSS_FLASH_BLINK_MS 80
#define BOSS_TELEPORT_PERIOD_MS 6000
#define BOSS_FADE_MS 300
#define BOSS_DRIFT_MS_PER_PX 40
#define BOSS_VERTICAL_MS_PER_PX 20
#define BOSS_HEIGHT_PERIOD_MS 8000
#define BOSS_BOB_PX 4
#define BOSS_BOB_PERIOD_MS 1600.f
#define BOSS_HIGH_ROW_OFFSET 0
#define BOSS_LOW_ROW_OFFSET 5
#define BOSS_BULLET_MS_PER_PX 16
#define BOSS_BULLET_LIFE_MS 3000
#define BOSS_BULLET_HITBOX 8
#define BOSS_DEATH_MS 1500
#define BOSS_DEATH_BLINK_MS 60
#define BOSS_BLAST_INTERVAL_MS 200
#define BOSS_BLAST_LENGTH_MS 500

enum EnemyBossAnims
{
	ENEMY_BOSS_DRIFT, ENEMY_BOSS_CAST
};

static const int TELEPORT_OFFSETS[BOSS_TELEPORT_POINTS][2] = { { -5, 0 }, { 0, 0 }, { 5, 0 }, { -3, 2 }, { 3, 2 } };
static const int BLAST_OFFSETS[BOSS_DEATH_BLASTS][2] = { { -4, -4 }, { 20, -2 }, { 8, 10 }, { -6, 18 }, { 22, 20 }, { 8, -8 } };
static const int BULLET_DIRS[4][2] = { { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };


EnemyBoss::EnemyBoss() : Enemy()
{
	player = NULL;
	program = NULL;
	bulletSprite = NULL;
	blastSprite = NULL;
	state = BOSS_DRIFT;
	spawnPos = glm::ivec2(0, 0);
	bSpawnKnown = false;
	bTeleportQueued = false;
	hp = BOSS_HP;
	flashTimer = 0;
	teleportTimer = 0;
	fadeTimer = 0;
	heightTimer = 0;
	driftTimer = 0;
	verticalTimer = 0;
	nextPoint = 0;
	clearBullets();
}

EnemyBoss::~EnemyBoss()
{
	if(bulletSprite != NULL)
		delete bulletSprite;
	if(blastSprite != NULL)
		delete blastSprite;
}

void EnemyBoss::setPlayer(Player *p)
{
	player = p;
}

void EnemyBoss::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	program = &shaderProgram;

	spritesheet.loadFromFile(assetPath("images/e_boss.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	float texW = spritesheet.width() > 0 ? float(spritesheet.width()) : 128.f;
	float texH = spritesheet.height() > 0 ? float(spritesheet.height()) : 48.f;

	float stepX = 16.f / texW;
	float stepY = 16.f / texH;

	sprite = Sprite::createSprite(glm::vec2(BOSS_SIZE, BOSS_SIZE), glm::vec2(2.f * stepX, 2.f * stepY), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

	sprite->setAnimationSpeed(ENEMY_BOSS_DRIFT, 4);
	sprite->addKeyframe(ENEMY_BOSS_DRIFT, glm::vec2(0.f * stepX, 0.f));
	sprite->addKeyframe(ENEMY_BOSS_DRIFT, glm::vec2(2.f * stepX, 0.f));

	sprite->setAnimationSpeed(ENEMY_BOSS_CAST, 6);
	sprite->addKeyframe(ENEMY_BOSS_CAST, glm::vec2(4.f * stepX, 0.f));
	sprite->addKeyframe(ENEMY_BOSS_CAST, glm::vec2(6.f * stepX, 0.f));

	bulletSprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
	bulletSprite->setNumberAnimations(1);
	bulletSprite->setAnimationSpeed(0, 8);
	bulletSprite->addKeyframe(0, glm::vec2(0.f * stepX, 2.f * stepY));
	bulletSprite->addKeyframe(0, glm::vec2(1.f * stepX, 2.f * stepY));
	bulletSprite->changeAnimation(0);

	blastSheet.loadFromFile(assetPath("images/bomba.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	blastSheet.setWrapS(GL_CLAMP_TO_EDGE);
	blastSheet.setWrapT(GL_CLAMP_TO_EDGE);
	blastSheet.setMinFilter(GL_NEAREST);
	blastSheet.setMagFilter(GL_NEAREST);

	float blastW = blastSheet.width() > 0 ? float(blastSheet.width()) : 128.f;
	float blastH = blastSheet.height() > 0 ? float(blastSheet.height()) : 128.f;
	float blastStepX = 16.f / blastW;
	float blastStepY = 16.f / blastH;

	blastSprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(blastStepX, blastStepY), &blastSheet, &shaderProgram);
	blastSprite->setNumberAnimations(1);
	blastSprite->setAnimationSpeed(0, 6);
	blastSprite->addKeyframe(0, glm::vec2(3.f * blastStepX, 1.f * blastStepY));
	blastSprite->addKeyframe(0, glm::vec2(4.f * blastStepX, 1.f * blastStepY));
	blastSprite->addKeyframe(0, glm::vec2(5.f * blastStepX, 1.f * blastStepY));
	blastSprite->changeAnimation(0);

	direction = MOVE_DIR_RIGHT;
	state = BOSS_DRIFT;
	hp = BOSS_HP;
	clearBullets();
	sprite->changeAnimation(ENEMY_BOSS_DRIFT);

	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyBoss::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(bDying)
	{
		blastSprite->update(deltaTime);
		dyingTime += deltaTime;
		if(dyingTime >= BOSS_DEATH_MS)
			bDead = true;
		return;
	}

	if(!bSpawnKnown)
	{
		spawnPos = posEnemy;
		bSpawnKnown = true;
	}

	if(flashTimer > 0)
	{
		flashTimer -= deltaTime;
		if(flashTimer < 0)
			flashTimer = 0;
	}

	bulletSprite->update(deltaTime);
	updateBullets(deltaTime);

	if(state == BOSS_DRIFT)
	{
		updateDrift(deltaTime);
		teleportTimer += deltaTime;
		if(flashTimer == 0 && (bTeleportQueued || teleportTimer >= BOSS_TELEPORT_PERIOD_MS))
			startTeleport();
	}
	else
		updateTeleport(deltaTime);

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}

void EnemyBoss::render()
{
	float alpha = fadeAlpha();
	float flash = isFlashBright() ? 1.f : 0.f;
	bool bBlend = alpha < 1.f;

	if(bBlend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	program->setUniform4f("color", 1.f, 1.f, 1.f, alpha);
	program->setUniform4f("flashColor", 1.f, 1.f, 1.f, flash);
	sprite->render();
	program->setUniform4f("flashColor", 0.f, 0.f, 0.f, 0.f);
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	if(bBlend)
		glDisable(GL_BLEND);

	for(int i = 0; i < BOSS_MAX_BULLETS; i++)
	{
		if(!bullets[i].bAlive)
			continue;
		bulletSprite->setPosition(glm::vec2(float(tileMapDispl.x + bullets[i].pos.x), float(tileMapDispl.y + bullets[i].pos.y)));
		bulletSprite->render();
	}

	if(bDying)
		renderDeathBlasts();
}

void EnemyBoss::die()
{
	if(bDying)
		return;
	bDying = true;
	bDead = false;
	dyingTime = 0;
	state = BOSS_DRIFT;
	flashTimer = 0;
	clearBullets();
	blastSprite->changeAnimation(0);
}

void EnemyBoss::hit()
{
	if(bDying || state != BOSS_DRIFT || flashTimer > 0)
		return;

	hp--;
	flashTimer = BOSS_HIT_FLASH_MS;
	bTeleportQueued = true;

	if(hp <= 0)
		die();
}

bool EnemyBoss::hitsPlayer(const glm::vec2 &playerPos, const glm::vec2 &playerSize) const
{
	if(bDying)
		return false;

	int inset = (16 - BOSS_BULLET_HITBOX) / 2;
	for(int i = 0; i < BOSS_MAX_BULLETS; i++)
	{
		if(!bullets[i].bAlive)
			continue;
		float bx = float(bullets[i].pos.x + inset);
		float by = float(bullets[i].pos.y + inset);
		bool bOverlapX = (playerPos.x < bx + BOSS_BULLET_HITBOX) && (playerPos.x + playerSize.x > bx);
		bool bOverlapY = (playerPos.y < by + BOSS_BULLET_HITBOX) && (playerPos.y + playerSize.y > by);
		if(bOverlapX && bOverlapY)
			return true;
	}
	return false;
}

void EnemyBoss::updateDrift(int deltaTime)
{
	heightTimer += deltaTime;

	driftTimer += deltaTime;
	while(driftTimer >= BOSS_DRIFT_MS_PER_PX)
	{
		driftTimer -= BOSS_DRIFT_MS_PER_PX;
		int dx = (direction == MOVE_DIR_RIGHT) ? 1 : -1;
		if(!stepHorizontal(dx))
			direction = (direction == MOVE_DIR_RIGHT) ? MOVE_DIR_LEFT : MOVE_DIR_RIGHT;
	}

	verticalTimer += deltaTime;
	while(verticalTimer >= BOSS_VERTICAL_MS_PER_PX)
	{
		verticalTimer -= BOSS_VERTICAL_MS_PER_PX;
		int target = targetHeight();
		if(posEnemy.y < target)
			stepVertical(1);
		else if(posEnemy.y > target)
			stepVertical(-1);
	}
}

void EnemyBoss::updateTeleport(int deltaTime)
{
	fadeTimer += deltaTime;
	if(fadeTimer < BOSS_FADE_MS)
		return;

	fadeTimer = 0;
	if(state == BOSS_TELEPORT_OUT)
	{
		moveToNextPoint();
		state = BOSS_TELEPORT_IN;
	}
	else
	{
		fireBurst();
		state = BOSS_DRIFT;
		teleportTimer = 0;
		sprite->changeAnimation(ENEMY_BOSS_DRIFT);
	}
}

void EnemyBoss::updateBullets(int deltaTime)
{
	for(int i = 0; i < BOSS_MAX_BULLETS; i++)
	{
		if(!bullets[i].bAlive)
			continue;

		bullets[i].age += deltaTime;
		if(bullets[i].age >= BOSS_BULLET_LIFE_MS)
		{
			bullets[i].bAlive = false;
			continue;
		}

		bullets[i].moveTimer += deltaTime;
		while(bullets[i].bAlive && bullets[i].moveTimer >= BOSS_BULLET_MS_PER_PX)
		{
			bullets[i].moveTimer -= BOSS_BULLET_MS_PER_PX;
			bullets[i].pos += bullets[i].dir;
			glm::ivec2 center = bullets[i].pos + glm::ivec2(8, 8);
			if(center.x < 0 || center.y < 0 || map->isGround(center.x / TILE_SIZE, center.y / TILE_SIZE))
				bullets[i].bAlive = false;
		}
	}
}

void EnemyBoss::startTeleport()
{
	state = BOSS_TELEPORT_OUT;
	fadeTimer = 0;
	bTeleportQueued = false;
	sprite->changeAnimation(ENEMY_BOSS_CAST);
}

void EnemyBoss::moveToNextPoint()
{
	glm::ivec2 points[BOSS_TELEPORT_POINTS];
	bool bValid[BOSS_TELEPORT_POINTS];
	int nearest = -1;
	float nearestDist = 0.f;

	for(int i = 0; i < BOSS_TELEPORT_POINTS; i++)
	{
		points[i] = spawnPos + glm::ivec2(TELEPORT_OFFSETS[i][0] * TILE_SIZE, TELEPORT_OFFSETS[i][1] * TILE_SIZE);
		bValid[i] = !blockedAt(points[i]);
		if(!bValid[i] || player == NULL || player->isDying())
			continue;

		glm::vec2 playerCenter = player->getPosition() + glm::vec2(8.f, 8.f);
		glm::vec2 pointCenter = glm::vec2(points[i]) + glm::vec2(BOSS_SIZE / 2, BOSS_SIZE / 2);
		glm::vec2 d = playerCenter - pointCenter;
		float dist = d.x * d.x + d.y * d.y;
		if(nearest == -1 || dist < nearestDist)
		{
			nearest = i;
			nearestDist = dist;
		}
	}

	for(int k = 0; k < BOSS_TELEPORT_POINTS; k++)
	{
		int i = (nextPoint + k) % BOSS_TELEPORT_POINTS;
		if(!bValid[i] || i == nearest || points[i] == posEnemy)
			continue;
		posEnemy = points[i];
		nextPoint = (i + 1) % BOSS_TELEPORT_POINTS;
		return;
	}
}

void EnemyBoss::fireBurst()
{
	glm::ivec2 origin = posEnemy + glm::ivec2(BOSS_SIZE / 2 - 8, BOSS_SIZE / 2 - 8);
	int dir = 0;
	for(int i = 0; i < BOSS_MAX_BULLETS && dir < 4; i++)
	{
		if(bullets[i].bAlive)
			continue;
		bullets[i].pos = origin;
		bullets[i].dir = glm::ivec2(BULLET_DIRS[dir][0], BULLET_DIRS[dir][1]);
		bullets[i].age = 0;
		bullets[i].moveTimer = 0;
		bullets[i].bAlive = true;
		dir++;
	}
}

void EnemyBoss::clearBullets()
{
	for(int i = 0; i < BOSS_MAX_BULLETS; i++)
	{
		bullets[i].pos = glm::ivec2(0, 0);
		bullets[i].dir = glm::ivec2(0, 0);
		bullets[i].age = 0;
		bullets[i].moveTimer = 0;
		bullets[i].bAlive = false;
	}
}

bool EnemyBoss::stepHorizontal(int dx)
{
	posEnemy.x += dx;
	if(blockedAt(posEnemy))
	{
		posEnemy.x -= dx;
		return false;
	}
	return true;
}

bool EnemyBoss::stepVertical(int dy)
{
	int lowest = spawnPos.y + BOSS_LOW_ROW_OFFSET * TILE_SIZE;
	if(dy > 0 && posEnemy.y + dy > lowest)
		return false;

	posEnemy.y += dy;
	if(blockedAt(posEnemy))
	{
		posEnemy.y -= dy;
		return false;
	}
	return true;
}

bool EnemyBoss::blockedAt(const glm::ivec2 &pos) const
{
	if(map == NULL)
		return false;

	int x0 = pos.x / TILE_SIZE;
	int x1 = (pos.x + BOSS_SIZE - 1) / TILE_SIZE;
	int y0 = pos.y / TILE_SIZE;
	int y1 = (pos.y + BOSS_SIZE - 1) / TILE_SIZE;
	for(int y = y0; y <= y1; y++)
		for(int x = x0; x <= x1; x++)
			if(map->isGround(x, y))
				return true;
	return false;
}

int EnemyBoss::targetHeight() const
{
	int highest = spawnPos.y + BOSS_HIGH_ROW_OFFSET * TILE_SIZE;
	int lowest = spawnPos.y + BOSS_LOW_ROW_OFFSET * TILE_SIZE;

	int half = BOSS_HEIGHT_PERIOD_MS / 2;
	int phase = heightTimer % BOSS_HEIGHT_PERIOD_MS;
	float t = phase < half ? float(phase) / float(half) : float(BOSS_HEIGHT_PERIOD_MS - phase) / float(half);
	float dip = t * t;
	float bob = float(BOSS_BOB_PX) * sinf(2.f * 3.14159265f * float(heightTimer) / BOSS_BOB_PERIOD_MS);

	int target = highest + int(float(lowest - highest) * dip + bob);
	if(target < highest)
		target = highest;
	if(target > lowest)
		target = lowest;
	return target;
}

float EnemyBoss::fadeAlpha() const
{
	if(state == BOSS_TELEPORT_OUT)
		return 1.f - float(fadeTimer) / float(BOSS_FADE_MS);
	if(state == BOSS_TELEPORT_IN)
		return float(fadeTimer) / float(BOSS_FADE_MS);
	return 1.f;
}

bool EnemyBoss::isFlashBright() const
{
	if(bDying)
		return (dyingTime / BOSS_DEATH_BLINK_MS) % 2 == 0;
	if(flashTimer > 0)
		return (flashTimer / BOSS_FLASH_BLINK_MS) % 2 == 0;
	return false;
}

void EnemyBoss::renderDeathBlasts()
{
	for(int i = 0; i < BOSS_DEATH_BLASTS; i++)
	{
		int start = i * BOSS_BLAST_INTERVAL_MS;
		if(dyingTime < start || dyingTime >= start + BOSS_BLAST_LENGTH_MS)
			continue;
		blastSprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x + BLAST_OFFSETS[i][0]), float(tileMapDispl.y + posEnemy.y + BLAST_OFFSETS[i][1])));
		blastSprite->render();
	}
}
