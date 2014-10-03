#include "depthBuffer.h"


depthBuffer::depthBuffer(void)
{
}


depthBuffer::~depthBuffer(void)
{
}

void depthBuffer::init(unsigned int w, unsigned int h)
{
	// Generate the new framebuffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	

	// Depth texture
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	
	// Check for errors and restore default fbo.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("error status: 0x%x\n", status);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void depthBuffer::bind()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
}

void depthBuffer::unBind()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void depthBuffer::bindTex()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
}