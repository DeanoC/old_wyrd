#define LOGURU_IMPLEMENTATION 1
#define CX_ERROR_IMPLEMENTATION 1
#include "core/core.h"
#include "shell/consoleapp.h"
#include "vulkan/system.h"

namespace Shell {
ConsoleApp::~ConsoleApp()
{
	if(userInit.gpuComputeRequired || userInit.gpuRenderRequired)
	{
		gpuStable->Finish();
		gpuStable.reset();
	}

}

ConsoleApp::ConsoleApp()
{
}

auto ConsoleApp::sysInit(int argc_, char** argv_) -> void
{
	loguru::g_stderr_verbosity = 0;    // 0 (INFO) by default.
	loguru::g_colorlogtostderr = true; // If you don't want color in your terminal.
	loguru::g_flush_interval_ms = 0;    // Unbuffered (0) by default.
	loguru::g_preamble = false; // Prefix each log line with date, time etc?

	// Turn off individual parts of the preamble
	loguru::g_preamble_date = false; // The date field
	loguru::g_preamble_time = false; // The time of the current day
	loguru::g_preamble_uptime = true; // The time since init call
	loguru::g_preamble_thread = false; // The logging thread
	loguru::g_preamble_file = false; // The file from which the log originates from
	loguru::g_preamble_verbose = false; // The verbosity field
	loguru::g_preamble_pipe = true; // The pipe symbol right before the message

	loguru::init(argc_, argv_);
	// Put every log message in "everything.log":
	loguru::add_file("everything.log", loguru::Truncate, loguru::Verbosity_MAX);
	// Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
	loguru::add_file("latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);

	arguments.resize(argc_);
	for(auto i = 0; i < argc_; ++i)
	{
		arguments[i] = std::string(argv_[i]);
	}


}

auto ConsoleApp::getArguments() const -> std::vector<std::string> const&
{
	return arguments;
}

auto ConsoleApp::init(ShellConfig const& init_) -> bool
{
	userInit = init_;

	if(init_.wantConsoleOutput)
	{
		ensureConsoleWindowsExists();
	}

	if(gpusInitialised())
	{
		// at the moment only vulkan supported
		gpuStable = std::make_unique<Vulkan::System>();
		bool okay = gpuStable->Init( userInit.appName, userInit.gpuBackendSpecific );
		if(okay == false)
		{
			userInit.gpuRenderRequired = false;
			userInit.gpuComputeRequired = false;
			return false;
		}
	}

	return true;
}

auto ConsoleApp::update() -> bool
{
	bool appQuit = false;
	return !appQuit;
}

} // end namespace