#pragma once
#include "imgui.h"
#include <Renderer.h>
#include <tinyfiledialogs.h>

void RendererInitSetting(Renderer &renderer);

void LoadMenu(Renderer &renderer);

void RendererMenu(Renderer &renderer);

void DebugMenu(Renderer &renderer);

void SVOGIDebugMenu(Renderer& renderer);

void ModelMenu(Renderer& renderer);