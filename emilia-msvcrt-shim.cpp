#include "emilia-msvcrt-shim.h"

#if defined(__MINGW32__) && !defined(_UCRT)
#include <cstdlib>

namespace
{
constexpr int MaxQuickExitHandlers = 32;
void (*quickExitHandlers[MaxQuickExitHandlers])(void);
int quickExitHandlerCount;
}

extern "C" int at_quick_exit(void (*handler)(void))
{
	if (quickExitHandlerCount >= MaxQuickExitHandlers)
		return -1;
	quickExitHandlers[quickExitHandlerCount++] = handler;
	return 0;
}

/* libmsvcrt-os.a's snprintf alias object references asm symbol
 * "__ms_vsnprintf" while libmingwex.a exports "___ms_vsnprintf" on i386;
 * bridge the prefix gap over msvcrt's _vsnprintf with C99 termination. */
#include <cstdarg>
#include <cstdio>
extern "C" int reactball_ms_vsnprintf(char* buffer, size_t count, const char* format, va_list args)
	asm("___ms_vsnprintf");
extern "C" int reactball_ms_vsnprintf(char* buffer, size_t count, const char* format, va_list args)
{
	int written = _vsnprintf(buffer, count, format, args);
	if (buffer != nullptr && count > 0)
		buffer[count - 1] = '\0';
	return written;
}

extern "C" void quick_exit(int status)
{
	for (int index = quickExitHandlerCount - 1; index >= 0; index--)
		quickExitHandlers[index]();
	_exit(status);
}
#endif
