/*
 * Artery V2X Simulation Framework
 * Copyright 2017-2018 Christina Obermaier
 * Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
 */

#ifndef ARTERY_DUTCONTENTION_H
#define ARTERY_DUTCONTENTION_H
/********************************************************************************
 * Class declaration
 ********************************************************************************/
#include <inet/linklayer/ieee80211/mac/contention/Contention.h>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
namespace artery {

    class DutContention : public inet::ieee80211::Contention {
    public:
        void initialize(int stage) override;

        /**
         * Tries to minimize the contention delay as much as possible
         */
        void startContention(int cw, simtime_t ifs, simtime_t eifs, simtime_t slotTime,
                             inet::ieee80211::Contention::ICallback *callback) override;

    private:
        omnetpp::simtime_t mIfs;
    };

} // namespace artery

#endif /* ARTERY_DUTCONTENTION_H */