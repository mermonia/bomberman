#ifndef _QUAD_RENDERER_INCLUDE
#define _QUAD_RENDERER_INCLUDE

#include <GL/glew.h>
#include "ShaderProgram.h"

class QuadRenderer
{

public:
	QuadRenderer();
	~QuadRenderer();

	void init(ShaderProgram &program);
	void render(float x, float y, float width, float height, float u0, float v0, float u1, float v1);
	void renderSolid(float x, float y, float width, float height);

private:
	GLuint whiteTexId;
	GLuint vao;
	GLuint vbo;
	GLint posLocation;
	GLint texCoordLocation;

};

#endif // _QUAD_RENDERER_INCLUDE
