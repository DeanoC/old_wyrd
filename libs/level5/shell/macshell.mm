
#include "core/core.h"
#include "shell/macshell.h"
#include "input/provider.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include <thread>
#import <Cocoa/Cocoa.h>


@interface WyrdWindow : NSWindow {}
@end
@implementation WyrdWindow
- (BOOL)canBecomeKeyWindow
{
	// Required for NSWindowStyleMaskBorderless windows
	return YES;
}
- (BOOL)canBecomeMainWindow
{
	return YES;
}
@end

@interface WyrdApplicationDelegate : NSObject
@end

@implementation WyrdApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
//	for (window = _glfw.windowListHead;  window;  window = window->next)
//		_glfwInputWindowCloseRequest(window);

	return NSTerminateCancel;
}
@end

// No-op thread entry point
//
- (void)doNothing:(id)object
{
}

namespace Input {
Keyboard* g_Keyboard = nullptr;
Mouse* g_Mouse = nullptr;
}

namespace Shell {
static bool g_macQuit = false;

static std::unique_ptr<std::thread> eventThread;

struct MacPresentationWindow
{

};

class MacEventHandler : public Input::Mouse, public Input::Keyboard
{
public:
	auto OnEvent(CGEventTapProxy proxy, CGEventType type, CGEventRef event) -> CGEventRef
	{
		using namespace Input;

		if(IsKeyboardEvent(type))
		{
			CGKeyCode scanCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

		}

		if(IsMouseEvent(type))
		{
			switch(type)
			{
				case kCGEventLeftMouseDragged:
				case kCGEventLeftMouseDown:
				case kCGEventRightMouseDragged:
				case kCGEventRightMouseDown:
				case kCGEventOtherMouseDragged:
				case kCGEventOtherMouseDown:
				{
					int64_t button =  CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
					int64_t clickType = CGEventGetIntegerValueField(event,kCGMouseEventClickState);
					if(buttonDown(MouseButton(button)))
					{
						buttonState[button] |= Mouse::ButtonHeldFlag;
					}
					else
					{
						buttonState[button] |= Mouse::ButtonDownFlag;
					}

					// double and triple click count as a double click
					if(clickType > 1)
						buttonState[button] |= Mouse::ButtonDoubleClickFlag;
					break;
				}
				case kCGEventLeftMouseUp:
				case kCGEventRightMouseUp:
				case kCGEventOtherMouseUp:
				{
					int64_t button =  CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
					buttonState[button] = 0;
					break;
				}

				case kCGEventScrollWheel:
				{
					mouseWheel[0] = (float) CGEventGetDoubleValueField(event, kCGScrollWheelEventDeltaAxis1);
					mouseWheel[1] = (float) CGEventGetDoubleValueField(event, kCGScrollWheelEventDeltaAxis2);
					break;
				}

				case kCGEventMouseMoved:
				{
					CGPoint location = CGEventGetLocation(event);
					absMousePos[0] = location.x;
					absMousePos[1] = location.y;

					int64_t deltaX =  CGEventGetIntegerValueField(event, kCGMouseEventDeltaX);
					int64_t deltaY =  CGEventGetIntegerValueField(event, kCGMouseEventDeltaY);
					break;
				}

				default: assert(false); return nullptr;
			}
		}
		return nullptr;
	}

	static bool IsMouseEvent(CGEventType type)
	{
		return type == kCGEventLeftMouseDown ||
			   type == kCGEventLeftMouseUp ||
			   type == kCGEventRightMouseDown ||
			   type == kCGEventRightMouseUp ||
			   type == kCGEventMouseMoved ||
			   type == kCGEventLeftMouseDragged ||
			   type == kCGEventRightMouseDragged ||
			   type == kCGEventScrollWheel;
	}

	static bool IsKeyboardEvent(CGEventType type)
	{
		return type == kCGEventKeyUp ||
			   type == kCGEventKeyDown ||
			   type == kCGEventFlagsChanged;
	}
};
struct MacSpecific
{
	id object;
	id delegate;
	id view;
	id layer;

	std::vector<MacPresentationWindow> windows;
	std::unique_ptr<MacEventHandler> eventHandler;
};

CGEventRef OnEvent(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* context)
{
	MacEventHandler* eventHandler = static_cast<MacEventHandler*>(context);
	return eventHandler->OnEvent(proxy, type, event);
}

void EventLoopFunc()
{
	CFRunLoopRef ref = CFRunLoopGetCurrent();
	CGEventMask mask =
			CGEventMaskBit(kCGEventLeftMouseDown) |
			CGEventMaskBit(kCGEventLeftMouseUp) |
			CGEventMaskBit(kCGEventRightMouseDown) |
			CGEventMaskBit(kCGEventRightMouseUp) |
			CGEventMaskBit(kCGEventMouseMoved) |
			CGEventMaskBit(kCGEventLeftMouseDragged) |
			CGEventMaskBit(kCGEventRightMouseDragged) |
			CGEventMaskBit(kCGEventScrollWheel);

	CFMachPortRef tap = CGEventTapCreate(
			kCGHIDEventTap,
			kCGHeadInsertEventTap,
			kCGEventTapOptionListenOnly,
			mask,
			OnEvent,
			nullptr);

	CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
	CFRunLoopAddSource(ref, source, kCFRunLoopCommonModes);
	CGEventTapEnable(tap, true);

	CFRunLoopRun();

	CGEventTapEnable(tap, false);
	CFRunLoopRemoveSource(ref, source, kCFRunLoopCommonModes);
	CFRelease(source);
	CFRelease(tap);
}

MacShell::MacShell()
{
	m = new MacSpecific();
	m->eventHandler = std::make_unique<MacEventHandler>();

	[WyrdApplicationDelegate sharedApplication];
	// Make Cocoa enter multi-threaded mode
	[NSThread detachNewThreadSelector:@selector(doNothing:)
							 toTarget:NSApp
						   withObject:nil];
	m->delegate = [[WyrdApplicationDelegate alloc] init];

	[NSApp setDelegate:m->delegate];
	[NSApp run];

	Input::g_Keyboard = static_cast<Input::Keyboard*>(m->eventHandler.get());
	Input::g_Mouse = static_cast<Input::Mouse*>(m->eventHandler.get());
	eventThread = std::make_unique<std::thread>(&EventLoopFunc);
}

MacShell::~MacShell()
{

	eventThread->join();
	eventThread.reset();

	delete m;
}

auto MacShell::createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow*
{
	return nullptr;
}

auto MacShell::destroyPresentableWindow(PresentableWindow* window_) -> void
{
	if(window_ == nullptr) return;
}

auto MacShell::getInputProvider(PresentableWindow* window_) -> std::unique_ptr<Input::Provider>
{
	return std::move(Input::Provider::MacCreateProvider());
}
auto MacShell::init(ShellConfig const& init_) -> bool
{
	return true;
}

auto MacShell::update() -> bool
{
	return true;
}

auto MacShell::ensureConsoleWindowsExists() -> void
{

}

} // end namespace Shell

int main(int argc, char **argv)
{
	using namespace Shell;
	Shell::MacShell app;
	app.sysInit(argc, argv);

	int result = Main((ShellInterface&) app);

	return result;
}