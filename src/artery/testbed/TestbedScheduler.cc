#include "artery/testbed/OtaIndicationQueue.h"
#include "artery/testbed/TestbedScheduler.h"
#include "artery/plugins/SimSocket.h"
#include <omnetpp/cconfigoption.h>
#include <omnetpp/cconfiguration.h>
#include <omnetpp/cfutureeventset.h>
#include <omnetpp/regmacros.h>
#include <omnetpp/ccomponent.h>
#include <fstream>
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include "omnetpp.h"
#include <thread>
namespace artery
{

Register_Class(TestbedScheduler)
Register_GlobalConfigOption(CFG_SIMULATION_TOO_SLOW, "simulation-too-slow", CFG_INT , "100",
        "Threshold for a simulation to lag behind wall-clock (milliseconds).")
Register_GlobalConfigOption(CFG_STARTUP_TIME, "simulation-startup-time", CFG_DOUBLE, "4",
        "Time span at the beginning of a simulation in which simulation-too-slow is not checked (seconds).")


void TestbedScheduler::startRun()
{
    omnetpp::cConfiguration *config = omnetpp::getEnvir()->getConfig();
    mThresholdTooSlow = std::chrono::milliseconds(config->getAsInt(CFG_SIMULATION_TOO_SLOW));
    mStartupTime = config->getAsDouble(CFG_STARTUP_TIME);
    mBaseTime = std::chrono::system_clock::now();
    //Hier socket erstellen
  // SimSocket::createSocket("5555", "Tesst");

}

void TestbedScheduler::executionResumed()
{
    using namespace std::chrono;
    mBaseTime = system_clock::now() - microseconds(omnetpp::simTime().inUnit(omnetpp::SIMTIME_US));

}

void TestbedScheduler::putBackEvent(omnetpp::cEvent* event)
{
    sim->getFES()->putBackFirst(event);

}

omnetpp::cEvent* TestbedScheduler::guessNextEvent()
{

    try {
        return peekFirstNonStaleEvent();
    } catch (const omnetpp::cTerminationException&) {
        return nullptr;
    }
}
    using namespace omnetpp;
omnetpp::cEvent* TestbedScheduler::takeNextEvent()
{
    using namespace omnetpp;
    cEvent* init = peekFirstNonStaleEvent();
    cEvent* after = init;
    //std::cout << "takeNextEvent \n";
    do {
        init = after;
        doTiming(init);
        after = peekFirstNonStaleEvent();
    } while (init != after);

    cEvent* next = sim->getFES()->removeFirst();
    ASSERT(!next->isStale());

   // std::cout << "###############################################";
    //hier ssende
    SimSocket::sendMessage("JA GEGE");
    return next;
}

void TestbedScheduler::doTiming(omnetpp::cEvent* event)
{
    using namespace std::chrono;
    auto arrival = mBaseTime + microseconds(event->getArrivalTime().inUnit(omnetpp::SIMTIME_US));
    auto waitFor = arrival - system_clock::now();

    if (mOtaIndicationQueue && waitFor >= system_clock::duration::zero()) {
        mOtaIndicationQueue->flushQueue();
        waitFor = arrival - system_clock::now(); /*< update because of flushQueue's execution duration */
        mOtaIndicationQueue->waitFor(duration_cast<microseconds>(waitFor));
    } else if ((omnetpp::simTime() > mStartupTime) && (-waitFor > mThresholdTooSlow)) {
        int tooSlow_ms = duration_cast<milliseconds>(mThresholdTooSlow).count();
        int waitFor_ms = duration_cast<milliseconds>(waitFor).count();
        omnetpp::cRuntimeError("Simulation too far behind real-time. "
                "Maximum is set to %d milliseconds, but simulation lags behind %d milliseconds",
                tooSlow_ms, waitFor_ms);
    }
}

omnetpp::cEvent* TestbedScheduler::peekFirstNonStaleEvent()
{
    omnetpp::cEvent* event = nullptr;

    do {
        event = sim->getFES()->peekFirst();
        if (!event) {
            throw omnetpp::cTerminationException("No more events");
        } else if (event->isStale()) {
            event = nullptr;
            omnetpp::cEvent* tmp = sim->getFES()->removeFirst();
            delete tmp;
        }
    } while (!event);

    return event;
}

void TestbedScheduler::setOtaIndicationQueue(std::shared_ptr<OtaIndicationQueue> queue)
{

    mOtaIndicationQueue = queue;
}

} // namespace artery
