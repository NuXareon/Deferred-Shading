#include "utils.h"

class gbuffer
{
public:
	enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_POSITION,
		GBUFFER_NORMAL,
		GBUFFER_DIFFUSE,
		GBUFFER_N_TEXTURES
	};
	gbuffer(void);
	~gbuffer(void);
	void init(unsigned int widht, unsigned int height);
	void bind(int i);


	GLuint FBO;
	GLuint textures[GBUFFER_N_TEXTURES];
	GLuint depthTexture;
};

