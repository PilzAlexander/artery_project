//
// Created by vagrant on 12/9/21.
//

#ifndef ARTERY_SIMEVENTFROMINTERFACEVISITOR_H
#define ARTERY_SIMEVENTFROMINTERFACEVISITOR_H

#include <string>
#include <map>
#include <sstream>
#include "boost/variant.hpp"

/**
 * Class to convert a value, that is allowed to have different types, into a string value.
 */

namespace artery {
class SimEventFromInterfaceVisitor : public boost::static_visitor<std::string> {

    public:
        std::string operator() (int i) const {
            return "";
        }

        std::string operator() (double d) const {
            return "";
        }

        std::string operator() (const std::string &str) const {
            return str;
        }

    };

}

#endif //ARTERY_SIMEVENTFROMINTERFACEVISITOR_H
