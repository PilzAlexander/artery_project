#include "artery/plugins/OtaInterfaceStub.h"
#include "artery/plugins/OtaIndicationQueue.h"
#include "artery/plugins/DutScheduler.h"
#include "SimSocket.h"

#include <zmq.hpp>

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

void OtaInterfaceStub::sendMessage(const vanetza::MacAddress& MacSource, const vanetza::MacAddress& MacDest, const vanetza::byte_view_range& byteViewRange)
{
    // create module pointer to SimSocket with ID = 6
    cModule *mod = getSimulation()->getModule(6);
    auto *m_target = check_and_cast<artery::SimSocket *>(mod);
    m_target->publish();

    SimSocket::getOtaInterfaceStub(const_cast<vanetza::MacAddress &>(MacSource),
                                   const_cast<vanetza::MacAddress &>(MacDest),
                                   const_cast<vanetza::byte_view_range &>(byteViewRange));

}

void OtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket>)
{
}

} // namespace artery
