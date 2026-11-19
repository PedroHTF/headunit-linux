#pragma once

#include "hal/VehicleHal.h"

/** Regras de segurança que dependem do estado do carro. */
namespace DrivingRules {

/** Acima desta velocidade o vídeo é ocultado (o áudio continua). */
constexpr double VideoSpeedLimitKmh = 5.0;

/** @param allowWhileDriving configuração do usuário "vídeo em qualquer velocidade". */
inline bool isVideoAllowed(double speedKmh, bool allowWhileDriving = false)
{
    return allowWhileDriving || speedKmh < VideoSpeedLimitKmh;
}

inline bool shouldShowReverseCamera(VehicleHal::Ignition ignition, VehicleHal::Gear gear)
{
    return VehicleHal::isPowered(ignition) && gear == VehicleHal::Gear::Reverse;
}

} // namespace DrivingRules
