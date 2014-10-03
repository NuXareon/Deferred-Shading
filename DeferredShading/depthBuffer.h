#include "utils.h"

class depthBuffer
{
public:
	depthBuffer(void);
	~depthBuffer(void);
	void init(unsigned int widht, unsigned int height);
	void bind();
	void unBind();
	void bindTex();

	GLuint FBO;
	GLuint depthTexture;
};

