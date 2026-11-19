#include "camera/Guidelines.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cam::Guidelines {

namespace {
constexpr double kPi = 3.14159265358979323846;

Ground shift(double x, double y, const GuideOptions &o) { return {x + o.lineOffsetXM, y + o.lineOffsetYM}; }

GuideColor colorFor(double y)
{
    if (y < 0.5) return GuideColor::Red;
    if (y < 1.0) return GuideColor::Yellow;
    return GuideColor::Green;
}

std::string formatMeters(double m)
{
    if (m == std::floor(m)) return std::to_string(int(m)) + " m";
    char buf[16];
    std::snprintf(buf, sizeof buf, "%.1f m", m);
    std::string s(buf);
    std::replace(s.begin(), s.end(), '.', ',');
    return s;
}
} // namespace

double wheelAngleDeg(double steeringWheelDeg, const VehicleDimensions &car)
{
    return std::clamp(steeringWheelDeg / car.steeringRatio, -car.maxWheelAngleDeg, car.maxWheelAngleDeg);
}

std::vector<std::pair<double, std::string>> markers(const VehicleDimensions &car, const GuideOptions &options)
{
    std::vector<std::pair<double, std::string>> out;
    for (double m : options.distanceMarkersM)
        if (m > 0) out.emplace_back(m, formatMeters(m));
    for (int n : options.carLengthMarkers)
        if (n > 0) out.emplace_back(n * car.lengthM, n == 1 ? std::string("1 carro") : std::to_string(n) + " carros");
    std::stable_sort(out.begin(), out.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    std::vector<std::pair<double, std::string>> unique;
    for (const auto &m : out)
        if (unique.empty() || int(unique.back().first * 100) != int(m.first * 100)) unique.push_back(m);
    return unique;
}

std::vector<Ground> trajectory(double startX, const VehicleDimensions &car, double wheelAngle, double lengthM, int steps)
{
    const double delta = wheelAngle * kPi / 180.0;
    if (std::abs(delta) < 1e-4) return {{startX, 0.0}, {startX, lengthM}};
    const double axleY = -car.rearOverhangM;
    const double radius = car.wheelbaseM / std::tan(std::abs(delta));
    const double cx = delta > 0 ? radius : -radius; // volante para a direita: centro à direita
    const double total = lengthM / std::hypot(cx, axleY);
    const double dir = cx > 0 ? 1.0 : -1.0;
    std::vector<Ground> out;
    for (int i = 0; i <= steps; ++i) {
        const double ang = -dir * total * i / steps;
        const double rx = startX - cx, ry = 0.0 - axleY;
        out.push_back({cx + rx * std::cos(ang) - ry * std::sin(ang), axleY + rx * std::sin(ang) + ry * std::cos(ang)});
    }
    return out;
}

std::vector<GuideLine> build(const VehicleDimensions &car, const GuideOptions &o, double steeringWheelDeg)
{
    std::vector<GuideLine> lines;
    const double half = car.widthM / 2;
    const auto marks = markers(car, o);
    double maxY = 2.0;
    if (!marks.empty()) maxY = marks.back().first;
    maxY = std::max(maxY, 0.5);

    if (o.showStatic) {
        std::vector<double> stops{0.0};
        for (const auto &m : marks)
            if (m.first < maxY) stops.push_back(m.first);
        stops.push_back(maxY);
        for (size_t i = 0; i + 1 < stops.size(); ++i) {
            const GuideColor color = colorFor(stops[i]);
            for (double sx : {-half, half})
                lines.push_back({{shift(sx, stops[i], o), shift(sx, stops[i + 1], o)}, color, {}});
        }
        for (const auto &[y, label] : marks) {
            const double tick = car.widthM * 0.22;
            const GuideColor color = colorFor(y - 1e-6);
            lines.push_back({{shift(-half, y, o), shift(-half + tick, y, o)}, color, label});
            lines.push_back({{shift(half - tick, y, o), shift(half, y, o)}, color, {}});
        }
    }

    if (o.showDynamic) {
        const double sign = o.invertSteering ? -1.0 : 1.0;
        const double wheel = wheelAngleDeg(steeringWheelDeg * sign, car);
        for (double sx : {-half, half}) {
            GuideLine line{{}, GuideColor::Blue, {}};
            for (const Ground &g : trajectory(sx, car, wheel, o.dynamicLengthM))
                line.points.push_back(shift(g.x, g.y, o));
            lines.push_back(std::move(line));
        }
    }
    return lines;
}

} // namespace cam::Guidelines
