#pragma once

#include "math.h"

// Kierunek do księżyca (światło nocne padające na scenę).
inline constexpr Vec3 kMoonLightDir = {0.14f, 0.90f, 0.22f};
inline constexpr Vec3 kMoonLightColor = {0.55f, 0.68f, 0.95f};
inline constexpr Vec3 kNightAmbient = {0.035f, 0.045f, 0.07f};
inline constexpr Vec3 kNightFogColor = {0.008f, 0.025f, 0.05f};
