/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimEventFromInterfaceVisitor.h
  \brief    Class to convert a value, that is allowed to have different types, into a string value
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     17.11.2021
 ********************************************************************************/
#ifndef ARTERY_SIMEVENTFROMINTERFACEVISITOR_H
#define ARTERY_SIMEVENTFROMINTERFACEVISITOR_H
/********************************************************************************
 * Includes
 *********************************************************************************/
#include <string>
#include <map>
#include <sstream>
#include "boost/variant.hpp"
/********************************************************************************
 * Class declaration
 ********************************************************************************/
namespace artery {
    class SimEventFromInterfaceVisitor : public boost::static_visitor<std::string> {

    public:
        std::string operator()(int i) const {
            return "";
        }

        std::string operator()(double d) const {
            return "";
        }

        std::string operator()(const std::string &str) const {
            return str;
        }
    };
}

#endif //ARTERY_SIMEVENTFROMINTERFACEVISITOR_H