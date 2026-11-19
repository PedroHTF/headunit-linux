#pragma once

#include "camera/CameraModel.h"

#include <cstdint>
#include <vector>

namespace cam {

/**
 * Remove o fisheye: para cada pixel da imagem corrigida, o índice (v·largura + u) do pixel
 * de origem na imagem crua, ou −1. Calculado uma vez; aplicado a cada quadro.
 */
std::vector<int> buildUndistortMap(const CameraProjector &raw, const CameraProjector &corrected);

std::vector<uint32_t> applyMap(const std::vector<int> &map, const std::vector<uint32_t> &source,
                               uint32_t fill = 0xFF000000u);

} // namespace cam
