#include "texture.h"

texture::texture(GLenum tt, const std::string& f){
	textureTarget = tt;
	fileName = f;
	image = NULL;
}

bool texture::Load(){
	try {
		image = new Magick::Image(fileName);
		image->write(&blob, "RGBA");
	}
	catch (Magick::Error& error) {
		int b = 2;
	}

	int a = 1;
	return true;
}

void texture::Bind(GLenum TextureUnit){}
