#include "camera/Undistort.h"

namespace cam {

std::vector<int> buildUndistortMap(const CameraProjector &raw, const CameraProjector &corrected)
{
    const int w = corrected.width(), h = corrected.height();
    std::vector<int> map(size_t(w) * h, -1);
    for (int v = 0; v < h; ++v) {
        for (int u = 0; u < w; ++u) {
            const auto dir = corrected.ray({u + 0.5, v + 0.5});
            if (!dir) continue;
            const auto src = raw.projectDirection(*dir);
            if (!src || src->u < 0 || src->v < 0 || src->u >= raw.width() || src->v >= raw.height()) continue;
            map[size_t(v) * w + u] = int(src->v) * raw.width() + int(src->u);
        }
    }
    return map;
}

std::vector<uint32_t> applyMap(const std::vector<int> &map, const std::vector<uint32_t> &source, uint32_t fill)
{
    std::vector<uint32_t> out(map.size());
    for (size_t i = 0; i < map.size(); ++i)
        out[i] = map[i] >= 0 ? source[size_t(map[i])] : fill;
    return out;
}

} // namespace cam
