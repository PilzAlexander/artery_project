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

#include "artery/plugins/SimSocket.h"
#include "traci/BasicNodeManager.h"
#include <string>

#include "artery/application/VehicleDataProvider.h"

namespace traci
{
    class API;
    class ModuleMapper;
    class PersonSink;
    class VehicleCache;
    class VehicleSink;
    class Vehicle;

class DutNodeManager : public BasicNodeManager
    {
    public:
        void initialize() override;

    protected:
        virtual omnetpp::cModule* createModule(const std::string&, omnetpp::cModuleType*) override;
        virtual void updateVehicle(const std::string&, VehicleSink*) override;
        /**
         * Set received vehicle data to the DUT (works only with signals at the moment, as other values
         * are overwritten by the simulation)
         * @param id
         * @param inputDataMap
         */
        void setVehicleData(const std::string &id, artery::SimSocket::DataMap inputDataMap);

    private:
        std::string m_twinId;
        std::string m_twinName;

        const traci::VehicleController* mVehicleController = nullptr;

};

} /* namespace traci */


#endif //ARTERY_DUTNODEMANAGER_H

/********************************************************************************
 * EOF
 ********************************************************************************/