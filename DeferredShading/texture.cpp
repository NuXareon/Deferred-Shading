#include "texture.h"

Texture::Texture(GLenum tt, const std::string& f){
	textureTarget = tt;
	fileName = f;
	image = NULL;
}

bool Texture::Load(){
	try {
		image = new Magick::Image(fileName);
		image->write(&blob, "RGBA");
	}
	catch (Magick::Error& error) {
		return false;
	}

	glGenTextures(1, &textureObj);
	glBindTexture(textureTarget, textureObj);
	glTexImage2D(textureTarget, 0, GL_RGBA, image->columns(), image->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());
	glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

void Texture::Bind(GLenum TextureUnit){
	QGLFunctions glFuncs(QGLContext::currentContext());
	glFuncs.glActiveTexture(TextureUnit);
	glBindTexture(textureTarget, textureObj);
}
