#include "artery/dut/DutContention.h"

namespace artery
{

Define_Module(DutContention)

void DutContention::initialize(int stage)
{
    inet::ieee80211::Contention::initialize(stage);
    if (stage == 0) {
        mIfs = par("interframeSpacing");
    }
}

void DutContention::startContention(int cw, simtime_t ifs, simtime_t eifs, simtime_t slotTime, inet::ieee80211::Contention::ICallback *callback)
{
    inet::ieee80211::Contention::startContention(0, mIfs, eifs, slotTime, callback);
}

} // namespace artery
