package traci;

simple DutNodeManager extends BasicNodeManager like NodeManager
{
    parameters:
        @class(traci::DutNodeManager);
        @signal[traci.send.update](type=unsigned long);

        string twinId;
        string twinName = default("twin");


}
