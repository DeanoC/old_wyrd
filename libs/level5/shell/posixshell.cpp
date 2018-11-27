#include "core/core.h"
#include "shell/posixshell.h"

#if PLATFORM == POSIX || PLATFORM == APPLE_MAC
namespace Shell
{

auto PosixShell::init(ShellConfig const& init_) -> bool
{
	return ConsoleApp::init(init_);
}

auto PosixShell::update() -> bool
{
	return ConsoleApp::update();
}

auto PosixShell::ensureConsoleWindowsExists() -> void
{
	ConsoleApp::ensureConsoleWindowsExists();
}

auto PosixShell::createPresentableWindow(PresentableWindowConfig const& config_)->PresentableWindow*
{
	return nullptr;
}

void PosixShell::destroyPresentableWindow(PresentableWindow* window_)
{

}

std::unique_ptr<Input::Provider> PosixShell::getInputProvider(PresentableWindow* window_)
{
	return nullptr;
}

int main(int argc, char **argv)
{
	using namespace Shell;
	PosixShell app;
	app.sysInit(argc, argv);

	int result = Main((ShellInterface&) app);

	return result;
}


}
#endif