#include "camera/SyntheticScene.h"

#include <cmath>

namespace cam {

namespace {
constexpr uint32_t kSky = 0xFF2E3A48, kAsphalt = 0xFF3B3E43, kAsphaltDark = 0xFF34373B, kPaint = 0xFFE8E8E0,
                   kCurb = 0xFFB0A890, kGridA = 0xFF50555C, kGridB = 0xFF2A2D31, kBumper = 0xFF15171A;

uint32_t grid(Ground g)
{
    const int cx = int(std::floor(g.x)), cy = int(std::floor(g.y));
    return ((cx + cy) & 1) == 0 ? kGridA : kGridB;
}

uint32_t parking(Ground g)
{
    if (g.y > 5.5) return kCurb;
    const double m = std::fmod(std::fmod(g.x + 1.25, 2.5) + 2.5, 2.5);
    const bool stallLine = m < 0.06 || 2.5 - m < 0.06;
    if (stallLine && g.y > 0.3) return kPaint;
    if (std::abs(g.y - 5.2) < 0.06) return kPaint;
    return (int(std::floor(g.x * 2) + std::floor(g.y * 2)) & 1) == 0 ? kAsphalt : kAsphaltDark;
}
} // namespace

std::vector<uint32_t> renderScene(const CameraProjector &camera, Scene scene)
{
    std::vector<uint32_t> out(size_t(camera.width()) * camera.height());
    for (int v = 0; v < camera.height(); ++v) {
        for (int u = 0; u < camera.width(); ++u) {
            const auto g = camera.toGround({u + 0.5, v + 0.5});
            uint32_t c;
            if (!g) c = kSky;
            else if (g->y < 0) c = kBumper;
            else c = scene == Scene::Grid ? grid(*g) : parking(*g);
            out[size_t(v) * camera.width() + u] = c;
        }
    }
    return out;
}

} // namespace cam
