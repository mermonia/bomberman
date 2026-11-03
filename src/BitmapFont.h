#ifndef _BITMAP_FONT_INCLUDE
#define _BITMAP_FONT_INCLUDE

#include <GL/glew.h>

class BitmapFont
{

public:
	BitmapFont();
	~BitmapFont();

	void init();
	void use() const;
	void glyphCoords(char c, float &u0, float &u1) const;

private:
	GLuint texId;

};

#endif // _BITMAP_FONT_INCLUDE
