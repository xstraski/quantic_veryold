#ifndef GAME_RENDERER_DX11_H
#define GAME_RENDERER_DX11_H

#include "game_renderer.h"

// NOTE(ivan): Game DX11 renderer globals.
extern struct renderer_state {
	IDXGISwapChain *SwapChain;
	ID3D11Device *Device;
	ID3D11DeviceContext *DeviceContext;
} RendererState;

#endif // #ifndef GAME_RENDERER_DX11_H
