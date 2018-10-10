#include "core/core.h"
#include "shell/shell.h"
#include "shell/cmdline.h"

#if !defined(WYRD_SHELL_NOSHELL)

#if !defined(WYRD_SHELL_APP_NAME)
#define WYRD_SHELL_APP_NAME "Wyrd App"
#endif

#if PLATFORM == WINDOWS
#if !defined(WYRD_SHELL_CONSOLE_APP)
#else
#endif
#endif // end PLATFORM == WINDOWS

int main(int argc, char const* const *argv)
{
	using namespace Shell;
	CmdLine app(argc, argv);

	int result = Main((ShellInterface&)app);

	return result;
}


#endif // end !WYRD_SHELL_NOSHELL