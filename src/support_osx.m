#include <Cocoa/Cocoa.h>

/* Bring the application to the front when starting it from the terminal */
void nori_raise_osx() {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication]; /* Creates a connection to the windowing environment */
	[NSApp activateIgnoringOtherApps:YES]; /* Pop to front */
	[pool release];
}
