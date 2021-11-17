package traci;

simple DutNodeManager extends BasicNodeManager like NodeManager
{
    parameters:
        @class(traci::DutNodeManager);
        string twinId;
        string twinName = default("twin");
}
