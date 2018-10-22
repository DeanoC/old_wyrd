#include "core/core.h"
#include "shell/shell.h"
#include "shell/cmdline.h"

#if !defined(WYRD_SHELL_NOSHELL)

#if !defined(WYRD_SHELL_APP_NAME)
#define WYRD_SHELL_APP_NAME "Wyrd App"
#endif

#if PLATFORM == WINDOWS
#if !defined(WYRD_SHELL_CONSOLE_APP)
#if _NTDDI_VERSION >= NTDDI_WINBLUE

#include <ShellScalingAPI.h>
#include <fcntl.h>
#include <io.h>

#endif
namespace {

char g_moduleFilename[_MAX_PATH];
HINSTANCE g_hInstance;
HINSTANCE g_hPrevInstance;
HWND g_hWnd = NULL;
int g_nCmdShow;
static const int MAX_CMDLINE_ARGS = 1024;   //!< Max cmdline args we support
int g_argc;                            		//!< global cmdline arg count
char *g_argv[MAX_CMDLINE_ARGS];            //!< global cmdline args
}

// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole()
{
	using namespace std;
	int hConHandle;
	long lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console
	lStdHandle = (long) GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

	// point to console as well
	ios::sync_with_stdio();

}

//!-----------------------------------------------------
//! Pumps windows messages, also pauses here when we have
//! lost focus
//!-----------------------------------------------------
void WinMainCmdLineToMainArgs(char *command_line)
{
	char *arg;
	int index;

	// early out if already set up (either double call or via a main -> winmain
	// thunk from a command line app
	if(g_argc != 0)
	{
		return;
	}

	// count the arguments
	g_argc = 1;
	arg = command_line;

	while(arg[0] != 0)
	{
		while(arg[0] != 0 && arg[0] == ' ')
		{
			arg++;
		}

		if(arg[0] != 0)
		{
			g_argc++;
			while(arg[0] != 0 && arg[0] != ' ')
			{
				arg++;
			}
		}
	}

	if(g_argc > MAX_CMDLINE_ARGS)
	{
		g_argc = MAX_CMDLINE_ARGS;
	}

	// tokenize the arguments
	arg = command_line;
	index = 1;

	while(arg[0] != 0)
	{
		while(arg[0] != 0 && arg[0] == ' ')
		{
			arg++;
		}

		if(arg[0] != 0)
		{
			g_argv[index] = arg;
			index++;

			while(arg[0] != 0 && arg[0] != ' ')
			{
				arg++;
			}

			if(arg[0] != 0)
			{
				arg[0] = 0;
				arg++;
			}
		}
	}

	// put the program name into argv[0]
	GetModuleFileNameA(NULL, g_moduleFilename, _MAX_PATH);
	g_argv[0] = g_moduleFilename;
}


//! Pumps windows messages, also pauses here when we have
//! lost focus
void WinGetMessages(void)
{
	if(g_hWnd == NULL)
	{
		return;
	}

	MSG Message;
	while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WinMainCmdLineToMainArgs(lpCmdLine);

//#if _NTDDI_VERSION >= NTDDI_WINBLUE
//	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
//#else
//	SetProcessDPIAware();
//#endif

	RedirectIOToConsole();

	g_hInstance = hInstance;
	g_hPrevInstance = hPrevInstance;
	g_nCmdShow = nCmdShow;

	using namespace Shell;
	CmdLine app(g_argc, g_argv);

	int result = Main((ShellInterface&) app);

	return result;
}

#endif
#endif // end PLATFORM == WINDOWS

int main(int argc, char const *const *argv)
{
	using namespace Shell;
	CmdLine app(argc, argv);

	int result = Main((ShellInterface&) app);

	return result;
}


#endif // end !WYRD_SHELL_NOSHELL