#include "artery/plugins/OtaInterfaceStub.h"
#include "artery/plugins/OtaIndicationQueue.h"
#include "artery/plugins/DutScheduler.h"
#include "SimSocket.h"

namespace artery
{

Define_Module(OtaInterfaceStub)

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

void OtaInterfaceStub::sendMessage(const vanetza::MacAddress&, const vanetza::MacAddress&, const vanetza::byte_view_range&)
{
    //publishSimMsg();
}

void OtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket>)
{
}

} // namespace artery
