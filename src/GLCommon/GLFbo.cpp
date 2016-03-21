

#include "GLFbo.h"





GLFbo::GLFbo():mMultisampling(false)
{
	mpFbo[0] = NULL;
	mpFbo[1] = NULL;
	
	mpFboMulti[0] = NULL;
	mpFboMulti[1] = NULL;

	mpTex[0] = NULL;
	mpTex[1] = NULL;
}


GLFbo::~GLFbo()
{
}


bool GLFbo::IsValid(int field) const
{
	if(field==0)
		return (mpFbo[0]!=NULL && mpFbo[0]->valid);
	else
		return (mpFbo[1]!=NULL && mpFbo[1]->valid);
}


Fbo* GLFbo::GetFbo(int field) const
{
	return mpFbo[field];
}

gl::Texture* GLFbo::GetTexture(int field) const
{
	return &(*mpTex[field]);
}

int GLFbo::Width() const
{
	return mWidth;
}

int GLFbo::Height() const
{
	return mHeight;
}



void GLFbo::Initialize(int w, int h, 
						int sizeBits,           // bits per component
						int num_samples,		// number of samples
						GLboolean alpha,        // alpha
						GLboolean depth,		// depth -> can be false for render_buffer_ext
						gl::Texture* pTex_0,	// texture object - field 0
						gl::Texture* pTex_1)	// texture object - field 1         
{
	bool lSuccess = false;

	mWidth = w;
	mHeight = h;

	mpTex[0] = &(*pTex_0);
	mpTex[1] = &(*pTex_1);

	for(int i=0; i<2; ++i)
	{		
		mpFbo[i] = new Fbo();
		
		if(mpTex[i])
			lSuccess = mpFbo[i]->create(w, h, 8, 1, alpha, depth, mpTex[i]->Id(), mpTex[i]->Type());
		else
			lSuccess = mpFbo[i]->create(w, h, 8, 1, alpha, depth, NULL);
		mpFbo[i]->unbind();


		if(!lSuccess)
			throw(gl::exception("Could not create Fbo"));

		if(num_samples>1)	// Enable Multisample
		{
			mMultisampling = true;
			
			mpFboMulti[i] = new Fbo();	// Create multisampled FBO
			
			if(mpTex[i])
 				lSuccess = mpFboMulti[i]->create(w, h, 8, num_samples, alpha, depth, NULL);
			else
				lSuccess = mpFboMulti[i]->create(w, h, 8, num_samples, alpha, depth, NULL);

			mpFboMulti[i]->unbind();
		}
		else
		{
			mMultisampling = false;
		}

		if(!lSuccess)
			throw(gl::exception("Could not create Fbo Multisampled"));
	}

	
}


void GLFbo::Uninitialize()
{
	for(int i=0; i<2; ++i)
	{
		mpFbo[i]->destroy();
		mpFbo[i] = NULL;

		if (mMultisampling)
		{
			mpFboMulti[i]->destroy();
			mpFboMulti[i] = NULL;
		}
	}
}



void GLFbo::BeginRender(int field)
{
	if(mpFbo[field])
	{
		// push the projection matrix and the entire GL state before
		// doing any rendering into our framebuffer object
		//glPushAttrib(GL_VIEWPORT_BIT|GL_TEXTURE_BIT);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		if(mpTex[field])
			mpTex[field]->Unbind();
		
		if(!mMultisampling)
		{
			mpFbo[field]->bind(mWidth, mHeight);
		}
		else
		{
			mpFboMulti[field]->bind(mWidth, mHeight);
			glEnable(GL_MULTISAMPLE);
		}
	}
}


void GLFbo::EndRender(int field)
{
	if(mpFbo[field])
	{
		if(!mMultisampling)
		{
			mpFbo[field]->unbind();
		}
		else
		{
			// If using multisample render buffer, then blit to downsample and filter
			mpFboMulti[field]->bindRead(mWidth, mHeight);
			mpFbo[field]->bindDraw(mWidth, mHeight);
			
			CheckGLError("<GLFbo::EndDraw -  >")

			mpFbo[field]->blit(mWidth, mHeight);
			
			CheckGLError("<GLFbo::EndDraw - blit>")
			
			glDisable(GL_MULTISAMPLE);
			
			// Unbind FBOs
			mpFboMulti[field]->unbind();
			mpFbo[field]->unbind();
		}
		
		// pop the projection matrix and GL state back for rendering
		// to the actual widget
		glPopAttrib();
	}
}

