/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Contributors Alexander Pilz, Johannes Winter
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_DUTOTAINTERFACELAYER_H
#define ARTERY_DUTOTAINTERFACELAYER_H

#include "artery/networking/GeoNetPacket.h"
#include "artery/utility/Geometry.h"
#include <omnetpp/csimplemodule.h>
#include <vanetza/units/velocity.hpp>
#include "artery/application/VehicleDataProvider.h"

namespace traci { class VehicleController; }

namespace artery
{

class DUTOtaInterface;

class DUTOtaInterfaceLayer : public omnetpp::cSimpleModule, public omnetpp::cListener
{
public:
    /**
     * Initializes the module and registers at the DUTOtaInterface
     */
    void initialize(int) override;
    int numInitStages() const override { return 2; }
    void finish() override;

    /**
     * Receives messages from lower layers and handles it to the DUTOtaInterface
     * \param msg Message to be transmitted to DUTOtaInterface
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
    DUTOtaInterface* mOtaModule;
    omnetpp::cGate* mRadioDriverIn = nullptr;
    omnetpp::cGate* mRadioDriverOut = nullptr;
    traci::VehicleController* mVehicleController = nullptr;
    VehicleKinematics dynamicsDut;
};

} // namespace artery

#endif /* ARTERY_DUTOTAINTERFACELAYER_H */
