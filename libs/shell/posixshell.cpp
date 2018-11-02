

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

auto PosixShell::createPresentableWindow()->PresentableWindow* final
{
	return nullptr;
}

int main(int argc, char **argv)
{
	using namespace Shell;
	PosixShell app;
	app.sysInit(arc, argv)

	int result = Main((ShellInterface&) app);

	return result;
}


}
#endif