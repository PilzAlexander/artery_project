#include "traci/DutModuleMapper.h"
#include <omnetpp/ccomponenttype.h>

namespace traci
{

    Define_Module(DutModuleMapper)

    void DutModuleMapper::initialize()
    {
        m_twinId = par("twinId").stringValue();
        m_twinType = omnetpp::cModuleType::get(par("twinType"));
        BasicModuleMapper::initialize();
    }

    omnetpp::cModuleType* DutModuleMapper::vehicle(NodeManager& manager, const std::string& id)
    {
        return m_twinId == id ? m_twinType : BasicModuleMapper::vehicle(manager, id);
    }

} /* namespace traci */
