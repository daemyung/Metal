//
// This file is part of the "Metal" project
// See "LICENSE" for license information.
//

#include "window.h"

#include <imgui_impl_osx.h>

#include "example.h"

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
        auto view = (__bridge View *)displayLinkContext;
        auto example = view->example;
        if (example) {
            example->Update();
            example->Render();
        }
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

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)setFrameSize:(NSSize)size {
    [super setFrameSize:size];
    if (example) {
        example->Resize({size.width, size.height});
    }
}

- (void)windowWillClose:(NSNotification *)notification {
    CVDisplayLinkStop(_display_link);
    CVDisplayLinkRelease(_display_link);
    if (example) {
        example->Term();
    }
}

- (void)viewDidMoveToWindow {
    [super viewDidMoveToWindow];

    CVDisplayLinkCreateWithActiveCGDisplays(&_display_link);
    CVDisplayLinkSetOutputCallback(_display_link, &DisplayLinkOutputCallback, (__bridge void *)self);
    CVDisplayLinkSetCurrentCGDisplay(_display_link, [self GetDirectDisplayID]);
    CVDisplayLinkStart(_display_link);
}

- (void)mouseMoved:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
    if (example) {
        example->OnMouseMove(event, [self GetMousePoint:event]);
    }
}

- (void)mouseDown:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
    if (example) {
        example->OnMouseButtonDown(event, [self GetMousePoint:event]);
    }
}

- (void)rightMouseDown:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)otherMouseDown:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)mouseUp:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
    if (example) {
        example->OnMouseButtonDown(event, [self GetMousePoint:event]);
    }
}

- (void)rightMouseUp:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)otherMouseUp:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)mouseDragged:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
    if (example) {
        example->OnMouseMove(event, [self GetMousePoint:event]);
    }
}

- (void)rightMouseDragged:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)otherMouseDragged:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)scrollWheel:(NSEvent *)event {
    ImGui_ImplOSX_HandleEvent(event, self);
    if (example) {
        example->OnMouseWheel(-[event deltaY]);
    }
}

- (void)keyDown:(NSEvent*)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (void)keyUp:(NSEvent*)event {
    ImGui_ImplOSX_HandleEvent(event, self);
}

- (CGDirectDisplayID)GetDirectDisplayID {
    return (CGDirectDisplayID)[self.window.screen.deviceDescription[@"NSScreenNumber"] unsignedIntegerValue];
}

- (NSPoint)GetMousePoint:(NSEvent *)event {
    auto point = [self convertPoint:event.locationInWindow fromView:nil];
    point.y = self.bounds.size.height - point.y;
    return point;
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
    example->BindToWindow(this);
    example->Init();
    example->Resize(GetResolution());

    _view->example = example;
    [NSApp run];
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
