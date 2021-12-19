/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Contributors Alexander Pilz, Johannes Winter
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_OTA_INTERFACE_STUB_H
#define ARTERY_OTA_INTERFACE_STUB_H
/********************************************************************************
 * Includes
 *********************************************************************************/
#include "artery/dut/DUTOtaInterface.h"
#include "artery/dut/DUTOtaIndicationQueue.h"
#include <omnetpp/csimplemodule.h>
#include "SimSocket.h"
/********************************************************************************
 * Class declaration
 ********************************************************************************/
namespace artery {
/**
 * The DUTOtaInterfaceConnection provides a implementation of the DUTOtaInterface which can work with equipped hardware.
 * It implements all necessary methods.
 * It can be used to compile the dut without any external library available (like the S.E.A. API required by the OtaInterfaceUsrp).
 */
    class DUTOtaInterfaceConnection : public DUTOtaInterface, public omnetpp::cSimpleModule {
    public:
        /**
         * Register Physical Twin module at the Ota OtaInterface
         * Throws cRuntimeError if a second modules tries to register
         * Must be called from the OtaInterfaceLayer when joining simulation (eg. in the intialize() method)
         */
        void registerModule(DUTOtaInterfaceLayer *) override;

        /**
         * Unregisters the registeredModule and closes the the GPSD socket if it was opened
         * Must be called from the OtaInterfaceLayer when leaving the simulation (eg. in the finish() method)
         */
        void unregisterModule() override;

        /**
         * Sends message from Physical Twin module to the USRP Device
         * Throws cRuntimeError if message is from a not registered module
         *
         * \param source MAC address of the sending node
         * \param destination MAC address of the receiving module, usually the MAC address of the device under test
         * \param data Byte range which should be transmitted over the air
         */
        void sendMessage(const vanetza::MacAddress &, const vanetza::MacAddress &,
                         const vanetza::byte_view_range &) override;

        /**
         * Receives a GeonetPacket which was scheduled by the ThreadSafeScheduler
         *
         * \param GeonetPacket which should be sent to the simulation
         */
        void receiveMessage(std::unique_ptr<GeoNetPacket>) override;

        /**
         * Tests if the physical twin is registered at the OtaInterface
         *
         * \return true if physical twin is registered
         */
        bool hasRegisteredModule() override { return mRegisteredModule != nullptr; }

        /**
         * puts  receives bytes into geoNetPacket
         *
         * @param buffer receives bytes as std::vector<unsigned char>
         */
        void placeGeoNetPacketInQueue(const std::array<unsigned char, 6> macSource,
                                      const std::array<unsigned char, 6> macDest,
                                      std::vector<unsigned char> buffer);

    protected:
        void initialize() override;

    private:
        DUTOtaInterfaceLayer *mRegisteredModule = nullptr;
        std::shared_ptr<DUTOtaIndicationQueue> mOtaIndicationQueue;
    };
} // namespace artery

#endif /* ARTERY_OTA_INTERFACE_STUB_H */
