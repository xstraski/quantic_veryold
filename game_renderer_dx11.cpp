#include "game_renderer.h"
#include "game_platform_win32.h"

renderer_state RendererState = {};

static RENDERER_INIT(RendererInit) {
	HRESULT HR;

	IDXGIFactory *Factory;
	HR = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&Factory);
	if (SUCCEEDED(HR)) {
		IDXGIAdapter *Adapter;
		HR = Factory->EnumAdapters(0, &Adapter);
		if (SUCCEEDED(HR)) {
			
		}
	}
}

static RENDERER_SHUTDOWN(RendererShutdown) {
}

extern "C"
RENDERER_GET_API(RendererGetAPI) {
	static renderer_api RendererAPI = {};

	RendererAPI.Init = RendererInit;
	RendererAPI.Shutdown = RendererShutdown;
	
	return &RendererAPI;
}
