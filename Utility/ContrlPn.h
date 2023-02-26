struct _DXData
{
        float WattsIn, WattsOut, LevelPercentage, EfficiencyRatio, Amperage, TT, TR,
		BiggestAmperage, kEff;
	int  Status, IsConnected, BatteryMode;
};

struct _DXReq
{
        int Connect, On, Close;
};
