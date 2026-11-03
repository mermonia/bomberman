#include "QuadRenderer.h"

QuadRenderer::QuadRenderer()
{
	whiteTexId = 0;
	vao = 0;
	vbo = 0;
	posLocation = 0;
	texCoordLocation = 0;
}

QuadRenderer::~QuadRenderer()
{
	if(whiteTexId != 0)
		glDeleteTextures(1, &whiteTexId);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
}

void QuadRenderer::init(ShaderProgram &program)
{
	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	glGenTextures(1, &whiteTexId);
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glBindVertexArray(0);
}

void QuadRenderer::render(float x, float y, float width, float height, float u0, float v0, float u1, float v1)
{
	float vertices[24] = {
		x, y, u0, v0,
		x + width, y, u1, v0,
		x + width, y + height, u1, v1,
		x, y, u0, v0,
		x + width, y + height, u1, v1,
		x, y + height, u0, v1
	};

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void QuadRenderer::renderSolid(float x, float y, float width, float height)
{
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	render(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f);
}
