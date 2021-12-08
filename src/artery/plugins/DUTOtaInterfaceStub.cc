#include "artery/plugins/DUTOtaInterfaceStub.h"
#include "artery/plugins/DUTOtaIndicationQueue.h"
#include "artery/plugins/DutScheduler.h"
#include "SimSocket.h"

#include <zmq.hpp>

namespace artery
{

Define_Module(DUTOtaInterfaceStub)

void DUTOtaInterfaceStub::initialize()
{
    auto scheduler = dynamic_cast<DutScheduler*>(omnetpp::getSimulation()->getScheduler());
    if (scheduler) {
        scheduler->setOtaIndicationQueue(std::make_shared<DUTOtaIndicationQueue>(this));
    } else {
        EV_INFO << "No DUTOtaIndicationQueue passed to scheduler";
    }
}

void DUTOtaInterfaceStub::registerModule(DUTOtaInterfaceLayer* layer)
{
    mRegisteredModule = layer;
}

void DUTOtaInterfaceStub::unregisterModule()
{
    mRegisteredModule = nullptr;
}

void DUTOtaInterfaceStub::sendMessage(const vanetza::MacAddress& macSource, const vanetza::MacAddress& macDest, const vanetza::byte_view_range& byteViewRange)
{
    // create module pointer to SimSocket with ID = 6
    cModule *mod = getSimulation()->getModule(6);
    auto *mTarget = check_and_cast<artery::SimSocket *>(mod);
    mTarget->publishSimMsg(macSource, macDest, byteViewRange);
}

void DUTOtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket> DUTGeoNetPacket)
{
    //DUTGeoNetPacket->getPayload();
    //
}

} // namespace artery
