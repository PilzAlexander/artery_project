//
// Created by vagrant on 12/14/21.
//

#ifndef ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H
#define ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H

#include <string>
#include <map>
#include <sstream>
#include "boost/variant.hpp"

/**
 * Class to convert a value, that is allowed to have different types, into a string value.
 */

namespace artery {
    class SimEventFromInterfaceVisitorInt : public boost::static_visitor<int> {

    public:
        int operator() (int i) const {
            return i;
        }

        int operator() (double d) const {
            return 10;
        }

        int operator() (const std::string &str) const {
            return 20;
        }

    };

}

#endif //ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H
