#ifndef SRC_TRACI_TESTBEDNODEMANAGER_H_
#define SRC_TRACI_TESTBEDNODEMANAGER_H_

#include "traci/BasicNodeManager.h"
#include "artery/plugins/InterfaceConnection.h"
#include <string>

namespace traci
{

class TestbedNodeManager : public BasicNodeManager
{
public:
    void initialize() override;
    static std::string m_twinId;
    static std::string m_twinName;

protected:
    virtual omnetpp::cModule* createModule(const std::string&, omnetpp::cModuleType*) override;

private:
    //std::string m_twinId;
    //std::string m_twinName;
};

} /* namespace traci */

#endif /* SRC_TRACI_TESTBEDNODEMANAGER_H_ */
