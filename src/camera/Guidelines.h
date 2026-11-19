#pragma once

#include "camera/CameraModel.h"

#include <string>
#include <utility>
#include <vector>

namespace cam {

/** Medidas do carro (metros). */
struct VehicleDimensions
{
    double widthM = 1.75;
    double lengthM = 4.30;
    double wheelbaseM = 2.60;
    double rearOverhangM = 0.85;     // eixo traseiro → para-choque
    double steeringRatio = 15.0;     // graus de volante por grau de roda
    double maxWheelAngleDeg = 35.0;
};

struct GuideOptions
{
    bool showStatic = true;
    bool showDynamic = true;
    std::vector<double> distanceMarkersM{0.5, 1.0, 2.0};
    std::vector<int> carLengthMarkers{1};
    double lineOffsetXM = 0.0;
    double lineOffsetYM = 0.0;
    bool invertSteering = false;
    double dynamicLengthM = 3.0;
};

enum class GuideColor { Red, Yellow, Green, Blue, White };

struct GuideLine
{
    std::vector<Ground> points;
    GuideColor color;
    std::string label; // vazio = sem rótulo
};

namespace Guidelines {

double wheelAngleDeg(double steeringWheelDeg, const VehicleDimensions &car);

std::vector<GuideLine> build(const VehicleDimensions &car, const GuideOptions &options, double steeringWheelDeg);

/** (distância, rótulo) em ordem; marcas de "carros" usam o comprimento do carro. */
std::vector<std::pair<double, std::string>> markers(const VehicleDimensions &car, const GuideOptions &options);

/** Caminho do canto do para-choque andando de ré (modelo de bicicleta). */
std::vector<Ground> trajectory(double startX, const VehicleDimensions &car, double wheelAngleDeg,
                               double lengthM, int steps = 24);

} // namespace Guidelines
} // namespace cam
