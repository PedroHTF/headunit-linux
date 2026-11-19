#pragma once

#include "camera/CameraModel.h"

#include <cstdint>
#include <vector>

namespace cam {

enum class Scene { Parking, Grid };

/**
 * Imagem falsa da câmera, gerada pelo próprio modelo (cada pixel vira um raio que acerta o
 * chão). Com fisheye, as retas do chão saem curvas, como numa câmera real.
 */
std::vector<uint32_t> renderScene(const CameraProjector &camera, Scene scene);

} // namespace cam
