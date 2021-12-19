package artery.dut;

import artery.inet.World;
import artery.dut.SimSocket;

network DutWorld extends World
{
    submodules:
        otaInterface: <default("DUTOtaInterfaceConnection")> like DUTOtaInterface {
            parameters:
                @display("p=70,40");
        }
        SimSocket: SimSocket{}
}
