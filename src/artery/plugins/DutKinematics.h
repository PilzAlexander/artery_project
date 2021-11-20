//
// Created by vagrant on 11/19/21.
//

#ifndef ARTERY_DUTKINEMATICS_H
#define ARTERY_DUTKINEMATICS_H


#include "artery/utility/Geometry.h"
#include <vanetza/units/acceleration.hpp>
#include <vanetza/units/angle.hpp>
#include <vanetza/units/velocity.hpp>
#include <vanetza/units/angular_velocity.hpp>

// forward declaration
namespace traci { class VehicleController; }

namespace artery
{

/**
 * VehicleKinematics stores attributes describing a vehicle's kinematic
 */
    struct DutKinematics
    {
        DutKinematics();

        Position position;
        GeoPosition geo_position;
        vanetza::units::Velocity speed;
        vanetza::units::Acceleration acceleration;
        vanetza::units::Angle heading; // from north, clockwise
        vanetza::units::AngularVelocity yaw_rate;
    };

    VehicleKinematics getDutKinematics(const traci::VehicleController&);

} // namespace artery

#endif /* ARTERY_VEHICLEKINEMATICS_H_VDOQF76E */
