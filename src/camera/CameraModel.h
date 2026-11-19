#pragma once

#include <cmath>
#include <optional>

/**
 * Geometria da câmera de ré (C++ puro, sem Qt). Mesmo modelo do headunit-android.
 *
 * Sistema de coordenadas do chão (metros), visto de cima:
 *  - origem: no chão, no centro do para-choque traseiro;
 *  - +Y: para trás do carro (para onde a câmera olha);
 *  - +X: para a direita **da imagem** (câmeras de ré espelham a imagem, como um retrovisor).
 */
namespace cam {

struct Ground { double x = 0, y = 0; };
struct Pixel { double u = 0, v = 0; };

struct Vec3
{
    double x = 0, y = 0, z = 0;
    Vec3 operator-(const Vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator+(const Vec3 &o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator*(double k) const { return {x * k, y * k, z * k}; }
    double dot(const Vec3 &o) const { return x * o.x + y * o.y + z * o.z; }
};

struct CameraPose
{
    double heightM = 0.9;   // altura da lente
    double pitchDeg = 30.0; // inclinação para baixo
    double yawDeg = 0.0;    // rotação para os lados
    double rollDeg = 0.0;   // giro da imagem
    double offsetXM = 0.0;  // deslocamento lateral
};

/**
 * Lente: fisheye de 0 (comum, retas ficam retas) a 1 (equidistante).
 * Modelo r = tan(c·θ)/c, com c = 1 − fisheye.
 */
struct Lens
{
    double horizontalFovDeg = 150.0;
    double fisheye = 0.8;

    double radius(double theta) const;
    double theta(double radius) const;
    double maxTheta() const;

private:
    double c() const;
};

/** Projeta chão → imagem e o caminho inverso. Serve para a imagem crua e a corrigida. */
class CameraProjector
{
public:
    CameraProjector(const CameraPose &pose, const Lens &lens, int width, int height);

    std::optional<Pixel> project(Ground g) const { return projectPoint({g.x, g.y, 0.0}); }
    std::optional<Pixel> projectPoint(const Vec3 &world) const { return projectDirection(world - m_center); }
    std::optional<Pixel> projectDirection(const Vec3 &d) const;
    std::optional<Vec3> ray(Pixel px) const;
    /** Ponto do chão visto no pixel (vazio se é céu). */
    std::optional<Ground> toGround(Pixel px) const;

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    Lens m_lens;
    int m_width;
    int m_height;
    Vec3 m_center;
    Vec3 m_forward, m_right, m_up;
    double m_focal;
};

} // namespace cam
