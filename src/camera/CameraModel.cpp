#include "camera/CameraModel.h"

#include <algorithm>

namespace cam {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMinC = 1e-3;
constexpr double kMaxArg = kPi / 2 * 0.98;
double rad(double deg) { return deg * kPi / 180.0; }
} // namespace

double Lens::c() const { return std::clamp(1.0 - fisheye, kMinC, 1.0); }

double Lens::radius(double t) const
{
    const double cc = c();
    if (cc <= kMinC) return t;
    return std::tan(std::min(cc * t, kMaxArg)) / cc;
}

double Lens::theta(double r) const
{
    const double cc = c();
    if (cc <= kMinC) return r;
    return std::atan(cc * r) / cc;
}

double Lens::maxTheta() const
{
    const double cc = c();
    if (cc <= kMinC) return kPi;
    return std::min(kMaxArg / cc, kPi);
}

CameraProjector::CameraProjector(const CameraPose &pose, const Lens &lens, int width, int height)
    : m_lens(lens), m_width(width), m_height(height), m_center{pose.offsetXM, 0.0, pose.heightM}
{
    const double p = rad(pose.pitchDeg), y = rad(pose.yawDeg), r = rad(pose.rollDeg);
    const Vec3 f0{std::sin(y) * std::cos(p), std::cos(y) * std::cos(p), -std::sin(p)};
    const Vec3 r0{std::cos(y), -std::sin(y), 0.0};
    const Vec3 u0{std::sin(y) * std::sin(p), std::cos(y) * std::sin(p), std::cos(p)};
    m_right = r0 * std::cos(r) + u0 * std::sin(r);
    m_up = u0 * std::cos(r) - r0 * std::sin(r);
    m_forward = f0;
    m_focal = (width / 2.0) / lens.radius(rad(lens.horizontalFovDeg / 2));
}

std::optional<Pixel> CameraProjector::projectDirection(const Vec3 &d) const
{
    const double xc = d.dot(m_right), yc = d.dot(m_up), zc = d.dot(m_forward);
    const double theta = std::atan2(std::hypot(xc, yc), zc);
    if (theta >= m_lens.maxTheta()) return std::nullopt;
    const double phi = std::atan2(yc, xc);
    const double r = m_focal * m_lens.radius(theta);
    return Pixel{m_width / 2.0 + r * std::cos(phi), m_height / 2.0 - r * std::sin(phi)};
}

std::optional<Vec3> CameraProjector::ray(Pixel px) const
{
    const double dx = px.u - m_width / 2.0, dy = m_height / 2.0 - px.v;
    const double theta = m_lens.theta(std::hypot(dx, dy) / m_focal);
    if (theta >= m_lens.maxTheta()) return std::nullopt;
    const double phi = std::atan2(dy, dx), s = std::sin(theta);
    return m_right * (s * std::cos(phi)) + m_up * (s * std::sin(phi)) + m_forward * std::cos(theta);
}

std::optional<Ground> CameraProjector::toGround(Pixel px) const
{
    const auto d = ray(px);
    if (!d || d->z >= -1e-6) return std::nullopt;
    const double t = -m_center.z / d->z;
    const Vec3 p = m_center + *d * t;
    return Ground{p.x, p.y};
}

} // namespace cam
