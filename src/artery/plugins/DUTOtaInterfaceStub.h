/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_OTA_INTERFACE_STUB_H
#define ARTERY_OTA_INTERFACE_STUB_H

#include "artery/plugins/DUTOtaInterface.h"
#include <omnetpp/csimplemodule.h>
#include "SimSocket.h"

namespace artery
{
/**
 * The DUTOtaInterfaceStub provides a implementation of the DUTOtaInterface which acts as an stub to work without any hardware connected.
 * It implements all necessary methods, mos of them doing nothing.
 * It can be used to compile the testbed without any external library available (like the S.E.A. API required by the OtaInterfaceUsrp).
 */
class DUTOtaInterfaceStub : public DUTOtaInterface, public omnetpp::cSimpleModule
{
public:
    void initialize() override;
    void registerModule(DUTOtaInterfaceLayer*) override;
    void unregisterModule() override;
    void sendMessage(const vanetza::MacAddress&, const vanetza::MacAddress&, const vanetza::byte_view_range&) override;
    void receiveMessage(std::unique_ptr<GeoNetPacket>) override;
    bool hasRegisteredModule() override { return mRegisteredModule != nullptr; }
private:
    DUTOtaInterfaceLayer* mRegisteredModule = nullptr;
};

} // namespace artery

#endif /* ARTERY_OTA_INTERFACE_STUB_H */
