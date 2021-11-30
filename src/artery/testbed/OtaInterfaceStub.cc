#include "artery/testbed/OtaInterfaceStub.h"
#include "artery/testbed/OtaIndicationQueue.h"
#include "artery/testbed/TestbedScheduler.h"

namespace artery
{

Define_Module(DUTOtaInterfaceStub)

void DUTOtaInterfaceStub::initialize()
{
    auto scheduler = dynamic_cast<TestbedScheduler*>(omnetpp::getSimulation()->getScheduler());
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

void DUTOtaInterfaceStub::sendMessage(const vanetza::MacAddress&, const vanetza::MacAddress&, const vanetza::byte_view_range&)
{
    //nothing to do here, as no hardware is connected
}

void DUTOtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket>)
{
}

} // namespace artery
