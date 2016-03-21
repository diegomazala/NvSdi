#include "nvGPUutil.h"
#include "glExtensions.h"

bool CreateDummyGLWindow(HWND * hWnd, HGLRC * hGLRC)
{
		    
    HINSTANCE hInstance = GetModuleHandle(NULL);	// Need a handle to this process instance    		
    // Create the window class
	WNDCLASSEX	wc;						// Windows Class Structure
    wc.cbSize			= sizeof(WNDCLASSEX); 
    wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc		= DefWindowProc;			    // WndProc Handles Messages, we'll pass a static version, which will receive a 'this' pointer
    wc.cbClsExtra		= 0;									// No Extra Window Data
    wc.cbWndExtra		= 0;									// No Extra Window Data
    wc.hInstance		= hInstance;							// Set The Instance
    wc.hIcon			= NULL;									// Load The Default Icon
    wc.hIconSm		    = NULL; 
    wc.hCursor			= LoadCursor(hInstance, IDC_ARROW);		// Load The Arrow Pointer
    wc.hbrBackground	= NULL;									// No Background Required For GL
    wc.lpszMenuName		= NULL;									// We Don't Want A Menu
    wc.lpszClassName	= "Dummy";						// Set The Class Name

    // register the window class
    RegisterClassEx(&wc);
	    
 	    // Call the windows function to create the window.  The 
    *hWnd = CreateWindowEx(	NULL, "Dummy", NULL, NULL, 0, 0, 1, 1,	NULL, NULL, NULL, NULL);								
            	  
    // Get the windows device context
    HDC hDC=GetDC(*hWnd);	
      	
    ///// STEP 2: Now we need to create an OpenGL context for this window

    // We need a pixel format descriptor.  A PFD tells how OpenGL draws
    static	PIXELFORMATDESCRIPTOR pfd=				
    {
	    sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
	    1,											// Version Number
	    PFD_DRAW_TO_WINDOW |						// Format Must Support Window
	    PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
	    PFD_DOUBLEBUFFER,							// Must Support Double Buffering
	    PFD_TYPE_RGBA,								// Request An RGBA Format
	    8,										    // Select Our Color Depth
	    0, 0, 0, 0, 0, 0,							// Color Bits Ignored
	    0,											// No Alpha Buffer
	    0,											// Shift Bit Ignored
	    0,											// No Accumulation Buffer
	    0, 0, 0, 0,									// Accumulation Bits Ignored
	    16,											// 16Bit Z-Buffer (Depth Buffer)  
	    0,											// No Stencil Buffer
	    0,											// No Auxiliary Buffer
	    PFD_MAIN_PLANE,								// Main Drawing Layer
	    0,											// Reserved
	    0, 0, 0										// Layer Masks Ignored
    };
	
    int pixelformat;			
    // create the pixel pixel format descriptor
    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 ) 
	{ 
		return false;
	}
    //  set the pixel format descriptor
    if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) 
	{   
		return false;
	}
    // Create a wGL rendering context
    *hGLRC=wglCreateContext(hDC);	    
    //  Activate the rendering context
    wglMakeCurrent(hDC,*hGLRC);	
	
	return true;

}

bool DestroyGLWindow(HWND * hWnd, HGLRC * hGLRC)
{
	HDC hDC=GetDC(*hWnd);
	wglMakeCurrent(NULL,NULL);	
	wglDeleteContext(*hGLRC);
	if(ReleaseDC(*hWnd,hDC) == 0)
		return false;
	if(DestroyWindow(*hWnd) == FALSE)
		return false;
	return true;
}
//
//CNvGpuTopology
//
CNvGpuTopology::CNvGpuTopology()
{
	m_bInitialized = false;
	m_nGpu = 0;
	if(init())
		m_bInitialized = true;
}

CNvGpuTopology::~CNvGpuTopology()
{
	if(!m_bInitialized)
		return;
	for( int i = 0; i < m_nGpu; i++)
	{
		delete m_lGpu[i];
		m_lGpu[i] = NULL;
	}
}

bool CNvGpuTopology::init()
{
	if(m_bInitialized)
		return true;

	HWND hWnd;
	HGLRC hGLRC;
	if(CreateDummyGLWindow(&hWnd,&hGLRC) == false)
		return false;

	if(!loadAffinityExtension())
	{
		printf("Could not load OpenGL Affinity extension\n");
		return false;
	}

	printf("\n\nListing Gpus available for OpenGL Gpu Affinity\n");
	
	unsigned int GPUIdx = 0;    

	GPU_DEVICE gpuDevice;
	gpuDevice.cb = sizeof(gpuDevice);	
	HGPUNV hGPU;
	bool bDisplay;
	bool bPrimary;
	while(wglEnumGpusNV(GPUIdx,&hGPU))	// First call this function to get a handle to the gpu
	{									    // wglEnumPGUsNV will fails if DeviceIdx > the available devices
        printf("GPU# %d:\n",GPUIdx);        
		bDisplay = false;
		bPrimary = false;
		// Now get the detailed information about this device:
		//how many displays it's attached to and whethre any of them 
        int DisplayDeviceIdx = 0;
		while(wglEnumGpuDevicesNV(hGPU,DisplayDeviceIdx,&gpuDevice))
		{			
            printf(" Display# %d\n",DisplayDeviceIdx);
			printf("   Name: %s\n",gpuDevice.DeviceName);
			printf("   String: %s\n",gpuDevice.DeviceString);
			
			bDisplay = true;
			
			if(gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
                printf("   Attached to the desktop: LEFT=%d, RIGHT=%d, TOP=%d, BOTTOM=%d\n",
					gpuDevice.rcVirtualScreen.left, gpuDevice.rcVirtualScreen.right, gpuDevice.rcVirtualScreen.top, gpuDevice.rcVirtualScreen.bottom);
			}
			else
			{
				printf("   Not attached to the desktop\n");
			}
		  
            // See if it's the primary GPU
            if(gpuDevice.Flags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			{
				printf("   This is the PRIMARY Display Device\n");
				bPrimary = true;
			}						
			DisplayDeviceIdx++;
			
		}
		CNvGpu *gpu = new CNvGpu();
		gpu->init(hGPU,bPrimary,bDisplay);

		m_lGpu[GPUIdx] = gpu;
		
		GPUIdx++;
	}

	m_nGpu = GPUIdx;
	m_bInitialized = true;

	// We can kill the dummy window now
	if(DestroyGLWindow(&hWnd,&hGLRC) == false)
		return false;
	

	return true;

}


CNvGpuTopology& CNvGpuTopology::instance()
{		
	static CNvGpuTopology instance;
	instance.init();
	return instance;
}


int CNvGpuTopology::getNumGpu()
{
	return m_nGpu;
}

CNvGpu *CNvGpuTopology::getGpu(int index)
{
	if(index >= 0 && index < m_nGpu)
		return m_lGpu[index];
	return NULL;
}

int   CNvGpuTopology::getPrimaryGpuIndex()
{
	for(int i = 0;i < m_nGpu;i++)
	{
		if(m_lGpu[i]->isPrimary())
			return i;
	}
	return 0;
}

CNvGpu *CNvGpuTopology::getPrimaryGpu()
{
	for(int i = 0;i < m_nGpu;i++)
	{
		if(m_lGpu[i]->isPrimary())
			return m_lGpu[i];
	}
	return NULL;

}


//
//CNvGpu
//


bool  CNvGpu::init(HGPUNV gpuAffinityHandle,bool bPrimary, bool bDisplay)
{
	m_hGpu = gpuAffinityHandle;
	m_bPrimary = bPrimary;	
	m_bDisplay = bDisplay;
	return true;
}

