////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	GLNvSdiUnity3D.cpp
///	
/// \author	Diego
/// \date	October 2012
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GLNvSdi.h"
#include "GLTeapot.h"
#include "glExtensions.h"
#include "GLFbo.h"
#include "EnumString.h"
#include <math.h>


// Graphics device identifiers in Unity
enum GfxDeviceRenderer
{
	kGfxRendererOpenGL = 0,          // OpenGL
	kGfxRendererD3D9,                // Direct3D 9
	kGfxRendererD3D11,               // Direct3D 11
	kGfxRendererGCM,                 // Sony PlayStation 3 GCM
	kGfxRendererNull,                // "null" device (used in batch mode)
	kGfxRendererHollywood,           // Nintendo Wii
	kGfxRendererXenon,               // Xbox 360
	kGfxRendererOpenGLES,            // OpenGL ES 1.1
	kGfxRendererOpenGLES20Mobile,    // OpenGL ES 2.0 mobile variant
	kGfxRendererMolehill,            // Flash 11 Stage3D
	kGfxRendererOpenGLES20Desktop,   // OpenGL ES 2.0 desktop variant (i.e. NaCl)
	kGfxRendererCount
};


// Event types for UnitySetGraphicsDevice
enum GfxDeviceEventType 
{
	kGfxDeviceEventInitialize = 0,
	kGfxDeviceEventShutdown,
	kGfxDeviceEventBeforeReset,
	kGfxDeviceEventAfterReset,
};


extern "C"
{

	static int g_DeviceType = -1;

	
	static float g_Time;
	GLNVSDI_API void SetTimeFromUnity (float t) { g_Time = t; }

	GLNVSDI_API void UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
	{
		// Set device type to -1, i.e. "not recognized by our plugin"
		g_DeviceType = -1;


		// If we've got an OpenGL device, remember device type. There's no OpenGL
		// "device pointer" to remember since OpenGL always operates on a currently set
		// global context.
		if (deviceType == kGfxRendererOpenGL)
		{
			//DebugLog ("Set OpenGL graphics device\n");
			g_DeviceType = deviceType;
		}
	}



	// --------------------------------------------------------------------------
	// SetDefaultGraphicsState
	//
	// Helper function to setup some "sane" graphics state. Rendering state
	// upon call into our plugin can be almost completely arbitrary depending
	// on what was rendered in Unity before.
	// Before calling into the plugin, Unity will set shaders to null,
	// and will unbind most of "current" objects (e.g. VBOs in OpenGL case).
	//
	// Here, we set culling off, lighting off, alpha blend & test off, Z
	// comparison to less equal, and Z writes off.

	static void SetDefaultGraphicsState ()
	{
		// OpenGL case
		if (g_DeviceType == kGfxRendererOpenGL)
		{
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_BLEND);
			glDisable (GL_ALPHA_TEST);
			glDepthFunc (GL_LEQUAL);
			glEnable (GL_DEPTH_TEST);
			glDepthMask (GL_FALSE);
		}
	}

	// --------------------------------------------------------------------------
	// UnityRenderEvent
	// This will be called for GL.IssuePluginEvent script calls; eventID will
	// be the integer passed to IssuePluginEvent. In this example, we just ignore
	// that value.

	GLNVSDI_API void UnityRenderEvent (int eventID)
	{
		// Unknown graphics device type? Do nothing.
		if (g_DeviceType == -1)
			return;


		// A colored triangle. Note that colors will come out differently
		// in D3D9 and OpenGL, for example, since they expect color bytes
		// in different ordering.
		struct MyVertex {
			float x, y, z;
			unsigned int color;
		};
		MyVertex verts[3] = {
			{ -0.5f, -0.25f,  0, 0xFFff0000 },
			{  0.5f, -0.25f,  0, 0xFF00ff00 },
			{  0,     0.5f ,  0, 0xFF0000ff },
		};


		// Some transformation matrices: rotate around Z axis for world
		// matrix, identity view matrix, and identity projection matrix.

		float phi = g_Time;
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);

		float worldMatrix[16] = {
			cosPhi,-sinPhi,0,0,
			sinPhi,cosPhi,0,0,
			0,0,1,0,
			0,0,0.7f,1,
		};
		float identityMatrix[16] = {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1,
		};
		float projectionMatrix[16] = {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1,
		};

		// Setup sane graphics state (a helper function above)
		SetDefaultGraphicsState ();


		// Now do actual rendering!



		// OpenGL case
		if (g_DeviceType == kGfxRendererOpenGL)
		{
			// Transformation matrices
			glMatrixMode (GL_MODELVIEW);
			glLoadMatrixf (worldMatrix);
			glMatrixMode (GL_PROJECTION);
			// Tweak the projection matrix a bit to make it match what identity
			// projection would do in D3D9 case.
			projectionMatrix[10] = 2.0f;
			projectionMatrix[14] = -1.0f;
			glLoadMatrixf (projectionMatrix);

			// Vertex layout
			glVertexPointer (3, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
			glEnableClientState (GL_VERTEX_ARRAY);
			glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(verts[0]), &verts[0].color);
			glEnableClientState (GL_COLOR_ARRAY);

			// Draw!
			glDrawArrays (GL_TRIANGLES, 0, 3);
		}
	}

	
	
};	//extern "C"