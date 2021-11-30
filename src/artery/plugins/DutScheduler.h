/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_TESTBEDSCHEDULER_H_NJ0QMNVB
#define ARTERY_TESTBEDSCHEDULER_H_NJ0QMNVB

#include <omnetpp/cscheduler.h>
#include <omnetpp/simtime.h>
#include <omnetpp/ccomponent.h>
#include <chrono>
#include <memory>
#include "traci/BasicNodeManager.h"
#include <string>

namespace artery
{

class DUTOtaIndicationQueue;

class DutScheduler : public omnetpp::cScheduler
{
public:
    void startRun() override;
    void executionResumed() override;
    omnetpp::cEvent* takeNextEvent() override;
    omnetpp::cEvent* guessNextEvent() override;
    void putBackEvent(omnetpp::cEvent*) override;

    virtual void setOtaIndicationQueue(std::shared_ptr<DUTOtaIndicationQueue>);

protected:
    virtual void doTiming(omnetpp::cEvent*);
    omnetpp::cEvent* peekFirstNonStaleEvent();

private:
    std::shared_ptr<DUTOtaIndicationQueue> mOtaIndicationQueue = nullptr;
    std::chrono::system_clock::time_point mBaseTime;
    std::chrono::system_clock::duration mThresholdTooSlow;
    omnetpp::simtime_t mStartupTime;
};

} // namespace artery

#endif /* ARTERY_TESTBEDSCHEDULER_H_NJ0QMNVB */
