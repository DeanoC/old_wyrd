
#include "core/core.h"
#include "shell/winshell.h"
#include "input/provider.h"

#if PLATFORM == WINDOWS

#include <fcntl.h>
#include <io.h>
#include "input/keyboard.h"
#include "input/mouse.h"

namespace Input {
Keyboard* g_Keyboard = nullptr;
Mouse* g_Mouse = nullptr;
}

namespace Shell {
static bool g_windowsQuit = false;

struct Win32PresentationWindow
{
	HINSTANCE hinstance;
	HWND hwnd;
};


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(Input::Keyboard::WinProcessMessages(message, wParam, lParam)) return 0;
	if(Input::Mouse::WinProcessMessages(message, wParam, lParam)) return 0;

	switch(message)
	{
		case WM_DESTROY:
			g_windowsQuit = true;
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			ValidateRect(hWnd, NULL);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

struct WindowsWinMainCreationParams
{
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLine;
	int nCmdShow;
};

struct WindowsSpecific
{
	auto createStandardArgs(LPSTR command_line) -> void;
	auto getMessages(void) -> void;
	HINSTANCE hInstance;

	std::vector<Win32PresentationWindow> windows;
	int nCmdShow;

	static const int MAX_CMDLINE_ARGS = 1024;
	int argc;
	char* argv[MAX_CMDLINE_ARGS];
	char moduleFilename[_MAX_PATH];
};

auto WindowsSpecific::createStandardArgs(LPSTR command_line) -> void
{
	char* arg;
	int index;

	// count the arguments
	argc = 1;
	arg = command_line;

	while(arg[0] != 0)
	{
		while(arg[0] != 0 && arg[0] == ' ')
		{
			arg++;
		}

		if(arg[0] != 0)
		{
			argc++;
			while(arg[0] != 0 && arg[0] != ' ')
			{
				arg++;
			}
		}
	}

	if(argc > MAX_CMDLINE_ARGS)
	{
		argc = MAX_CMDLINE_ARGS;
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
			argv[index] = arg;
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
	argv[0] = moduleFilename;
}

//! Pumps windows messages
auto WindowsSpecific::getMessages(void) -> void
{
	if(windows.empty())
	{
		MSG Message;
		while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	} else
	{
		for(auto[instance, hwnd] : windows)
		{
			MSG Message;
			while(PeekMessage(&Message, hwnd, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}
}

WinShell::WinShell(WindowsWinMainCreationParams* params)
{
	w = new WindowsSpecific{};
	GetModuleFileNameA(NULL, w->moduleFilename, _MAX_PATH);
	w->createStandardArgs(params->lpCmdLine);
	w->hInstance = params->hInstance;
	w->nCmdShow = params->nCmdShow;

	sysInit(w->argc, w->argv);
}

WinShell::~WinShell()
{
	delete w;
}

auto WinShell::ensureConsoleWindowsExists() -> void
{
	// maximum mumber of lines the output console should have
	static const WORD MAX_CONSOLE_LINES = 500;

	using namespace std;
	int hConHandle;
	HANDLE lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console
	lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

	// point to console as well
	ios::sync_with_stdio();

}

auto WinShell::init(ShellConfig const& init_) -> bool
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = w->hInstance;
	wcex.hIcon = NULL;//LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "Wyrd";
	wcex.hIconSm = NULL;//LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
	if(!RegisterClassEx(&wcex))
		return false;

	return ConsoleApp::init(init_);
}

auto WinShell::update() -> bool
{
	w->getMessages();
	if(g_windowsQuit) return false;
	return ConsoleApp::update();
};

auto WinShell::createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow*
{
	if(config_.directInput)
	{
		if(Input::g_Keyboard == nullptr)
		{
			Input::g_Keyboard = new Input::Keyboard();
		}
		if(Input::g_Mouse == nullptr)
		{
			Input::g_Mouse = new Input::Mouse();
		}
	}

	// Create window
	RECT rc = {0, 0, (LONG) config_.width, (LONG) config_.height};
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD styleEx = 0;
	if(config_.fullscreen == false)
	{
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	}

	HWND hwnd = CreateWindowEx(styleEx,
							   "Wyrd",
							   config_.windowName.c_str(),
							   style,
							   0, 0, rc.right - rc.left, rc.bottom - rc.top,
							   nullptr,
							   nullptr,
							   w->hInstance,
							   nullptr);
	if(!hwnd) return nullptr;
	w->getMessages();
	ShowWindow(hwnd, w->nCmdShow);
	w->getMessages();

	return (PresentableWindow*) &w->windows.emplace_back(Win32PresentationWindow{w->hInstance, hwnd});
}

auto WinShell::destroyPresentableWindow(PresentableWindow* window_) -> void
{
	if(window_ == nullptr) return;
	auto window = (Win32PresentationWindow*)(window_);
	for(auto& win : w->windows)
	{
		if(win.hwnd == window->hwnd)
		{
			DestroyWindow(window->hwnd);
			win.hwnd = {};
		}
	}

}

auto WinShell::getInputProvider(PresentableWindow* window_) -> std::unique_ptr<Input::Provider>
{
	// make sure we have the global and mouse handlers on windows
	if(Input::g_Keyboard == nullptr)
	{
		Input::g_Keyboard = new Input::Keyboard();
	}
	if(Input::g_Mouse == nullptr)
	{
		Input::g_Mouse = new Input::Mouse();
	}

	// TODO lots of stuff here, for now we just have a basic vpad via
	// wasd and mouse emulations

	return std::move(Input::Provider::WinCreateProvider());
}

} // end namespace Shell

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	using namespace Shell;
	WindowsWinMainCreationParams params;
	params.hInstance = hInstance;
	params.hPrevInstance = hPrevInstance;
	params.lpCmdLine = lpCmdLine;
	params.nCmdShow = nCmdShow;

	WinShell shell(&params);
	int result = Main((ShellInterface&) shell);

	if(Input::g_Keyboard != nullptr)
	{
		delete Input::g_Keyboard;
	}

	if(Input::g_Mouse != nullptr)
	{
		delete Input::g_Mouse;
	}

	return result;
}

#endif // end PLATFORM == WINDOWS
