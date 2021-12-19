#include "artery/dut/DUTOtaIndicationQueue.h"
#include "artery/dut/DUTOtaInterface.h"

namespace artery
{

DUTOtaIndicationQueue::DUTOtaIndicationQueue(DUTOtaInterface* interface) :
    mOtaInterface(interface), mNotified(false)
{
}

void DUTOtaIndicationQueue::waitFor(std::chrono::microseconds waitFor)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mCondVar.wait_for(lock, waitFor, [this]{ return mNotified; })) {
        // create sending event only if the physical twin has already been created
        if (mOtaInterface->hasRegisteredModule()) {
            for (auto& ind : mIndicationList) {
                mOtaInterface->receiveMessage(std::move(ind));
            }
        }
        mIndicationList.clear();
        mNotified = false;
    }
}

void DUTOtaIndicationQueue::trigger(std::unique_ptr<GeoNetPacket> ind)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mIndicationList.emplace_back(std::move(ind));
    mNotified = true;
    mCondVar.notify_one();
}

void DUTOtaIndicationQueue::flushQueue()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mOtaInterface->hasRegisteredModule()) {
        for (auto& ind : mIndicationList) {
            mOtaInterface->receiveMessage(std::move(ind));
        }
    }
    mIndicationList.clear();
    mNotified = false;
}

} // namespace artery
