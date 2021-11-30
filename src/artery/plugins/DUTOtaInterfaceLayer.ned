package artery.plugins;

simple DUTOtaInterfaceLayer
{
	parameters:
		@class(DUTOtaInterfaceLayer);
		string otaInterfaceModule = default("otaInterface");
		string radioDriverModule;
		string mobilityModule = default(".^.mobility");

	gates:
		input lowerLayerIn;
		output lowerLayerOut;
		input radioDriverProperties @loose;
}
