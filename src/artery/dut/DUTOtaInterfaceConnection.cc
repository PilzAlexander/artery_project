#include "artery/dut/DUTOtaInterfaceConnection.h"
#include "artery/dut/DUTOtaIndicationQueue.h"
#include "artery/dut/DUTOtaInterfaceLayer.h"
#include "artery/dut/DutScheduler.h"
#include "SimSocket.h"
#include "artery/networking/GeoNetRequest.h"
#include <vanetza/access/data_request.hpp>
#include <vanetza/common/byte_order.hpp>
#include <vanetza/geonet/packet.hpp>

#include <zmq.hpp>

namespace artery {

    Define_Module(DUTOtaInterfaceConnection)

    void DUTOtaInterfaceConnection::initialize() {

        mOtaIndicationQueue.reset(new DUTOtaIndicationQueue(this));

        auto scheduler = dynamic_cast<DutScheduler *>(omnetpp::getSimulation()->getScheduler());
        if (scheduler) {
            //scheduler->setOtaIndicationQueue(std::make_shared<DUTOtaIndicationQueue>(this));
            scheduler->setOtaIndicationQueue(mOtaIndicationQueue);
        } else {
            EV_INFO << "No DUTOtaIndicationQueue passed to scheduler";
        }
    }

    void DUTOtaInterfaceConnection::registerModule(DUTOtaInterfaceLayer *layer) {
        mRegisteredModule = layer;
    }

    void DUTOtaInterfaceConnection::unregisterModule() {
        mRegisteredModule = nullptr;
    }

    void DUTOtaInterfaceConnection::sendMessage(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                                                const vanetza::byte_view_range &byteViewRange) {
        // create module pointer to SimSocket with ID = 6
        cModule *mod = getSimulation()->getModule(6);
        auto *mTarget = check_and_cast<artery::SimSocket *>(mod);
        //mTarget->publishSimMsg(macSource, macDest, byteViewRange);
    }

    void DUTOtaInterfaceConnection::receiveMessage(std::unique_ptr<GeoNetPacket> DUTGeoNetPacket) {
        if (hasRegisteredModule()) {
            Enter_Method("receiveMessage");
            mRegisteredModule->request(std::move(DUTGeoNetPacket));
        }
    }

    void DUTOtaInterfaceConnection::placeGeoNetPacketInQueue(const std::array<unsigned char, 6> macSource,
                                                             const std::array<unsigned char, 6> macDest,
                                                             std::vector<unsigned char> buffer) {

        vanetza::access::DataRequest req;
        req.destination_addr.octets = macDest;
        req.source_addr.octets = macSource;
        req.ether_type = vanetza::uint16be_t(0x8947); // GeoNet EtherType

        std::unique_ptr<vanetza::geonet::UpPacket> payload{
                new vanetza::geonet::UpPacket{
                        vanetza::CohesivePacket(std::move(buffer), vanetza::OsiLayer::Network)
                }
        };

        std::unique_ptr<GeoNetPacket> gn(new GeoNetPacket("GeoNet packet from DUT"));
        gn->setPayload(std::move(payload));
        gn->setControlInfo(new GeoNetRequest(req));
        DUTOtaInterfaceConnection::receiveMessage(std::move(gn));
    }

} // namespace artery