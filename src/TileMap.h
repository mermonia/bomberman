#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

struct EnemySpawn
{
	std::string type;
	glm::ivec2 tile;
};

class TileMap
{

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void update(int deltaTime);
	void render() const;
	void free();

	int getTileSize() const { return tileSize; }
	int getMapWidth() const { return mapSize.x; }
	int getMapHeight() const { return mapSize.y; }
	glm::ivec2 getMapSize() const { return mapSize; }

	bool hasPlayerSpawn() const { return playerSpawn.x >= 0 && playerSpawn.y >= 0; }
	glm::ivec2 getPlayerSpawn() const { return playerSpawn; }
	bool hasDoor() const { return doorTile.x >= 0 && doorTile.y >= 0; }
	glm::ivec2 getDoorTile() const { return doorTile; }
	const std::vector<EnemySpawn> &getEnemySpawns() const { return enemySpawns; }

	bool isBush(int tileX, int tileY) const;
	bool destroyBush(int tileX, int tileY);

	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool isGround(int tileX, int tileY) const;
	void clearDynamicSolids();
	void setDynamicSolid(int tileX, int tileY);
	void triggerBarrier(int tileX, int tileY);

private:
	struct BarrierCollapse
	{
		int tileX;
		int topY;
		int clearY;
		int timer;
	};

	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);
	bool isSolidTile(int tile) const;
	bool isBlockedTile(int tileX, int tileY) const;
	bool isBarrierSegment(int tile) const;

private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
	int *map;
	bool *dynamicSolid;
	bool zeroBased;
	int airTile;
	glm::ivec2 playerSpawn;
	glm::ivec2 doorTile;
	int doorTileId;
	std::vector<int> bushTiles;
	std::vector<EnemySpawn> enemySpawns;
	glm::vec2 renderMinCoords;
	ShaderProgram *renderProgram;
	int barrierTopTile, barrierBodyTile, barrierBaseTile;
	std::vector<BarrierCollapse> collapsingBarriers;

};

#endif

