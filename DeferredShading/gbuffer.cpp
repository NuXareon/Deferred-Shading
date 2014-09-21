#include "gbuffer.h"

gbuffer::gbuffer(void)
{
}

gbuffer::~gbuffer(void)
{
}

void gbuffer::init(unsigned int w, unsigned int h)
{
	// Generate the new framebuffer object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);

	// Generate gbuffer textures
	glGenTextures(GBUFFER_N_TEXTURES, textures);

	// Bind the textures to the framebuffer object
	for (int i = 0; i < GBUFFER_N_TEXTURES; ++i){
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
		// Avoid possible distortions because of linear interpolation when we map the textures on the F.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, textures[i], 0); 
	}

	// Depth texture
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	// Set Draw Buffers
	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(GBUFFER_N_TEXTURES,drawBuffers);
	
	// Check for errors and restore default fbo.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("error status: 0x%x\n", status);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void gbuffer::bind(int i)
{
	if (i == GBUFFER_DRAW) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	else if (i == GBUFFER_READ) glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	else if (i == GBUFFER_DEFAULT) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	else if (i == GBUFFER_READ_TEX) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		for (unsigned int i = 0; i < GBUFFER_N_TEXTURES; i++) {
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
		}
	}
}