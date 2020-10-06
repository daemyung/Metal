//
// This file is part of the "Metal" project
// See "LICENSE" for license information.
//

#include "window.h"

#include <QuartzCore/CAMetalLayer.h>

//----------------------------------------------------------------------------------------------------------------------

@interface AppDelegate : NSObject<NSApplicationDelegate> {
}
@end

//----------------------------------------------------------------------------------------------------------------------

@implementation AppDelegate {
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
@end

//----------------------------------------------------------------------------------------------------------------------

static CVReturn DisplayLinkOutputCallback(CVDisplayLinkRef displayLink,
                                          const CVTimeStamp *inNow,
                                          const CVTimeStamp *inOutputTime,
                                          CVOptionFlags flagsIn,
                                          CVOptionFlags *flagsOut,
                                          void *displayLinkContext) {
    @autoreleasepool {
        auto example = static_cast<Example *>(displayLinkContext);
        // Do render.
    }

    return kCVReturnSuccess;
}

//----------------------------------------------------------------------------------------------------------------------

@implementation View {
    CVDisplayLinkRef _display_link;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:(frameRect)];
    if (self) {
        self.wantsLayer = YES;
    }
    return self;
}

- (CALayer *)makeBackingLayer {
    return [CAMetalLayer layer];
}

- (void)windowWillClose:(NSNotification *)notification {
    if (_display_link) {
        CVDisplayLinkStop(_display_link);
        CVDisplayLinkRelease(_display_link);
    }
}

- (void)MainLoop:(Example*) example {
    CVDisplayLinkCreateWithActiveCGDisplays(&_display_link);
    CVDisplayLinkSetOutputCallback(_display_link, &DisplayLinkOutputCallback, example);
    CVDisplayLinkSetCurrentCGDisplay(_display_link, [self GetDirectDisplayID]);
    CVDisplayLinkStart(_display_link);
    [NSApp run];
}

- (CGDirectDisplayID)GetDirectDisplayID {
    return (CGDirectDisplayID)[self.window.screen.deviceDescription[@"NSScreenNumber"] unsignedIntegerValue];
}

@end

//----------------------------------------------------------------------------------------------------------------------

Window* Window::GetInstance() {
    static std::unique_ptr<Window> window(new Window());
    return window.get();
}

//----------------------------------------------------------------------------------------------------------------------

Window::~Window() {
}

//----------------------------------------------------------------------------------------------------------------------

void Window::MainLoop(Example *example) {
    [_view MainLoop:example];
}

//----------------------------------------------------------------------------------------------------------------------

Resolution Window::GetResolution() const {
    const auto& size = _view.bounds.size;
    return {size.width, size.height};
}

//----------------------------------------------------------------------------------------------------------------------

Window::Window() {
    InitApplication();
    InitWindow(kFHDResolution);
    InitView();
}

//----------------------------------------------------------------------------------------------------------------------

void Window::InitApplication() {
    if (!NSApp) {
        NSApp = [NSApplication sharedApplication];
        if (!NSApp) {
            std::runtime_error("Fail to get an application");
        }

        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp setDelegate:[AppDelegate new]];
    }
}

//----------------------------------------------------------------------------------------------------------------------

void Window::InitWindow(const Resolution &resolution) {
    const auto kContentRect = NSMakeRect(0.0f, 0.0f, GetWidth(resolution), GetHeight(resolution));
    const auto kWindowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

    _window = [[NSWindow alloc] initWithContentRect:kContentRect
                                              styleMask:kWindowStyle
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    if (!_window) {
        throw std::runtime_error("Fail to initialize a window");
    }
    
    [_window setTitle:@"Metal"];
    [_window setAcceptsMouseMovedEvents:YES];
    [_window center];
    [_window makeKeyAndOrderFront:nil];
}

//----------------------------------------------------------------------------------------------------------------------

void Window::InitView() {
    _view = [[View alloc] initWithFrame:_window.contentLayoutRect];
    if (!_view) {
        throw std::runtime_error("Fail to initialize a view.");
    }

    [_window setDelegate:_view];
    [_window setContentView:_view];
}

//----------------------------------------------------------------------------------------------------------------------
