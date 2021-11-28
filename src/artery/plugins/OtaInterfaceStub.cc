#include "artery/plugins/OtaInterfaceStub.h"
#include "artery/plugins/OtaIndicationQueue.h"
#include "OtaInterfaceLayer.h"
#include "artery/plugins/DutScheduler.h"
#include "SimSocket.h"

namespace artery
{

Define_Module(OtaInterfaceStub)

//const simsignal_t OtaInterfaceStub::ReceiveMessage = cComponent::registerSignal("traci.ota.msg");

void OtaInterfaceStub::initialize()
{
    auto scheduler = dynamic_cast<DutScheduler*>(omnetpp::getSimulation()->getScheduler());
    if (scheduler) {
        scheduler->setOtaIndicationQueue(std::make_shared<OtaIndicationQueue>(this));
    } else {
        EV_INFO << "No OtaIndicationQueue passed to scheduler";
    }
}

void OtaInterfaceStub::registerModule(OtaInterfaceLayer* layer)
{
    mRegisteredModule = layer;
}

void OtaInterfaceStub::unregisterModule()
{
    mRegisteredModule = nullptr;
}

void OtaInterfaceStub::sendMessage(const vanetza::MacAddress& MacSource, const vanetza::MacAddress& MacDest, const vanetza::byte_view_range& byteViewRange)
{
    SimSocket::getOtaInterfaceStub(const_cast<vanetza::MacAddress &>(MacSource),
                                   const_cast<vanetza::MacAddress &>(MacDest),
                                   const_cast<vanetza::byte_view_range &>(byteViewRange));
}

void OtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket>)
{
}

} // namespace artery
