//
// Created by vagrant on 11/17/21.
//

#ifndef ARTERY_DUTMODULEMAPPER_H
#define ARTERY_DUTMODULEMAPPER_H

#include "traci/BasicModuleMapper.h"

namespace traci
{

    class DutModuleMapper : public BasicModuleMapper
    {
    public:
        void initialize() override;
        omnetpp::cModuleType* vehicle(NodeManager& manager, const std::string& id) override;

    private:
        std::string m_twinId;
        omnetpp::cModuleType* m_twinType;
    };

} // namespace traci

#endif //ARTERY_DUTMODULEMAPPER_H
