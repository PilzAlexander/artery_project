package artery.dut;

import inet.linklayer.ieee80211.mac.contention.Contention;

simple DutContention extends Contention
{
	parameters:
		@class(DutContention);
		// IFS defaults to AC_VO AIFS (highest priority)
		double interFrameSpacing = default(0.000058) @unit(s);
}
