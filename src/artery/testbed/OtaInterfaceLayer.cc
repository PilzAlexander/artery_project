#include "artery/networking/GeoNetIndication.h"
#include "artery/networking/GeoNetPacket.h"
#include "artery/nic/RadioDriverBase.h"
#include "artery/testbed/OtaInterfaceLayer.h"
#include "artery/testbed/OtaInterface.h"
#include "artery/testbed/TestbedRadio.h"
#include "artery/traci/ControllableVehicle.h"
#include "artery/traci/VehicleController.h"
#include <inet/common/ModuleAccess.h>
#include <vanetza/net/packet_variant.hpp>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

namespace artery
{

Define_Module(DUTOtaInterfaceLayer)

void DUTOtaInterfaceLayer::initialize(int stage)
{
    if (stage == 1) {
        std::string otaInterfaceModule = par("otaInterfaceModule");
        mOtaModule = dynamic_cast<DUTOtaInterface*>(getModuleByPath(otaInterfaceModule.c_str()));
        if (!mOtaModule) {
            throw omnetpp::cRuntimeError(this, "Specified OTA module %s not found! "
                    "Check if withTestbed was set to true", otaInterfaceModule.c_str());
        }
        mOtaModule->registerModule(this);
        mRadioDriverIn = gate("lowerLayerIn");
        mRadioDriverOut = gate("lowerLayerOut");

        auto mobility = inet::getModuleFromPar<ControllableVehicle>(par("mobilityModule"), this);
        mVehicleController = mobility->getVehicleController();
        ASSERT(mVehicleController);
    }
}

void DUTOtaInterfaceLayer::finish()
{
    mOtaModule->unregisterModule();
}

void DUTOtaInterfaceLayer::handleMessage(omnetpp::cMessage* message)
{
    if (message->getArrivalGate() == mRadioDriverIn) {
        auto packet = check_and_cast<GeoNetPacket*>(message);
        auto info = check_and_cast<GeoNetIndication*>(message->removeControlInfo());
        if (info) {
            using namespace vanetza;
            auto range = create_byte_view(packet->getPayload(), OsiLayer::Network, OsiLayer::Application);
            mOtaModule->sendMessage(info->source, info->destination, range);
        }
    }

    delete message;
}

void DUTOtaInterfaceLayer::request(std::unique_ptr<GeoNetPacket> packet)
{
    Enter_Method("request");
    GeoNetPacket* ptr = packet.release();
    take(ptr);
    send(ptr, mRadioDriverOut);
}

GeoPosition DUTOtaInterfaceLayer::getCurrentPosition()
{
    return mVehicleController->getGeoPosition();
}

vanetza::units::Velocity DUTOtaInterfaceLayer::getCurrentSpeed()
{
    return mVehicleController->getSpeed();
}

Angle DUTOtaInterfaceLayer::getCurrentHeading()
{
    return mVehicleController->getHeading();
}

} // namespace artery
