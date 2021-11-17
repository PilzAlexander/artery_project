/********************************************************************************
  \project  INFM_HIL_Interface
  \file     DutNodeManager.h
  \brief    Node manager for the dut simulation
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     17.11.2021
 ********************************************************************************/

/********************************************************************************
 * Includes
 *********************************************************************************/

#ifndef ARTERY_DUTNODEMANAGER_H
#define ARTERY_DUTNODEMANAGER_H

/********************************************************************************
 * Class declarations
 ********************************************************************************/

#include "traci/BasicNodeManager.h"
#include <string>

namespace traci
{

    class DutNodeManager : public BasicNodeManager
    {
    public:
        void initialize() override;

    protected:
        virtual omnetpp::cModule* createModule(const std::string&, omnetpp::cModuleType*) override;

    private:
        std::string m_twinId;
        std::string m_twinName;
    };

} /* namespace traci */


#endif //ARTERY_DUTNODEMANAGER_H

/********************************************************************************
 * EOF
 ********************************************************************************/