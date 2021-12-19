/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_OTA_INTERFACE_H
#define ARTERY_OTA_INTERFACE_H

#include "artery/networking/GeoNetPacket.h"
#include <vanetza/common/byte_view.hpp>
#include <vanetza/net/mac_address.hpp>
#include <memory>

namespace artery
{

class DUTOtaInterfaceLayer;

/**
 * Interface module for all DUTOtaInterface classes.
 * The DUTOtaInterface is a global OMNeT++ Module which maintains the data exchange between a certain DUT and the interface hardware
 */
class DUTOtaInterface
{
public:
    /**
     * Registers a DUTOtaInterfaceLayer at the DUTOtaInterface
     *
     * \param OtaInterfaceLayer to register
     */
    virtual void registerModule(DUTOtaInterfaceLayer*) = 0;

    /**
     * Unregisters an allready registered DUTOtaInterfaceLayer
     */
    virtual void unregisterModule() = 0;

    /**
     * Transmits a simulated packet to the DUT
     *
     * \param cModule which is the sender (usually an DUTOtaInterfaceLayer)
     * \param MacAddress source MAC address (usually the MAC of a simulated vehicle)
     * \param MacAddress destination MAC address
     * \param byte_view_range containing the message data
     */
    virtual void sendMessage(const vanetza::MacAddress& source, const vanetza::MacAddress& destination, const vanetza::byte_view_range& data) = 0;

    /**
     * Receives a GeonetPacket which was scheduled by the used scheduler
     * The packet should be transmitted using a OMNeT++ module (eg. the registered DUTOtaInterfaceLayer)
     *
     * \param GeonetPacket which should be sent to the simulation
     */
    virtual void receiveMessage(std::unique_ptr<GeoNetPacket>) = 0;

    /**
     * Should check if a module was registered at the DUTOtaInterface
     */
    virtual bool hasRegisteredModule() = 0;

    virtual ~DUTOtaInterface() = default;
};

} // namespace artery

#endif /* ARTERY_OTA_INTERFACE_H */
