#include <QtOpenGL>
#include <Magick++.h>

class texture
{
public:
	texture(GLenum TextureTarget, const std::string& FileName);
	bool Load();
	void Bind(GLenum TextureUnit);

private:
	std::string fileName;
	GLenum textureTarget;
	GLuint textureObj;
	Magick::Image* image;
	Magick::Blob blob;
};

