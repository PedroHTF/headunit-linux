#include "camera/CameraModel.h"
#include "camera/Guidelines.h"
#include "camera/Undistort.h"

#include <QtTest>
#include <cmath>

using namespace cam;

class TestCamera : public QObject
{
    Q_OBJECT

    const CameraPose pose{};
    const CameraProjector fisheye{pose, Lens{150, 0.8}, 640, 360};
    const CameraProjector pinhole{pose, Lens{100, 0.0}, 640, 360};

    static double maxDeviation(const CameraProjector &cam)
    {
        std::vector<Pixel> pts;
        for (int i = 0; i <= 20; ++i)
            if (auto p = cam.project({-1.5 + i * 0.15, 1.2})) pts.push_back(*p);
        const Pixel a = pts.front(), b = pts.back();
        double worst = 0;
        for (const Pixel &p : pts)
            worst = std::max(worst, std::abs((b.v - a.v) * p.u - (b.u - a.u) * p.v + b.u * a.v - b.v * a.u)
                                        / std::hypot(b.v - a.v, b.u - a.u));
        return worst;
    }

private slots:
    void pointBehindIsOnCenterColumn() { QCOMPARE(fisheye.project({0, 2})->u, 320.0); }

    void fartherIsHigher() { QVERIFY(pinhole.project({0, 3})->v < pinhole.project({0, 1})->v); }

    void groundRoundTrip()
    {
        for (const auto *c : {&fisheye, &pinhole}) {
            const auto back = c->toGround(*c->project({0.7, 2.3}));
            QVERIFY(std::abs(back->x - 0.7) < 1e-6);
            QVERIFY(std::abs(back->y - 2.3) < 1e-6);
        }
    }

    void skyHasNoGround() { QVERIFY(!pinhole.toGround({320, 0})); }
    void straightStaysStraightWithoutFisheye() { QVERIFY(maxDeviation(pinhole) < 0.5); }
    void fisheyeBendsLines() { QVERIFY(maxDeviation(fisheye) > 5.0); }

    void undistortCenter()
    {
        const auto map = buildUndistortMap(CameraProjector{pose, Lens{150, 0.8}, 64, 36}, CameraProjector{pose, Lens{100, 0}, 64, 36});
        const int c = map[18 * 64 + 32];
        QVERIFY(std::abs(c % 64 - 32) <= 1);
        QVERIFY(std::abs(c / 64 - 18) <= 1);
    }

    void straightTrajectory()
    {
        const auto path = Guidelines::trajectory(0.9, VehicleDimensions{}, 0.0, 3.0);
        QCOMPARE(path.back().x, 0.9);
        QCOMPARE(path.back().y, 3.0);
    }

    void steeringRightCurvesRight()
    {
        const auto path = Guidelines::trajectory(0.0, VehicleDimensions{}, 20.0, 3.0);
        QVERIFY(path.back().x > 0.2);
        for (size_t i = 1; i < path.size(); ++i) QVERIFY(path[i].y > path[i - 1].y);
    }

    void leftMirrorsRight()
    {
        const auto r = Guidelines::trajectory(0.9, VehicleDimensions{}, 20.0, 3.0).back();
        const auto l = Guidelines::trajectory(-0.9, VehicleDimensions{}, -20.0, 3.0).back();
        QVERIFY(std::abs(r.x + l.x) < 1e-9);
        QVERIFY(std::abs(r.y - l.y) < 1e-9);
    }

    void wheelAngle()
    {
        QCOMPARE(Guidelines::wheelAngleDeg(150, VehicleDimensions{}), 10.0);
        QCOMPARE(Guidelines::wheelAngleDeg(900, VehicleDimensions{}), 35.0);
    }

    void carLengthMarkers()
    {
        VehicleDimensions car;
        car.lengthM = 4.0;
        GuideOptions o;
        o.distanceMarkersM = {1.0};
        o.carLengthMarkers = {1, 2};
        const auto m = Guidelines::markers(car, o);
        QCOMPARE(int(m.size()), 3);
        QCOMPARE(m[1].first, 4.0);
        QCOMPARE(QString::fromStdString(m[1].second), QStringLiteral("1 carro"));
        QCOMPARE(QString::fromStdString(m[2].second), QStringLiteral("2 carros"));
    }

    void invertSteering()
    {
        GuideOptions o;
        o.showStatic = false;
        o.invertSteering = true;
        for (const auto &line : Guidelines::build(VehicleDimensions{}, o, 300))
            QVERIFY(line.points.back().x < line.points.front().x);
    }
};

QTEST_GUILESS_MAIN(TestCamera)
#include "tst_camera.moc"
