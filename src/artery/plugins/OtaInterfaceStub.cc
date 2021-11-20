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

void OtaInterfaceStub::sendMessage(const vanetza::MacAddress&, const vanetza::MacAddress&, const vanetza::byte_view_range&)
{
    /*
    try {
        //std::cout << "Message: " << msgToSend << endl;
        publisherSocket_.send(vanetza::byte_view_range, zmq::send_flags::none);

    } catch (zmq::error_t cantSend) {
        cerr << "Socket can't send: " << cantSend.what() << endl;
        unbind("tcp://*:7777");
    }
     */
}

void OtaInterfaceStub::receiveMessage(std::unique_ptr<GeoNetPacket>)
{
}

} // namespace artery
