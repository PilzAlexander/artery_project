/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_OTA_INDICATION_QUEUE_H
#define ARTERY_OTA_INDICATION_QUEUE_H
/********************************************************************************
 * Includes
 *********************************************************************************/
#include "artery/networking/GeoNetPacket.h"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
namespace artery {

    class DUTOtaInterface;

/**
 * The DUTOtaIndicationQueue can be used to dispatch messages between an DUTOtaInterface and an OMNeT++ scheduler
 */
    class DUTOtaIndicationQueue {
    public:
        DUTOtaIndicationQueue(DUTOtaInterface *interface);

        /**
         * Called by the scheduler to wait for the next event
         */
        virtual void waitFor(std::chrono::microseconds);

        /**
         * Called by the DUTOtaInterface when a new GeoNetPacket must be scheduled by OMNeT++
         * \param GeoNetPacket to send
         */
        virtual void trigger(std::unique_ptr<GeoNetPacket>);

        /**
         * Called by the scheduler if it is faster than real time and all packets in the queue can be sent
         */
        virtual void flushQueue();

    private:
        std::condition_variable mCondVar;
        std::vector<std::unique_ptr<GeoNetPacket>> mIndicationList;
        std::mutex mMutex;
        DUTOtaInterface *mOtaInterface;
        bool mNotified;
    };

} // namespace artery

#endif /* ARTERY_OTA_INDICATION_QUEUE_H */

