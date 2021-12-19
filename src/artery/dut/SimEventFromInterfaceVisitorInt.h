/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimEventFromInterfaceVisitorInt.h
  \brief    Class to convert a value, that is allowed to have different types, into a int value
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     17.11.2021
 ********************************************************************************/
#ifndef ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H
#define ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H
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
    class SimEventFromInterfaceVisitorInt : public boost::static_visitor<int> {

    public:
        int operator()(int i) const {
            return i;
        }

        int operator()(double d) const {
            return 0;
        }

        int operator()(const std::string &str) const {
            return 0;
        }
    };
}

#endif //ARTERY_SIMEVENTFROMINTERFACEVISITORINT_H