#include "platform.h"

#ifdef USE_WIN32


DWORD WINAPI ThreadStartProc(LPVOID lpParameter)
{
    thread_create_s p = *(thread_create_s *)lpParameter; // copy to stack
    delete (thread_create_s *)lpParameter;
    p.func(p.par);
    return 0;
}


#endif // USE_WIN32
