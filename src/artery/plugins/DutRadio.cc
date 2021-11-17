#include "artery/plugins/DutRadio.h"
#include <inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h>
#include <inet/physicallayer/common/packetlevel/RadioMedium.h>
#include <inet/physicallayer/common/packetlevel/Radio.h>

namespace artery
{

Define_Module(DutRadio)

void DutRadio::initialize(int stage)
{
    Ieee80211Radio::initialize(stage);
    mReachableTime = par("reachableTime");
}

void DutRadio::endReception(omnetpp::cMessage *timer)
{
    auto radioFrame = static_cast<inet::physicallayer::RadioFrame*>(timer->getControlInfo());
    auto transmission = radioFrame->getTransmission();
    auto macFrame = dynamic_cast<const inet::ieee80211::Ieee80211DataOrMgmtFrame*>(transmission->getMacFrame());
    auto part = static_cast<inet::physicallayer::IRadioSignal::SignalPart>(timer->getKind());
    auto receptionDecision = getMedium()->getReceptionDecision(this, radioFrame->getListening(), transmission, part);
    if (receptionDecision->isReceptionSuccessful() && macFrame) {
        mReachableNodes[macFrame->getTransmitterAddress()] = simTime();
    }
    Radio::endReception(timer);
}

void DutRadio::updateReachableNodes()
{
    for (auto reachableNode = mReachableNodes.begin(); reachableNode != mReachableNodes.end();) {
        if ((simTime() - reachableNode->second) > mReachableTime) {
            reachableNode = mReachableNodes.erase(reachableNode);
        } else {
            ++reachableNode;
        }
    }
}

const std::map<inet::MACAddress, omnetpp::simtime_t>& DutRadio::getReachableNodes()
{
    updateReachableNodes();
    return mReachableNodes;
}

} // namespace artery
