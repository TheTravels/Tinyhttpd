#ifndef _DIAG_H_
#define _DIAG_H_


#define FuelType 1//���ͳ�
#define DieselType 2//���ͳ�

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
    uint8_t CurrentDTC[100];  //��ǰ���ϴ���
    uint8_t UnsettledDTC[100];//δ������ϴ���
    uint32_t Current;//��ǰ���ϴ�������
    uint32_t Unsettled;//δ������ϴ�������
    uint32_t Mileage;
}DTC_INFO;

typedef struct{
    /*��������*/
    uint32_t OMCEC;
    uint32_t ICC;
    /*���ͳ�IUPR����*/
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
    /*���ͳ�IUPR����*/
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
    uint32_t ODO; //�����ۼ���ʻ���
    uint32_t Mileage;//MIL �Ƶ�������ʻ�����  
    uint32_t ThrottlePosition;//���ſ���
    uint32_t EngineRev;//������ת��
    uint32_t EngineOutputPower;//�������������
    uint32_t Velocity;//����
    uint32_t MAF;//������(g/s)�����ѹ��
    uint32_t ChargeAirPressure;//��ѹѹ��
    uint32_t FuelConsumption;//������
    uint32_t NOXConcentration;//����������Ũ��
    uint32_t UreaInjectionVolume;//����������
    uint32_t EGT;//�����¶�
    uint32_t DPFDifferentialPressure;//����������ѹ��
    uint32_t EGRPosition;//EGR ����
    uint32_t FuelDeliveryPressure;//ȼ������ѹ��
    uint32_t CoolantTemp;//��ȴҺ�¶�
    uint32_t BatteryVoltage;//��ص�ѹ
    uint32_t AirFlowMeter;//���������ƴ�����
    uint32_t AirPressure;//����ѹ��������
    uint32_t RelativeSolarTP;/*������λ��*/
    uint32_t AbsoulteSolarTP; //���Խ�����λ��
    uint32_t AbsoulteSolarTP_B;//���Խ�����λ��B
    uint32_t AbsoulteSolarTP_C;////���Խ�����λ��C
    uint32_t AbsoulteSolarTP_D;
    uint32_t AbsoulteSolarTP_E;
    uint32_t AbsoulteSolarTP_G;//���Խ�����λ��G
    uint32_t CrankshaftPosition;//����λ�ô�����
    uint32_t Camshaftposition;//͹����λ�ô�����
    uint32_t IAT;/*�����¶�*/ 
    uint32_t WaterTemperature;//ˮ��
    uint32_t O2S_11;/*����������*/
    uint32_t O2S_21;
    uint32_t O2S_12;
    uint32_t O2S_22;/*����������*/
    uint32_t PostOxygen;//��������
    uint32_t KnockSensor;//���𴫸���
    uint32_t CATEMP;/*�߻����¶�*/
    uint32_t FiresNumber;//ʧ����
    uint32_t FuelPulseWidth;//����������
    uint32_t EngineLoad;//����������
    uint32_t SHRTFT1;/*������ȼ�ͽ���*/
    uint32_t LONGFT1;
    uint32_t LgnitionAdvanceAngle;//�����ǰ��
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
    bool RelativeSolarTP_Flag;/*������λ��*/
    bool AbsoulteSolarTP_Flag;
    bool AbsoulteSolarTP_B_Flag;
    bool AbsoulteSolarTP_C_Flag;
    bool AbsoulteSolarTP_D_Flag;
    bool AbsoulteSolarTP_E_Flag;
    bool AbsoulteSolarTP_G_Flag;/*������λ��*/
    bool CrankshaftPosition_Flag;
    bool Camshaftposition_Flag;
    bool IAT_Flag;/*�����¶�*/   
    bool WaterTemperature_Flag;
    bool O2S_11_Flag;/*����������*/
    bool O2S_12_Flag;/*����������*/
    bool O2S_21_Flag;/*����������*/
    bool O2S_22_Flag;/*����������*/
    bool PostOxygen_Flag;
    bool KnockSensor_Flag;
    bool CATEMP_Flag;/*�߻����¶�*/
    bool FiresNumber_Flag;
    bool FuelPulseWidth_Flag;
    bool EngineLoad_Flag;
    bool SHRTFT1_Flag;/*������ȼ�ͽ���*/
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
