#include "DutKinematics.h"
#include "artery/application/VehicleKinematics.h"
#include "artery/traci/VehicleController.h"
#include <limits>

namespace artery
{
    DutKinematics::DutKinematics() :
            acceleration(vanetza::units::Acceleration::from_value(std::numeric_limits<double>::quiet_NaN())),
            yaw_rate(vanetza::units::AngularVelocity::from_value(std::numeric_limits<double>::quiet_NaN()))
    {
    }

    DutKinematics getDutKinematics(const traci::VehicleController& controller)
    {
        std::cout << "ID" << controller.getVehicleId() << std::endl;
        artery::VehicleKinematics kinematics;
        kinematics.position = controller.getPosition();
        kinematics.geo_position = controller.getGeoPosition();
        kinematics.speed = controller.getSpeed();
        kinematics.heading = controller.getHeading().getTrueNorth();
        return kinematics;
    }

} // namespace artery