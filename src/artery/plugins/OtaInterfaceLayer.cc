#include "artery/networking/GeoNetIndication.h"
#include "artery/networking/GeoNetPacket.h"
#include "artery/nic/RadioDriverBase.h"
#include "artery/plugins/OtaInterfaceLayer.h"
#include "artery/plugins/OtaInterface.h"
#include "artery/traci/ControllableVehicle.h"
#include <inet/common/ModuleAccess.h>
#include <vanetza/net/packet_variant.hpp>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>
#include <artery/traci/MobilityBase.h>
#include "artery/application/VehicleKinematics.h"
#include "SimSocket.h"

namespace artery
{

Define_Module(OtaInterfaceLayer)

void OtaInterfaceLayer::initialize(int stage)
{
    if(stage == 0){
        auto& mobilityPar = par("mobilityModule");
        auto* mobilityModule = getModuleByPath(mobilityPar);
        if (mobilityModule) {
            std::cout << "##################################" << std::endl;
            mobilityModule->subscribe(MobilityBase::stateChangedSignal, this);
        } else {
            error("Module on path '%s' is not a VehicleMobility", mobilityModule->getFullPath().c_str());
        }
    }

    if (stage == 1) {
        std::string otaInterfaceModule = par("otaInterfaceModule");
        mOtaModule = dynamic_cast<OtaInterface*>(getModuleByPath(otaInterfaceModule.c_str()));
        if (!mOtaModule) {
            throw omnetpp::cRuntimeError(this, "Specified OTA module %s not found! "
                    "Check if withPlugins was set to true", otaInterfaceModule.c_str());
        }
        mOtaModule->registerModule(this);
        mRadioDriverIn = gate("lowerLayerIn");
        mRadioDriverOut = gate("lowerLayerOut");

        auto mobility = inet::getModuleFromPar<ControllableVehicle>(par("mobilityModule"), this);
        mVehicleController = mobility->getVehicleController();
        ASSERT(mVehicleController);
    }
}

void OtaInterfaceLayer::finish()
{
    mOtaModule->unregisterModule();
}

void OtaInterfaceLayer::handleMessage(omnetpp::cMessage* message)
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

void OtaInterfaceLayer::request(std::unique_ptr<GeoNetPacket> packet)
{
    Enter_Method("request");
    GeoNetPacket* ptr = packet.release();
    take(ptr);
    send(ptr, mRadioDriverOut);
}

void OtaInterfaceLayer::receiveSignal(cComponent* component, simsignal_t signal, cObject* obj, cObject* details)
{
    if (signal == MobilityBase::stateChangedSignal && mVehicleController) {
        dynamicsDut = getKinematics(*mVehicleController);
        SimSocket::getVehicleDynamics(dynamicsDut);
    }
}

GeoPosition OtaInterfaceLayer::getCurrentPosition()
{
    return mVehicleController->getGeoPosition();
}

vanetza::units::Velocity OtaInterfaceLayer::getCurrentSpeed()
{
    return mVehicleController->getSpeed();
}

Angle OtaInterfaceLayer::getCurrentHeading()
{
    return mVehicleController->getHeading();
}

} // namespace artery
