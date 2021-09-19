#pragma once
#include <Light.h>
#include <DirectionalLight.h>
#include <PointLight.h>
#include "imgui.h"
#include <gtc/type_ptr.hpp>

void LightMenu(PointLight& myPointLight, DirectionalLight& myDirectionalLight);