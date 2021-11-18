package artery.plugins;

import artery.inet.World;
import artery.plugins.SimSocket;

network DutWorld extends World
{
    submodules:
        otaInterface: <default("OtaInterfaceStub")> like OtaInterface {
            parameters:
                @display("p=70,40");
        }
        SimSocket: SimSocket{}
}
