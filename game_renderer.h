#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "game_platform.h"

// NOTE(ivan): Game renderer platform-specific initialization parameters.
struct renderer_init_platform_specific;

// NOTE(ivan): Game renderer prototypes.
#define RENDERER_INIT(Name) void Name(renderer_init_platform_specific *PlatformSpecific)
typedef RENDERER_INIT(renderer_init);

#define RENDERER_SHUTDOWN(Name) void Name(void)
typedef RENDERER_SHUTDOWN(renderer_shutdown);

// NOTE(ivan): Game renderer interface.
struct renderer_api {
	renderer_init *Init;
	renderer_shutdown *Shutdown;
};

// NOTE(ivan): Game renderer accessor.
#define RENDERER_GET_API(Name) renderer_api * Name(void)
typedef RENDERER_GET_API(renderer_get_api);

#endif // #ifndef GAME_RENDERER_H
