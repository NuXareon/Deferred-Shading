//#include <QtOpenGL>
#include "utils.h"
#include <Magick++.h>

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& FileName);
	bool Load();
	void Bind(GLenum TextureUnit);

private:
	std::string fileName;
	GLenum textureTarget;
	GLuint textureObj;
	Magick::Image* image;
	Magick::Blob blob;
};

