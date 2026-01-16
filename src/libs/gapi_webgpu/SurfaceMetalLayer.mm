#include "SurfaceMetalLayer.h"
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace RR::GAPI::WebGPU {
    void* GetOrCreateMetalLayer(void* nsWindowPtr) {
        NSWindow* nsWindow = (NSWindow*)nsWindowPtr;
        NSView* contentView = [nsWindow contentView];

        if ([contentView.layer isKindOfClass:[CAMetalLayer class]]) {
            return (void*)contentView.layer;
        } else {
            CAMetalLayer* metalLayer = [CAMetalLayer layer];
            [contentView setLayer:metalLayer];
            [contentView setWantsLayer:YES];
            return metalLayer;
        }
    }
}
