// This file is mostly borrowed from glfw. At least the hard bits were
// bugs my fault probably
//========================================================================
// GLFW 3.3 macOS - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2016 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
#include "core/core.h"
#include "shell/macshell.h"
#include "input/provider.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include <thread>
#include <carbon/carbon.h>
#import <Cocoa/Cocoa.h>
#include <sys/param.h> // For MAXPATHLEN

typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardLayoutInputSource)(void);
typedef void* (*PFN_TISGetInputSourceProperty)(TISInputSourceRef,CFStringRef);
typedef UInt8 (*PFN_LMGetKbdType)(void);

namespace Shell {

class MacEventHandler : public Input::Mouse, public Input::Keyboard
{
public:
	auto OnEvent(CGEventRef event) -> void;

	static bool IsMouseEvent(CGEventType type);
	static bool IsKeyboardEvent(CGEventType type);
};

struct MacPresentationWindow
{
	MacPresentationWindow(PresentableWindowConfig const& config_);

	id layer;
	bool shouldClose = false;
	std::unique_ptr<MacEventHandler> eventHandler;
};

struct MacSpecific
{
	MacSpecific();
	~MacSpecific();

	id object;
	id delegate;
	id view;
	id listener;
	id autoreleasePool;
	CGEventSourceRef eventSource;

	std::vector<std::shared_ptr<MacPresentationWindow>> windows;
};

} // end shell namespace (Objective C has to be global namespace)

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

@interface GLFWWindowDelegate : NSObject
{
	Shell::MacPresentationWindow* window;
}

- (instancetype)initWithWindow:(Shell::MacPresentationWindow *)initWindow;

@end

@implementation GLFWWindowDelegate

- (instancetype)initWithWindow:(Shell::MacPresentationWindow *)initWindow
{
	self = [super init];
	if (self != nil)
		window = initWindow;

	return self;
}

- (BOOL)windowShouldClose:(id)sender
{
	window->shouldClose = true;
	return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
}

- (void)windowDidMove:(NSNotification *)notification
{
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
}

- (void)windowDidResignKey:(NSNotification *)notification
{
}
@end
@interface WyrdContentView : NSView <NSTextInputClient>
{
	Shell::MacPresentationWindow* w;
	NSTrackingArea* trackingArea;
	NSMutableAttributedString* markedText;
}

- (instancetype)initWithWindow:(Shell::MacPresentationWindow *)initWindow;

@end

@implementation WyrdContentView

- (instancetype)initWithWindow:(Shell::MacPresentationWindow *)initWindow
{
	self = [super init];
	if (self != nil)
	{
		w = initWindow;
		trackingArea = nil;
		markedText = [[NSMutableAttributedString alloc] init];

		[self updateTrackingAreas];
	}

	return self;
}

- (void)dealloc
{
	[trackingArea release];
	[markedText release];
	[super dealloc];
}

- (BOOL)isOpaque
{
	return YES;
}

- (BOOL)canBecomeKeyView
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)wantsUpdateLayer
{
	return YES;
}

- (void)updateLayer
{
//	_glfwInputWindowDamage(window);
}

- (id)makeBackingLayer
{
	if (w->layer)
		return w->layer;

	return [super makeBackingLayer];
}

- (void)cursorUpdate:(NSEvent *)event
{
//	updateCursorImage(window);
}

- (void)mouseDown:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)mouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)mouseMoved:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)rightMouseDown:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)rightMouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)otherMouseDown:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)otherMouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)mouseExited:(NSEvent *)event
{
}

- (void)mouseEntered:(NSEvent *)event
{
}

- (void)viewDidChangeBackingProperties
{
/*	const NSRect contentRect = [window->ns.view frame];
	const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

	if (fbRect.size.width != window->ns.fbWidth ||
		fbRect.size.height != window->ns.fbHeight)
	{
		window->ns.fbWidth  = fbRect.size.width;
		window->ns.fbHeight = fbRect.size.height;
		_glfwInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
	}

	const float xscale = fbRect.size.width / contentRect.size.width;
	const float yscale = fbRect.size.height / contentRect.size.height;

	if (xscale != window->ns.xscale || yscale != window->ns.yscale)
	{
		window->ns.xscale = xscale;
		window->ns.yscale = yscale;
		_glfwInputWindowContentScale(window, xscale, yscale);

		if (window->ns.layer)
			[window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];
	}*/
}

- (void)updateTrackingAreas
{
	if (trackingArea != nil)
	{
		[self removeTrackingArea:trackingArea];
		[trackingArea release];
	}

	const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
										  NSTrackingActiveInKeyWindow |
										  NSTrackingEnabledDuringMouseDrag |
										  NSTrackingCursorUpdate |
										  NSTrackingInVisibleRect |
										  NSTrackingAssumeInside;

	trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
												options:options
												  owner:self
											   userInfo:nil];

	[self addTrackingArea:trackingArea];
	[super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
	[self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)flagsChanged:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)keyUp:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}

- (void)scrollWheel:(NSEvent *)event
{
	w->eventHandler->OnEvent([event CGEvent]);
}


- (BOOL)hasMarkedText
{
	return [markedText length] > 0;
}

- (NSRange)markedRange
{
	if ([markedText length] > 0)
		return NSMakeRange(0, [markedText length] - 1);
	else
		return NSMakeRange(0,0);
}

- (NSRange)selectedRange
{
	return NSMakeRange(0,0);
}

- (void)setMarkedText:(id)string
		selectedRange:(NSRange)selectedRange
	 replacementRange:(NSRange)replacementRange
{
	[markedText release];
	if ([string isKindOfClass:[NSAttributedString class]])
		markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
	else
		markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
	[[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
	return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
											   actualRange:(NSRangePointer)actualRange
{
	return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
	return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
						 actualRange:(NSRangePointer)actualRange
{
/*	int xpos, ypos;
	_glfwPlatformGetWindowPos(window, &xpos, &ypos);
	const NSRect contentRect = [window->ns.view frame];
	return NSMakeRect(xpos, transformY(ypos + contentRect.size.height), 0.0, 0.0);
 */
	return NSMakeRect(0,0,0.0,0.0);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
/*	NSString* characters;
	NSEvent* event = [NSApp currentEvent];
	const int mods = translateFlags([event modifierFlags]);
	const int plain = !(mods & GLFW_MOD_SUPER);

	if ([string isKindOfClass:[NSAttributedString class]])
		characters = [string string];
	else
		characters = (NSString*) string;

	NSUInteger i, length = [characters length];

	for (i = 0;  i < length;  i++)
	{
		const unichar codepoint = [characters characterAtIndex:i];
		if ((codepoint & 0xff00) == 0xf700)
			continue;

		_glfwInputChar(window, codepoint, mods, plain);
	}
 */
}

- (void)doCommandBySelector:(SEL)selector
{
}

@end

@interface WyrdApplication : NSApplication
{
}

@end

@implementation WyrdApplication

// No-op thread entry point
//
- (void)doNothing:(id)object
{
}
@end

@interface WyrdApplicationDelegate : NSObject
- (instancetype)initWithMacSpecific:(Shell::MacSpecific *)initSpecific;

@end

@implementation WyrdApplicationDelegate

Shell::MacSpecific* m;
- (instancetype)initWithMacSpecific:(Shell::MacSpecific *)initSpecific;
{
	self = [super init];
	if (self != nil)
		m = initSpecific;

	return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	for(auto& window : m->windows)
	{
		window->shouldClose = true;
	}

	return NSTerminateCancel;
}
@end

@interface WyrdLayoutListener : NSObject

@end

@implementation WyrdLayoutListener

TISInputSourceRef inputSource;
void* unicodeData;
CFBundleRef     bundle;

PFN_TISCopyCurrentKeyboardLayoutInputSource CopyCurrentKeyboardLayoutInputSource;
PFN_TISGetInputSourceProperty GetInputSourceProperty;
PFN_LMGetKbdType GetKbdType;
CFStringRef     kPropertyUnicodeKeyLayoutData;
- (void) dealloc
{
	if (inputSource)
	{
		CFRelease(inputSource);
		inputSource = nullptr;
		unicodeData = nullptr;
	}

	[super dealloc];
}

- (void) updateUnicodeDataNS
{
	if (inputSource)
	{
		CFRelease(inputSource);
		inputSource = nullptr;
		unicodeData = nullptr;
	}

	inputSource = CopyCurrentKeyboardLayoutInputSource();
	if (!inputSource)
	{
		LOG_S(WARNING) << "Cocoa: Failed to retrieve keyboard layout input source";
	}

	unicodeData = GetInputSourceProperty(inputSource, kPropertyUnicodeKeyLayoutData);
	if (!unicodeData)
	{
		LOG_S(WARNING) << "Cocoa: Failed to retrieve keyboard layout Unicode data";
	}
}

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object {
	[self updateUnicodeDataNS];
}
// Load HIToolbox.framework and the TIS symbols we need from it
//
- (void)initializeTIS
{
	// This works only because Cocoa has already loaded it properly
	bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.HIToolbox"));
	if (!bundle)
	{
		LOG_S(ERROR) << "Cocoa: Failed to load HIToolbox.framework";
	}

	CFStringRef* kPropertyUnicodeKeyLayoutData = (CFStringRef*)
			CFBundleGetDataPointerForName(bundle,CFSTR("kTISPropertyUnicodeKeyLayoutData"));

	CopyCurrentKeyboardLayoutInputSource = (PFN_TISCopyCurrentKeyboardLayoutInputSource)
			CFBundleGetFunctionPointerForName(bundle, CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));

	GetInputSourceProperty = (PFN_TISGetInputSourceProperty)
			CFBundleGetFunctionPointerForName(bundle, CFSTR("TISGetInputSourceProperty"));
	GetKbdType = (PFN_LMGetKbdType)
			CFBundleGetFunctionPointerForName(bundle, CFSTR("LMGetKbdType"));

	if (!kPropertyUnicodeKeyLayoutData ||
		!CopyCurrentKeyboardLayoutInputSource ||
		!GetInputSourceProperty ||
		!GetKbdType)
	{
		LOG_S(ERROR) << "Cocoa: Failed to load TIS API symbols";
	}

	[self updateUnicodeDataNS];
}

@end


namespace Input {
Keyboard* g_Keyboard = nullptr;
Mouse* g_Mouse = nullptr;
}

namespace Shell {

MacSpecific::MacSpecific()
{
	autoreleasePool = [[NSAutoreleasePool alloc] init];

	delegate = [[WyrdApplicationDelegate alloc] initWithMacSpecific:this];

	[WyrdApplication sharedApplication];

	// Make Cocoa enter multi-threaded mode
	[NSThread detachNewThreadSelector:@selector(doNothing:)
							 toTarget:NSApp
						   withObject:nil];


	listener = [[WyrdLayoutListener alloc] init];
	[[NSNotificationCenter defaultCenter]
			addObserver:listener
			   selector:@selector(selectedKeyboardInputSourceChanged:)
				   name:NSTextInputContextKeyboardSelectionDidChangeNotification
				 object:nil];

	eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
	CGEventSourceSetLocalEventsSuppressionInterval(eventSource, 0.0);

	[listener initializeTIS];

	[NSApp setDelegate:delegate];
	[NSApp run];
}

MacSpecific::~MacSpecific()
{
	if (eventSource)
	{
		CFRelease(eventSource);
		eventSource = nullptr;
	}

	if (delegate)
	{
		[NSApp setDelegate:nil];
		[delegate release];
		delegate = nil;
	}

	if (listener)
	{
		[[NSNotificationCenter defaultCenter]
				removeObserver:listener
						  name:NSTextInputContextKeyboardSelectionDidChangeNotification
						object:nil];
		[[NSNotificationCenter defaultCenter]
				removeObserver:listener];
		[listener release];
		listener = nil;
	}

	[autoreleasePool release];
	autoreleasePool = nil;
}

MacPresentationWindow::MacPresentationWindow(PresentableWindowConfig const& config_)
{
	eventHandler = std::make_unique<MacEventHandler>();

	Input::g_Keyboard = static_cast<Input::Keyboard*>(eventHandler.get());
	Input::g_Mouse = static_cast<Input::Mouse*>(eventHandler.get());

}
auto MacEventHandler::OnEvent(CGEventRef event) -> void
{
	using namespace Input;
	CGEventType type = CGEventGetType(event);

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

			default: assert(false);
		}
	}
}

bool MacEventHandler::IsMouseEvent(CGEventType type)
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

bool MacEventHandler::IsKeyboardEvent(CGEventType type)
{
	return type == kCGEventKeyUp ||
		   type == kCGEventKeyDown ||
		   type == kCGEventFlagsChanged;
}

MacShell::MacShell()
{
	m = new MacSpecific();
}

MacShell::~MacShell()
{
	delete m;
}

auto MacShell::createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow*
{
	auto w = new MacPresentationWindow(config_);


	return (PresentableWindow*)w;
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