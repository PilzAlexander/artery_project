/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_OTAINTERFACELAYER_H
#define ARTERY_OTAINTERFACELAYER_H

#include "artery/networking/GeoNetPacket.h"
#include "artery/utility/Geometry.h"
#include <omnetpp/csimplemodule.h>
#include <vanetza/units/velocity.hpp>
#include "artery/application/VehicleDataProvider.h"

namespace traci { class VehicleController; }

namespace artery
{

class OtaInterface;

class OtaInterfaceLayer : public omnetpp::cSimpleModule, public omnetpp::cListener
{
public:
    /**
     * Initializes the module and registers at the OtaInterface
     */
    void initialize(int) override;
    int numInitStages() const override { return 2; }
    void finish() override;

    /**
     * Receives messages from lower layers and handles it to the OtaInterface
     * \param msg Message to be transmitted to OtaInterface
     */
    void handleMessage(omnetpp::cMessage* msg) override;

    /**
     * Handles GN.DataRequest received from the OTA interface
     * \param GeoNetPacket to be sent over the OMNeT++ channel
     */
    void request(std::unique_ptr<GeoNetPacket>);

    /**
     * Fetches the current GeoPosition of the testbed vehicle
     * \return GeoPosition current position
     */
    GeoPosition getCurrentPosition();

    /**
     * Get current speed of testbed vehicle
     * \return vehicle velocity
     */
    vanetza::units::Velocity getCurrentSpeed();

    /**
     * Get current heading of testbed vehicle
     * \return heading
     */
    Angle getCurrentHeading();

protected:
    void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, omnetpp::cObject*, omnetpp::cObject*);

private:
    OtaInterface* mOtaModule;
    omnetpp::cGate* mRadioDriverIn;
    omnetpp::cGate* mRadioDriverOut;
    traci::VehicleController* mVehicleController;
    VehicleKinematics dynamicsDut;
};

} // namespace artery

#endif /* ARTERY_OTAINTERFACELAYER_H */
