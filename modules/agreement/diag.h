#ifndef _DIAG_H_
#define _DIAG_H_


#define FuelType 1//汽油车
#define DieselType 2//柴油车

typedef struct{
    char DISD[10];
    uint32_t MFT;
    uint32_t FST;
    uint32_t CC;
    uint32_t CCH;
    uint32_t OS;
    uint32_t OSH;
    uint32_t EECS;
    uint32_t EGR;
    uint32_t SAIS;
    uint32_t ICM;
    uint32_t DPF;
    uint32_t NMHC;
    uint32_t NOX;
    uint32_t BPCS;
    uint32_t EGS;
    uint32_t MIL;
    uint32_t DTCNumber;
}SYSTEM_CHECK_STATE;

typedef struct{
    uint8_t VIN[20];
    char OBDType[30];
  
}ECU_INFO;
typedef struct{
    uint8_t CurrentDTC[100];  //当前故障代码
    uint8_t UnsettledDTC[100];//未解决故障代码
    uint32_t Current;//当前故障代码数量
    uint32_t Unsettled;//未解决故障代码数量
    uint32_t Mileage;
}DTC_INFO;

typedef struct{
    /*柴汽共用*/
    uint32_t OMCEC;
    uint32_t ICC;
    /*汽油车IUPR数据*/
    uint32_t CMCCB1;
    uint32_t CMCECB1;
    uint32_t CMCCB2;
    uint32_t CMCECB2;
    uint32_t AO2SMCCB1;
    uint32_t AO2SMCECB1;
    uint32_t AO2SMCCB2;
    uint32_t AO2SMCECB2;
    uint32_t O2SMCCB1;
    uint32_t O2SMCECB1;
    uint32_t O2SMCCB2;
    uint32_t O2SMCECB2;
    uint32_t EGRC;
    uint32_t EGREC;
    uint32_t AMCCC;
    uint32_t AMCEC;
    uint32_t EVAPC;
    uint32_t EVAPEC;
    uint32_t GPFC1;
    uint32_t GPFEC1;
    uint32_t GPFC2;
    uint32_t GPFEC2;
    /*柴油车IUPR数据*/
    uint32_t NMHCC;
    uint32_t NMHCEC;
    uint32_t NOXCC;
    uint32_t NOXCEC;
    uint32_t NOXAC;
    uint32_t NOXAEC;
    uint32_t PMC;
    uint32_t PMEC;
    uint32_t WSC;
    uint32_t WSEC;
    uint32_t PPC;
    uint32_t PPEC;
}IUPR;

typedef struct{
    uint32_t ODO; //车辆累计行驶里程
    uint32_t Mileage;//MIL 灯点亮后行驶的里程  
    uint32_t ThrottlePosition;//油门开度
    uint32_t EngineRev;//发动机转速
    uint32_t EngineOutputPower;//发动机输出功率
    uint32_t Velocity;//车速
    uint32_t MAF;//进气量(g/s)或进气压力
    uint32_t ChargeAirPressure;//增压压力
    uint32_t FuelConsumption;//耗油量
    uint32_t NOXConcentration;//氮氧传感器浓度
    uint32_t UreaInjectionVolume;//尿素喷射量
    uint32_t EGT;//排气温度
    uint32_t DPFDifferentialPressure;//颗粒捕集器压差
    uint32_t EGRPosition;//EGR 开度
    uint32_t FuelDeliveryPressure;//燃油喷射压力
    uint32_t CoolantTemp;//冷却液温度
    uint32_t BatteryVoltage;//电池电压
    uint32_t AirFlowMeter;//空气流量计传感器
    uint32_t AirPressure;//空气压力传感器
    uint32_t RelativeSolarTP;/*节气门位置*/
    uint32_t AbsoulteSolarTP; //绝对节气门位置
    uint32_t AbsoulteSolarTP_B;//绝对节气门位置B
    uint32_t AbsoulteSolarTP_C;////绝对节气门位置C
    uint32_t AbsoulteSolarTP_D;
    uint32_t AbsoulteSolarTP_E;
    uint32_t AbsoulteSolarTP_G;//绝对节气门位置G
    uint32_t CrankshaftPosition;//曲轴位置传感器
    uint32_t Camshaftposition;//凸轮轴位置传感器
    uint32_t IAT;/*进气温度*/ 
    uint32_t WaterTemperature;//水温
    uint32_t O2S_11;/*氧气传感器*/
    uint32_t O2S_21;
    uint32_t O2S_12;
    uint32_t O2S_22;/*氧气传感器*/
    uint32_t PostOxygen;//氧传感器
    uint32_t KnockSensor;//爆震传感器
    uint32_t CATEMP;/*催化器温度*/
    uint32_t FiresNumber;//失火数
    uint32_t FuelPulseWidth;//喷油脉冲宽度
    uint32_t EngineLoad;//发动机负荷
    uint32_t SHRTFT1;/*长短期燃油较正*/
    uint32_t LONGFT1;
    uint32_t LgnitionAdvanceAngle;//点火提前角
}RT_DATA;

typedef struct{
    bool ODO_Flag;
    bool Mileage_Flag;
    bool ThrottlePosition_Flag;
    bool EngineRev_Flag;
    bool EngineOutputPower_Flag;
    bool Velocity_Flag;
    bool MAF_Flag;
    bool ChargeAirPressure_Flag;
    bool FuelConsumption_Flag;
    bool NOXConcentration_Flag;
    bool UreaInjectionVolume_Flag;
    bool EGT_Flag;
    bool DPFDifferentialPressure_Flag;
    bool EGRPosition_Flag;
    bool FuelDeliveryPressure_Flag;
    bool CoolantTemp_Flag;
    bool BatteryVoltage_Flag;
    bool AirFlowMeter_Flag;
    bool AirPressure_Flag;
    bool RelativeSolarTP_Flag;/*节气门位置*/
    bool AbsoulteSolarTP_Flag;
    bool AbsoulteSolarTP_B_Flag;
    bool AbsoulteSolarTP_C_Flag;
    bool AbsoulteSolarTP_D_Flag;
    bool AbsoulteSolarTP_E_Flag;
    bool AbsoulteSolarTP_G_Flag;/*节气门位置*/
    bool CrankshaftPosition_Flag;
    bool Camshaftposition_Flag;
    bool IAT_Flag;/*进气温度*/   
    bool WaterTemperature_Flag;
    bool O2S_11_Flag;/*氧气传感器*/
    bool O2S_12_Flag;/*氧气传感器*/
    bool O2S_21_Flag;/*氧气传感器*/
    bool O2S_22_Flag;/*氧气传感器*/
    bool PostOxygen_Flag;
    bool KnockSensor_Flag;
    bool CATEMP_Flag;/*催化器温度*/
    bool FiresNumber_Flag;
    bool FuelPulseWidth_Flag;
    bool EngineLoad_Flag;
    bool SHRTFT1_Flag;/*长短期燃油较正*/
    bool LONGFT1_Flag;
    bool LgnitionAdvanceAngle_Flag;
}RT_DATA_Flag;

typedef struct{

    SYSTEM_CHECK_STATE SystemCheckState;
    ECU_INFO ECUInfo;
    DTC_INFO DTCInfo;
    IUPR Iupr;
    RT_DATA RTData;
    RT_DATA_Flag RTDataFlag;
    
}OBD_DATA;



#endif
