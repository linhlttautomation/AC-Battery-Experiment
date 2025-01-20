#include "IQmathLib.h"
#include "F2837xD_Cla_defines.h"
#include "F2837xD_EPwm_defines.h"

#include "PV.h"
#include "PV_Shared.h"
#include "PeripheralHeaderIncludes.h"

#include "clb.h"
#include "clb_config.h"

interrupt void CANIntHandler(void);
__interrupt void epwm_isr(void);

//================================================================
// Khai bao cac ham
void DeviceInit(void);
void CLA_Init();
void PieCntlInit(void);
void PieVectTableInit(void);

void TripZone_Protection(void);
void CMPSS_Protection(void);

void SendMsgCan(uint16_t ID,uint16_t data1,uint16_t data2 ,uint16_t data3,uint16_t data4);
void ReciveCan(tCANMsgObject *pMsgObject, MsgDataCan *data);

//====================================================================

//====================================================================
// Alpha states
void A0(void);  //state A0
void B0(void);  //state B0
void C0(void);  //state C0

// A branch states    : A- TASKS
void STOP_STATE(void);
void RUN_STATE(void);
void CONFIG_STATE(void);
void CHECKING_STATE(void);

// B branch states    : B- TASKS
void SEND_CAN(void);

// C branch states
void C1(void);  //state C1

// Variable declarations
void (*Alpha_State_Ptr)(void);  // Base States pointer
void (*A_Task_Ptr)(void);       // State pointer A branch
void (*B_Task_Ptr)(void);       // State pointer B branch
void (*C_Task_Ptr)(void);       // State pointer C branch

static inline void dlog1(Uint16 value);
static inline void dlog2(Uint16 value);
static inline void dlog3(Uint16 value);

//-------------Log Data --------------------
//
#define DLOG_SIZE_1 1000
Uint16 DataLog1[DLOG_SIZE_1];
#pragma DATA_SECTION(DataLog1, "DLOG");

#define DLOG_SIZE_2 1000
Uint16 DataLog2[DLOG_SIZE_2];
#pragma DATA_SECTION(DataLog2, "DLOG");

#define DLOG_SIZE_3 1000
Uint16 DataLog3[DLOG_SIZE_3];
#pragma DATA_SECTION(DataLog3, "DLOG");

Uint16 ndx1 = 0;
Uint16 ndx2 = 0;
Uint16 ndx3 = 0;

int16   VTimer0[4];                 // Virtual Timers slaved off CPU Timer 0
int16   VTimer1[4];                 // Virtual Timers slaved off CPU Timer 1
int16   VTimer2[4];                 // Virtual Timers slaved off CPU Timer 2

PWMGEN pwm1 = PWMGEN_DEFAULTS;
//==========================================================================
// CMPSS parameters for Over Current Protection
Uint16  clkPrescale = 6,
        sampwin     = 30,
        thresh      = 18,
        LEM_curHi   = LEM(40.0),
        LEM_curLo   = LEM(40.0);

float curLimit = 50.0;
float curPeakLimit = 50.0;

//==========================================================================
// CAN
volatile unsigned long g_ulTxMsgCount = 0;
volatile unsigned long g_ulRxMsgCount = 0;
unsigned long u32CanAErrorStatus;
volatile unsigned long g_bErrFlag = 0;     // A flag to indicate that some
volatile unsigned long ulStatus;

// Transmission error occurred.
tCANMsgObject sTXCANMessage;
tCANMsgObject sRXCANMessage;
tCANBitClkParms canbit = {5,4,1,40};

MsgDataCan TxMsgData,RxMsgData;
unsigned char ucTXMsgData[8] ={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00 };
unsigned char ucRXMsgData[8];
unsigned int TxData[4];
unsigned int RxID, TxID = 0x00;
//===================================================================
unsigned int signal_mode = 0;                           // Bien xac dinh trạng thai run, stop, checkinh, config
unsigned int running_state = 0;                         // Bien xac dinh trạng thai run
unsigned int checking_state = 0;                        // Bien xac dinh trạng thai checking
unsigned int config_state = 0;                          // Bien xac dinh trạng thai config
unsigned int stop_state = 0;                            // Bien xac dinh trạng thai stop
unsigned int checking_error = 0;                        // Bien xac dinh loi checking
unsigned int signal_view_can = 0;                       // Bien luu trang thai tin hieu can truyen len
unsigned int check_run_error = 0;                       // Bien xac dinh loi run
unsigned int check_stop_error = 0;                      // Bien xac dinh loi stop
unsigned int send_can_loop = 0;                         // Bien vong lap gui can
unsigned int signal_response = 0;                       // Bien luu ID can phan hoi lai Master
unsigned int run_flag = 0;                              // Bien gia tri thoi gian run ok
unsigned int stop_flag = 0;                             // Bien gia tri thoi gian stop ok
unsigned int test_can = 0;                              // Bien gia tri test can

unsigned long time_power = 0;                            // Bien gia tri thoi gian cong suat
float Freq_AC = 0, Pow_AC_Grid = 0,Pow_AC_Load = 0,Pow_AC_Total = 0; // Bien gia tri công suất tính toán

//===================================================================
SETTING_PV Setting_PV;
LIMIT_PV Limit;

Uint16 START = 0;
Uint16 StartFlagAC = 0;
Uint16 StopFlagAC = 0;

Uint16 error = 0;
Uint16 errorFlag = 0;

int ChannelAdc = 0;

Uint16 Task1_Isr = 0;
Uint16 Task8_Isr = 0;
Uint16 Pwm_Isr = 0;

Uint16 trip1 = 0;
Uint16 trip2 = 0;
Uint16 trip3 = 0;
Uint16 trip4 = 0;

// adc static cal
int *adc_cal;

Uint16 RESET_IGBT = 0;
Uint16 ResetIGBTFlag = 0;
Uint16 TripFlagIGBT = 0;
Uint16 reset_charge = 0;
Uint16 OverCurrentMeasure = 0;
Uint16 clearTripFlagDMC = 0;

Uint16 OverCurrentChargeCap = 0;
Uint16 ChargeCapFlag = 1;

void main(){

    DeviceInit();       // init clock system, clock ngoai vi , set RAM or ROM

    //---------------------------------------------------------------------------

////Viet them
//    CpuTimer0Regs.TCR.bit.TSS = 1;         // Dừng CPU Timer 0
//    CpuTimer0Regs.TCR.bit.TRB = 1;         // Nạp lại giá trị bộ đếm
//    CpuTimer0Regs.TCR.bit.TIE = 1;         // Kích hoạt ngắt Timer (có thể không cần nếu không muốn ngắt)
//    CpuTimer0Regs.TCR.bit.TSS = 0;         // Bắt đầu Timer
////Viet them
    CpuTimer0Regs.PRD.all =  mSec5;     // A tasks
    CpuTimer1Regs.PRD.all =  mSec100;   // B tasks
    CpuTimer2Regs.PRD.all =  mSec1000;  // C tasks

    // Tasks State-machine init
    Alpha_State_Ptr = &A0;
    A_Task_Ptr = &STOP_STATE;
    B_Task_Ptr = &SEND_CAN;
    C_Task_Ptr = &C1;

    /*************************************************************************/
    CLA_Init();

    Cla1ForceTask8();
    Init_ADC_A();
    Init_ADC_B();
    Init_ADC_C();

    //
   // Initialize datalog
   //
       for(ndx1=0; ndx1<DLOG_SIZE_1; ndx1++)
       {
           DataLog1[ndx1] = 0;
       }
       ndx1 = 0;

       for(ndx2=0; ndx2<DLOG_SIZE_2; ndx2++)
       {
           DataLog2[ndx2] = 0;
       }
       ndx2 = 0;

       for(ndx3=0; ndx3<DLOG_SIZE_3; ndx3++)
       {
           DataLog3[ndx3] = 0;
       }
       ndx3 = 0;
    // Initialize the PWM module

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    pwm1.PeriodMax = (FREQUENCY_CLOCK_PWM/(2*SVM_SYSTEM)); /* Initializa ePWM */
    PWM_3PHASE_CONF(pwm1.PeriodMax, 60);
    //CMPSS_Protection();

    //------------------------PWM4------------------------------------
    PWM_DAC_CNF(7,125); // 20Khz :500; 40kHz: 250; 80 kHz: 125
    PWM_DAC_CNF(8,125); // 20Khz :500; 40kHz: 250; 80 kHz: 125

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    /**************************************************************************/

    // Step 7. Enable global Interrupts and higher priority real-time debug events:
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    //---------------------------------------------------------
    CANInit(CANA_BASE);                             // Initialize the CAN controller
    CANClkSourceSelect(CANA_BASE, 0);               // Setup CAN to be clocked off the PLL output clock (500kHz CAN-Clock)
    CANBitRateSet(CANA_BASE, 200000000, 500000);    //the CAN bus is set to 500 kHz
    CANBitTimingSet(CANA_BASE, &canbit);
    CANIntEnable(CANA_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);  // Enable interrupts on the CAN peripheral.

    // ***************************************************************
    EALLOW;
    PieVectTable.CANA0_INT = &CANIntHandler;
    PieVectTable.EPWM1_INT = &epwm_isr;

    PieCtrlRegs.PIEIER11.bit.INTx1 = 1;  // CLA 1
    IER |= M_INT11;
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;   // CAN A
    IER |= M_INT9;  // M_INT9
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;   // PWM 1
    IER |= M_INT3;
    EINT;
    EDIS;

    //-----------------------------------------------------------------------------------
    CANEnable(CANA_BASE);                                 // Enable the CAN for operation.
    CANGlobalIntEnable(CANA_BASE, CAN_GLB_INT_CANINT0);   // Enable CAN Global Interrupt line0

    sTXCANMessage.ui32MsgID = 1;                      // CAN message ID - use 1
    sTXCANMessage.ui32MsgIDMask = 1;                  // no mask needed for TX
    sTXCANMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;  // enable interrupt on TX
    sTXCANMessage.ui32MsgLen = sizeof(ucTXMsgData);   // size of message is 8
    sTXCANMessage.pucMsgData = ucTXMsgData;           // ptr to message content

    *(unsigned long *)ucRXMsgData = 0;
    sRXCANMessage.ui32MsgID = 0;                      // CAN message ID - use 1
    sRXCANMessage.ui32MsgIDMask = 0x00;                 // no mask needed for TX
    sRXCANMessage.ui32Flags = MSG_OBJ_USE_ID_FILTER|MSG_OBJ_RX_INT_ENABLE;  // enable interrupt on RX
    sRXCANMessage.ui32MsgLen = sizeof(ucRXMsgData);   // size of message is 8
    sRXCANMessage.pucMsgData = ucRXMsgData;           // ptr to message content

    // Setup the message object being used to receive messages
    CANMessageSet(CANA_BASE, 2, &sRXCANMessage, MSG_OBJ_TYPE_RX);

    //==================================================================================
    // Khoi tao ngoai vi

    GPIO_Init();
//    Setup_CLB();

    VTimer0[0] = 0;
    VTimer1[0] = 0;
    VTimer2[0] = 0;

    /*****************************************************************************/
    // Cpu To CLA
    CpuToCLA.EnableADC = 0;
    DelayMs(1000);
    CpuToCLA.ControlMode = GRID_CONNECTED_MODE;
    CpuToCLA.EnableADC = 1;
    CpuToCLA.EnableFlag = 0;
    CpuToCLA.UdcTesting = 650.0/Udc_max;

    #if(BUILDLEVEL == LEVEL1)
        CpuToCLA.VdTesting = 220.0/Us_max;
    #endif

    #if(BUILDLEVEL == LEVEL4)
        CpuToCLA.VdTesting = 10.0/Us_max;
    #endif

    CpuToCLA.VqTesting = 0;
    CpuToCLA.VzTesting = 0;
    CpuToCLA.Fref = 50.00/BASE_FREQ;

    //------------------------------------------------------------------------------
    // SETTING PV
    #if(BUILDLEVEL == LEVEL1)
        Setting_PV.Voltage = PV_VOLTAGE;
    #endif

    #if(BUILDLEVEL == LEVEL4)
        Setting_PV.Voltage = 110.0;
    #endif

    Setting_PV.Frequency = PV_FREQUENCY;
    Setting_PV.Power = PV_POWER;
    Setting_PV.Current = PV_CURRENT;
    Setting_PV.HeSo = PV_COSPHI;
    Setting_PV.Udc = PV_UDC;

    Setting_PV.ControlMode = GRID_CONNECTED_MODE;

    Setting_PV.FrequencyMax = PV_FREQUENCY_MAX;
    Setting_PV.FrequencyMin = PV_FREQUENCY_MIN;

    Setting_PV.VgridMax = PV_VGRID_MAX;
    Setting_PV.VgridMin = PV_VGRID_MIN;
    Setting_PV.IgridMax = PV_IGRID_MAX;

    Setting_PV.VLoadMax = PV_VLOAD_MAX;
    Setting_PV.VLoadMin = PV_VLOAD_MIN;
    Setting_PV.ILoadMax = PV_ILOAD_MAX;

    Setting_PV.UdcMax = PV_UDC_MAX;
    Setting_PV.UdcMin = PV_UDC_MIN;
    Setting_PV.TemperatureMax = PV_TEMP_MAX;

//    // Thong so ADC
    CpuToCLA.ADCoffset_Udc = 21;
    CpuToCLA.ADCoffset_VaG = 2140;
    CpuToCLA.ADCoffset_VbG = 2095;
    CpuToCLA.ADCoffset_VcG = 2092;
    CpuToCLA.ADCoffset_Ia_inv = 2093;
    CpuToCLA.ADCoffset_Ib_inv = 2105;
    CpuToCLA.ADCoffset_Ic_inv = 2063;

    CpuToCLA.ADCgain_Udc = 0.97;
    CpuToCLA.ADCgain_VaG = 0.94;
    CpuToCLA.ADCgain_VbG = 0.9;
    CpuToCLA.ADCgain_VcG = 0.9;
    CpuToCLA.ADCgain_Ia_inv = 1.05;
    CpuToCLA.ADCgain_Ib_inv = 1.09;
    CpuToCLA.ADCgain_Ic_inv = 0.92;

    //---------------------------------------------------------------------------------
    EALLOW;
    GpioCtrlRegs.GPBGMUX1.bit.GPIO32 = 0;   //
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;    // 0=GPIO,  1=Resv,  2=Resv,  3=COMP2OUT
    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;     // 1=OUTput,  0=INput
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;     // Enable pull-up
    DO_LOW
    EDIS;
    ENABLE_PWM_MARCO;

    //-----------------------------------------------------------------------------------

    #if(BUILDLEVEL == LEVEL1||BUILDLEVEL == LEVEL2||BUILDLEVEL == LEVEL3||BUILDLEVEL == LEVEL4)
    ENABLE_LOAD_ON_MARCO;
    ENABLE_GRID_OFF_MARCO;
    CHARGE_CAP_ON_MARCO;
    Setting_PV.VgridMin = 0;
    #endif

    #if(BUILDLEVEL == LEVEL5)
    ENABLE_LOAD_OFF_MARCO;
    ENABLE_GRID_ON_MARCO;
    CHARGE_CAP_ON_MARCO;
    ChargeCapFlag = 1;
    Setting_PV.VLoadMin = 0;
    #endif

    #if(BUILDLEVEL == LEVEL6)
    ENABLE_LOAD_OFF_MARCO;
    ENABLE_GRID_ON_MARCO;
    CHARGE_CAP_OFF_MARCO;
    Setting_PV.VLoadMin = 0;
    ChargeCapFlag = 0;
    #endif

    #if(BUILDLEVEL == LEVELALL)
    CHARGE_CAP_OFF_MARCO;

    ENABLE_LOAD_ON_MARCO;
    ENABLE_GRID_ON_MARCO;

    Setting_PV.VLoadMin = 0;

    ChargeCapFlag = 0;
    reset_charge = 0;

    #endif

    DelayMs(1000);

    clearTripFlagDMC = 1;
    RESET_IGBT = 1;

    while(1)
     {
        //====================================================

        if(RESET_IGBT == 1)
        {
            if(ResetIGBTFlag == 0)
            {
               TripFlagIGBT = 0;
               ResetIGBT();
               if(EPwm1Regs.TZFLG.bit.OST == 0x1)          // TripZ for PWMs is low (fault trip)
               {
                  trip1++;
                  EALLOW; // Clear flag TripZone (Cycle-By-Cycle Flag Clear)
                  EPwm1Regs.TZCLR.bit.OST = 1;
                  EPwm2Regs.TZCLR.bit.OST = 1;
                  EPwm3Regs.TZCLR.bit.OST = 1;
                  EPwm4Regs.TZCLR.bit.OST = 1;
                  EDIS;
              }
            }
            ResetIGBTFlag = 1;
            RESET_IGBT = 0;
        }
        else
        {
            ResetIGBTFlag = 0;
        }

        //----------------------------------------------------------
        if(clearTripFlagDMC == 1)
        {
            TripFlagIGBT = 0;
            OverCurrentMeasure = 0;
            if(EPwm1Regs.TZFLG.bit.OST == 0x1)          // TripZ for PWMs is low (fault trip)
            {
               trip2++;
               EALLOW; // Clear flag TripZone (One Shot)
               EPwm1Regs.TZCLR.bit.OST = 1;
               EPwm2Regs.TZCLR.bit.OST = 1;
               EPwm3Regs.TZCLR.bit.OST = 1;
               EPwm4Regs.TZCLR.bit.OST = 1;
               EDIS;
           }

           if(EPwm1Regs.TZFLG.bit.DCAEVT1 == 0x01 ||EPwm1Regs.TZFLG.bit.DCBEVT1 == 0x01)
           {
              EALLOW;
              // clear DCAEVT1 flags
              EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;
              EPwm2Regs.TZCLR.bit.DCAEVT1 = 1;
              EPwm3Regs.TZCLR.bit.DCAEVT1 = 1;
              EPwm4Regs.TZCLR.bit.DCAEVT1 = 1;

              // clear DCBEVT1 flags
              EPwm1Regs.TZCLR.bit.DCBEVT1 = 1;
              EPwm2Regs.TZCLR.bit.DCBEVT1 = 1;
              EPwm3Regs.TZCLR.bit.DCBEVT1 = 1;
              EPwm4Regs.TZCLR.bit.DCBEVT1 = 1;

              // clear HLATCH - (not in TRIP gen path)
              Cmpss4Regs.COMPSTSCLR.bit.HLATCHCLR = 1;
              Cmpss7Regs.COMPSTSCLR.bit.HLATCHCLR = 1;
              Cmpss8Regs.COMPSTSCLR.bit.HLATCHCLR = 1;

              // clear LLATCH - (not in TRIP gen path)
              Cmpss4Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
              Cmpss7Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
              Cmpss8Regs.COMPSTSCLR.bit.LLATCHCLR = 1;

              EDIS;
           }
           clearTripFlagDMC = 0;
        }

        if(START == 0 && OverCurrentMeasure == 1) clearTripFlagDMC = 1;
        if(START == 0 && TripFlagIGBT == 1) RESET_IGBT = 1;

        //=========================================================

         if(START == 1)
         {
            // LEVEL1
            #if(BUILDLEVEL == LEVEL1)
            if(Setting_PV.Voltage > PV_VLOAD_MAX) Setting_PV.Voltage = PV_VLOAD_MAX;
            CpuToCLA.VdTesting = ((float)Setting_PV.Voltage)/Us_max;
            CpuToCLA.EnableFlag = 1;
            #endif

            // LEVEL2
            #if(BUILDLEVEL == LEVEL2)
            if(Setting_PV.Voltage > PV_VLOAD_MAX) Setting_PV.Voltage = PV_VLOAD_MAX;
            CpuToCLA.UsTesting = ((float)Setting_PV.Voltage)/Us_max;
            CpuToCLA.VdTesting = ((float)Setting_PV.Voltage)/Us_max;
            CpuToCLA.EnableFlag = 1;
            #endif

            // LEVEL3
            #if(BUILDLEVEL == LEVEL3)
            if(Setting_PV.Current > PV_ILOAD_MAX) Setting_PV.Current = PV_ILOAD_MAX;
            CpuToCLA.IdRef = ((float)Setting_PV.Current)/Is_max;
            CpuToCLA.IqRef = 0;
            CpuToCLA.EnableFlag = 1;
            #endif
            // LEVEL4
            #if(BUILDLEVEL == LEVEL4)
            if(Setting_PV.Voltage > PV_VLOAD_MAX) Setting_PV.Voltage = PV_VLOAD_MAX;
            CpuToCLA.VdTesting = ((float)Setting_PV.Voltage)/Us_max;
//            CpuToCLA.UdRef = ((float)Setting_PV.Voltage)/Us_max;
            CpuToCLA.UqRef = 0;
            CpuToCLA.EnableFlag = 1;
            #endif

            // LEVEL5
            #if(BUILDLEVEL == LEVEL5)
            if(Setting_PV.Current > PV_CURRENT) Setting_PV.Current = PV_CURRENT;
            CpuToCLA.IdRef = ((float)Setting_PV.Current)/Is_max;
            CpuToCLA.IqRef = 0;
            CpuToCLA.IzRef = 0;
            CpuToCLA.EnableFlag = 1;
            #endif

            // LEVEL6
            #if(BUILDLEVEL == LEVEL6)
            ENABLE_LOAD_ON_MARCO;
            if(Setting_PV.Udc > PV_UDC_MAX) Setting_PV.Udc = PV_UDC_MAX;
            CpuToCLA.UdcRef = ((float)Setting_PV.Udc)/Udc_max;
            CpuToCLA.IqRef = 0;
            CpuToCLA.IzRef = 0;
            if(reset_charge == 0 )
            {
                 CHARGE_CAP_ON_MARCO;
                 DelayMs(500);
                 reset_charge = 1;
                 ChargeCapFlag = 1;
            }
            StartFlagAC = 1;
            CpuToCLA.EnableFlag = 1;
            #endif

            // LEVELALL
            #if(BUILDLEVEL == LEVELALL)

            if(Setting_PV.Voltage > PV_VLOAD_MAX) Setting_PV.Voltage = PV_VLOAD_MAX;
            CpuToCLA.UdRef = ((float)Setting_PV.Voltage)/Us_max;
            CpuToCLA.UqRef = 0;

            if(Setting_PV.Udc > PV_UDC_MAX) Setting_PV.Udc = PV_UDC_MAX;
            CpuToCLA.UdcRef = ((float)Setting_PV.Udc)/Udc_max;
            CpuToCLA.IqRef = 0;

            if(reset_charge == 0)
            {
                 CHARGE_CAP_ON_MARCO;
                 DelayMs(1000);
                 reset_charge = 1;
                 ChargeCapFlag = 1;
            }
            StartFlagAC = 1;

            if(CpuToCLA.EnableFlag == 0)
            {
                if(CpuToCLA.ControlMode == STAND_ALONE_MODE)
                {
                    // Bao ve khi co luoi ma chạy doc lap
                    if(ClaToCPU.MEASUARE_CPU.VaGRms > 20.0/Us_max || ClaToCPU.MEASUARE_CPU.VbGRms > 20.0/Us_max || ClaToCPU.MEASUARE_CPU.VcGRms > 20.0/Us_max)
                    {
                        CpuToCLA.EnableFlag = 0;
                        START = 0;
                        error = ERROR_VGRID_HIGH;
                    }
                    else  CpuToCLA.EnableFlag = 1;
                }
                if(CpuToCLA.ControlMode == GRID_CONNECTED_MODE || CpuToCLA.ControlMode == HYBRID_MODE)
                {
                    CpuToCLA.EnableFlag = 1;
                }
            }

            #endif

         }
         else
         {
             StopFlagAC = 1;
             reset_charge = 0;
             CpuToCLA.EnableFlag = 0;
         }

        // State machine entry & exit point
        //===========================================================
        (*Alpha_State_Ptr)();   // jump to an Alpha state (A0,B0,...)
        //===========================================================
     }



} //END MAIN CODE

//*****************************************************************************
// ISR  CLA
//*****************************************************************************
__interrupt void cla1Isr1 ()
{
    Task1_Isr++;
    static Uint16 i = 0;
    static Uint32 count_start = 0;

    //------------------------------------------------------------------------
     // Protection when charge cap
     if(ChargeCapFlag == 0)
     {
         /*
         if(ClaToCPU.ADC_CPU.Ia_inv > 10.0/Is_max || ClaToCPU.ADC_CPU.Ia_inv < -10.0/Is_max)
         {
             OverCurrentChargeCap = 1;
         }
         if(ClaToCPU.ADC_CPU.Ib_inv > 10.0/Is_max || ClaToCPU.ADC_CPU.Ib_inv < -10.0/Is_max)
         {
             OverCurrentChargeCap = 1;
         }
         if(ClaToCPU.ADC_CPU.Ic_inv > 10.0/Is_max || ClaToCPU.ADC_CPU.Ic_inv < -10.0/Is_max)
         {
             OverCurrentChargeCap = 1;
         }
          */

         if(EPwm1Regs.TZFLG.bit.DCAEVT1 == 0x01 || EPwm1Regs.TZFLG.bit.DCBEVT1 == 0x01)
         {
             OverCurrentChargeCap = 1;
         }
         if(OverCurrentChargeCap == 1)
         {
             ENABLE_GRID_OFF_MARCO;
         }
     }

     //-------------------------------------------------------------------
     // STOP -- Relay charge cap open
     if(CpuToCLA.ControlMode == GRID_CONNECTED_MODE)
     {
         if(START == 0 && StartFlagAC == 1)
         {
             count_start++;          // 2ms: 10
             if(count_start == 500)  // 100ms
             {
                 CHARGE_CAP_OFF_MARCO;
                 StartFlagAC = 0;
                 count_start = 0;
             }
         }
     }

     if(CpuToCLA.ControlMode == STAND_ALONE_MODE)
     {
         if(START == 0 && StartFlagAC == 1)
         {
             count_start++;          // 2ms: 10
             if(count_start == 25000)  // 5s
             {
                 CHARGE_CAP_OFF_MARCO;
                 StartFlagAC = 0;
                 count_start = 0;
                 ENABLE_GRID_ON_MARCO;
             }
         }
         if(START == 1 && StopFlagAC == 1)
         {
             StopFlagAC = 0;
             ENABLE_GRID_OFF_MARCO;
         }
     }

    //--------------------------------------------------------------------------------
    // Datalog


    dlog1(0.5*(ClaToCPU.ADC_CPU.datalog1+1)*20000);
    dlog2(0.5*(ClaToCPU.ADC_CPU.datalog2+1)*20000);
    dlog3(0.5*(ClaToCPU.ADC_CPU.datalog3+1)*20000);
    //dlog1(ClaToCPU.ADC_CPU.datalog1*20000);

// dlog1(ClaToCPU.ADC_CPU.datalog1*1000);
// dlog2(ClaToCPU.ADC_CPU.datalog2*1000);


    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    PieCtrlRegs.PIEACK.all = (PIEACK_GROUP1 | PIEACK_GROUP11);

    // Acknowledge the end-of-task interrupt for task 1
    PieCtrlRegs.PIEACK.all = M_INT11;//0400=0000 0100 0000 0000

}

__interrupt void cla1Isr2 ()
{asm(" ESTOP0");}
__interrupt void cla1Isr3 ()
{asm(" ESTOP0");}
__interrupt void cla1Isr4 ()
{asm(" ESTOP0");}
__interrupt void cla1Isr5 ()
{asm(" ESTOP0");}
__interrupt void cla1Isr6 ()
{asm(" ESTOP0");}
__interrupt void cla1Isr7 ()
{asm(" ESTOP0");}

__interrupt void cla1Isr8 ()
{
    Task8_Isr++;
    // Acknowledge the end-of-task interrupt for task 8
    PieCtrlRegs.PIEACK.all = M_INT11;
}

interrupt void CANIntHandler(void)
{
    // Read the CAN interrupt status to find the cause of the interrupt
    ulStatus = CANIntStatus(CANA_BASE, CAN_INT_STS_CAUSE);

    // If the cause is a controller status interrupt, then get the status
    if(ulStatus == CAN_INT_INT0ID_STATUS)
    {

        ulStatus = CANStatusGet(CANA_BASE, CAN_STS_CONTROL);

        if(((ulStatus  & ~(CAN_ES_TXOK | CAN_ES_RXOK)) != 7) &&
           ((ulStatus  & ~(CAN_ES_TXOK | CAN_ES_RXOK)) != 0))
        {
            g_bErrFlag = 1;
        }
    }

    // Ngat truyen CAN
    else if(ulStatus == 1)
    {
        CANIntClear(CANA_BASE, 1);
        g_ulTxMsgCount++;
        g_bErrFlag = 0;
    }

    // Xu ly du lieu CAN nhan duoc
    else if(ulStatus == 2)
    {
        CANMessageGet(CANA_BASE, 2, &sRXCANMessage, true);
        CANIntClear(CANA_BASE, 2);
        g_ulRxMsgCount++;
        g_bErrFlag = 0;
        ReciveCan(&sRXCANMessage, &RxMsgData);

        switch (RxMsgData.ID)
        {
        case MASTER_CONFIG_CONTROL_MODE:
            Setting_PV.ControlMode = RxMsgData.WORD_ONE;
            CpuToCLA.ControlMode   = RxMsgData.WORD_ONE;

            if(CpuToCLA.ControlMode == GRID_CONNECTED_MODE|| CpuToCLA.ControlMode == HYBRID_MODE)
            {
                Setting_PV.UdcMax = 800;
                Setting_PV.UdcMin = 200;

                Setting_PV.VgridMax = 250;
                Setting_PV.VgridMin = 180;

                Setting_PV.IgridMax = 40;

                Setting_PV.VLoadMax = 250;
                Setting_PV.VLoadMin = 180;

                Setting_PV.ILoadMax = 50;

                Setting_PV.FrequencyMax = 70;
                Setting_PV.FrequencyMin = 30;

                Setting_PV.TemperatureMax = 100;
            }

            if(CpuToCLA.ControlMode == STAND_ALONE_MODE)
            {
                Setting_PV.UdcMax = 800;
                Setting_PV.UdcMin = 0;

                Setting_PV.VgridMax = 250;
                Setting_PV.VgridMin = 0;

                Setting_PV.IgridMax = 40;

                Setting_PV.VLoadMax = 250;
                Setting_PV.VLoadMin = 0;

                Setting_PV.ILoadMax = 40;

                Setting_PV.FrequencyMax = 70;
                Setting_PV.FrequencyMin = 0;

                Setting_PV.TemperatureMax = 100;
            }

            break;
        case MASTER_RUN_SLAVE_DCAC:
            signal_mode = ST_RUN;
            break;
        case MASTER_STOP_SLAVE_DCAC:
            signal_mode = ST_STOP;
            break;
        case MASTER_CONFIG_SLAVE_DCAC:
            signal_mode = ST_CONFIG;
            break;
        case MASTER_CHECKING_SLAVE_DCAC:
            signal_mode = ST_CHECKING;
            break;
        case MASTER_REQUEST_VDC:
            signal_response = SLAVE_RESPOSE_VDC;
            break;
        case MASTER_REQUEST_IGRID:
            signal_response = SLAVE_RESPOSE_IGRID;
            break;
        case MASTER_REQUEST_VGRID:
            signal_response = SLAVE_RESPOSE_VGRID;
            break;
        case MASTER_REQUEST_FREQ_GRID:
            signal_response = SLAVE_RESPOSE_FREQ;
            break;
        case MASTER_REQUEST_POW_GRID:
            signal_response = SLAVE_RESPOSE_POW_GRID;
            break;
        case MASTER_REQUEST_ILOAD:
            signal_response = SLAVE_RESPOSE_ILOAD;
            break;
        case MASTER_REQUEST_VLOAD:
            signal_response = SLAVE_RESPOSE_VLOAD;
            break;
        case MASTER_REQUEST_POW_LOAD:
            signal_response = SLAVE_RESPOSE_POW_LOAD;
            break;
        }
        switch(signal_mode)
          {
          case ST_CONFIG:
          {
              config_state = RxMsgData.ID;
              switch (config_state)
              {
              case MASTER_CONFIG_LIMIT_VDC:
                  Setting_PV.UdcMax = RxMsgData.WORD_SEC;
                  Setting_PV.UdcMin = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_VGRID:
                  Setting_PV.VgridMax = RxMsgData.WORD_SEC;
                  Setting_PV.VgridMin = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_IGRID_MAX:
                  Setting_PV.IgridMax = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_VLOAD:
                  Setting_PV.VLoadMax = RxMsgData.WORD_SEC;
                  Setting_PV.VLoadMin = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_ILOAD_MAX:
                  Setting_PV.ILoadMax = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_FREQ:
                  Setting_PV.FrequencyMax = RxMsgData.WORD_SEC;
                  Setting_PV.FrequencyMin = RxMsgData.WORD_ONE;
                  break;
              case MASTER_CONFIG_LIMIT_TEMP_MAX:
                  Setting_PV.TemperatureMax = RxMsgData.WORD_ONE;
                  break;
              case MASTER_SET_VDC_REF:
                  Setting_PV.Udc = RxMsgData.WORD_ONE;
                  break;
              case MASTER_SET_VAC_REF:
                  Setting_PV.Voltage = RxMsgData.WORD_ONE;
                  break;
              }
          }
          break;
          case ST_CHECKING:
          {
              checking_state = 0;
              if(Setting_PV.ControlMode != STAND_ALONE_MODE)
              {
                  // Udc_max
                  if(ClaToCPU.ADC_CPU.Udc > Limit.UdcMax)           // OverUdc
                  {
                      checking_state = ERROR_VDC_HIGH;
                  }
                  // Udc_min
                  if(ClaToCPU.ADC_CPU.Udc < Limit.UdcMin)           // LowDC
                  {
                      checking_state = ERROR_VDC_LOW;
                  }

                  // Vgrid_MAX
                  if(ClaToCPU.MEASUARE_CPU.VaGRms > Limit.VgridMax||ClaToCPU.MEASUARE_CPU.VbGRms > Limit.VgridMax||ClaToCPU.MEASUARE_CPU.VcGRms > Limit.VgridMax)
                  {
                      checking_state = ERROR_VGRID_HIGH;
                  }
                  // Vgrid_Min
                  if(ClaToCPU.MEASUARE_CPU.VaGRms < Limit.VgridMin || ClaToCPU.MEASUARE_CPU.VbGRms < Limit.VgridMin || ClaToCPU.MEASUARE_CPU.VcGRms < Limit.VgridMin)
                  {
                      checking_state = ERROR_VGRID_LOW;
                  }

                  // Frequency MAX
                  if(ClaToCPU.MEASUARE_CPU.Freq > Limit.FrequencyMax)         // OverVoltage
                  {
                      checking_state = ERROR_FREQUENCY_HIGH;
                  }
                  // Frequnecy MIN
                  if(ClaToCPU.MEASUARE_CPU.Freq < Limit.FrequencyMin)
                  {
                      checking_state = ERROR_FREQUENCY_LOW;
                  }

              }
              if(checking_state == 0)
              {
                  checking_error = 1;
              }
              if(checking_state != 0)
              {
                  checking_error = 2;
                  signal_mode = 0;
              }

          }
          break;
          case ST_RUN:
          {
              error = 0;
              START = 1;
              running_state = 1;
              check_run_error = 1;
              check_stop_error = 0;
          }
          break;
          case ST_STOP:
          {
              START = 0;
              running_state = 2;
              check_run_error = 0;
              check_stop_error = 1;
          }
          break;
          case ST_SET:
          {
              switch (RxMsgData.ID)
              {
              case MASTER_SET_VDC_REF:
                  Setting_PV.Udc = RxMsgData.WORD_ONE;
                  break;
              case MASTER_SET_VAC_REF:
                  Setting_PV.Voltage = RxMsgData.WORD_ONE;
                  break;
              }
          }
        }
    }

    else
    {
        //
        // Spurious interrupt handling can go here.
        //
    }

    CANGlobalIntClear(CANA_BASE, CAN_GLB_INT_CANINT0);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

//===============================================================================
//=================================================================================
//  STATE-MACHINE SEQUENCING AND SYNCRONIZATION
//=================================================================================

//--------------------------------- FRAMEWORK -------------------------------------
void A0(void)
{
    // loop rate synchronizer for A-tasks
    if(CpuTimer0Regs.TCR.bit.TIF == 1)
    {
        CpuTimer0Regs.TCR.bit.TIF = 1;  // clear flag

        //-----------------------------------------------------------
        (*A_Task_Ptr)();        // jump to an A Task (A1,A2,A3,...)
        //-----------------------------------------------------------

        VTimer0[0]++;           // virtual timer 0, instance 0 (spare)
    }

    Alpha_State_Ptr = &B0;      // Comment out to allow only A tasks
}

void B0(void)
{
    // loop rate synchronizer for B-tasks
    if(CpuTimer1Regs.TCR.bit.TIF == 1)
    {
        CpuTimer1Regs.TCR.bit.TIF = 1;              // clear flag

        //-----------------------------------------------------------
        (*B_Task_Ptr)();        // jump to a B Task (B1,B2,B3,...)
        //-----------------------------------------------------------
        VTimer1[0]++;           // virtual timer 1, instance 0 (spare)
    }

    Alpha_State_Ptr = &C0;      // Allow C state tasks
}

void C0(void)
{
    // loop rate synchronizer for C-tasks
    if(CpuTimer2Regs.TCR.bit.TIF == 1)
    {
        CpuTimer2Regs.TCR.bit.TIF = 1;              // clear flag

        //-----------------------------------------------------------
        (*C_Task_Ptr)();        // jump to a C Task (C1,C2,C3,...)
        //-----------------------------------------------------------
        VTimer2[0]++;           //virtual timer 2, instance 0 (spare)
    }

    Alpha_State_Ptr = &A0;  // Back to State A0
}
//=================================================================================
//  A - TASKS
//=================================================================================
//--------------------------------------------------------
void STOP_STATE(void)  // Dash Board Measurements
//--------------------------------------------------------
{
    EALLOW ; GpioDataRegs.GPDTOGGLE.bit.GPIO125= 1 ; EDIS;
}
//--------------------------------------------------------
void RUN_STATE(void)  // Panel Connect Disconnect
//-----------------------------------------------------------------
{
}
//--------------------------------------------------------
void CONFIG_STATE(void)  // Talk to the Panel Emulator
//-----------------------------------------
{
}
//--------------------------------------------------------
void CHECKING_STATE(void)  // Talk to the Panel Emulator
//-----------------------------------------
{
}
//================================================================================
//  B - TASKS
//=================================================================================
//----------------------------------------
void SEND_CAN(void)  // MPPT Execution
//----------------------------------------
{
    // EALLOW ; GpioDataRegs.GPDTOGGLE.bit.GPIO125= 1 ; EDIS;
    test_can++;
    // Power_Calculator
    time_power++;
    Pow_AC_Load = (ClaToCPU.MEASUARE_CPU.IaLRms*ClaToCPU.MEASUARE_CPU.VaLRms + ClaToCPU.MEASUARE_CPU.IbLRms*ClaToCPU.MEASUARE_CPU.VbLRms + ClaToCPU.MEASUARE_CPU.IcLRms*ClaToCPU.MEASUARE_CPU.VcLRms)*Is_max*Us_max;
    Pow_AC_Grid = (ClaToCPU.MEASUARE_CPU.IaInvRms*ClaToCPU.MEASUARE_CPU.VaGRms + ClaToCPU.MEASUARE_CPU.IaInvRms*ClaToCPU.MEASUARE_CPU.VaGRms + ClaToCPU.MEASUARE_CPU.IaInvRms*ClaToCPU.MEASUARE_CPU.VaGRms)*Is_max*Us_max;
    if (CpuToCLA.ControlMode == GRID_CONNECTED_MODE|| CpuToCLA.ControlMode == HYBRID_MODE)
    Pow_AC_Total = (time_power*Pow_AC_Grid*0.1)/3600.0*1000.0;
    else Pow_AC_Total = (time_power*Pow_AC_Load*0.1)/3600.0*1000.0;

    send_can_loop++;
    if(send_can_loop == 1)
    {
        switch(signal_response)
        {
        case SLAVE_RESPOSE_VDC:
            SendMsgCan(SLAVE_RESPOSE_VDC, ClaToCPU.ADC_CPU.Udc*Udc_max, 0, 0, 0);
            signal_response = 0;
            break;
        case SLAVE_RESPOSE_IGRID:
            SendMsgCan(SLAVE_RESPOSE_IGRID, (uint16_t)(ClaToCPU.MEASUARE_CPU.IaInvRms*Is_max*10), (uint16_t)(ClaToCPU.MEASUARE_CPU.IbInvRms*Is_max*10), (uint16_t)(ClaToCPU.MEASUARE_CPU.IcInvRms*Is_max*10), 0);
            signal_response = 0;
            break;
        case SLAVE_RESPOSE_VGRID:
            SendMsgCan(SLAVE_RESPOSE_VGRID, (uint16_t)(ClaToCPU.MEASUARE_CPU.VaGRms*Us_max), (uint16_t)(ClaToCPU.MEASUARE_CPU.VbGRms*Us_max), (uint16_t)(ClaToCPU.MEASUARE_CPU.VcGRms*Us_max), 0);
            signal_response = 0;
            break;
        case SLAVE_RESPOSE_FREQ:
            if(CpuToCLA.ControlMode == GRID_CONNECTED_MODE|| CpuToCLA.ControlMode == HYBRID_MODE)
            {
                Freq_AC = ClaToCPU.MEASUARE_CPU.Freq;
                if(Freq_AC > 50.5/BASE_FREQ) Freq_AC = 50.2/BASE_FREQ ;
                if(Freq_AC < 49.5/BASE_FREQ) Freq_AC = 49.6/BASE_FREQ ;
            }
            else Freq_AC = 50.0/BASE_FREQ;

            SendMsgCan(SLAVE_RESPOSE_FREQ, (uint16_t)(Freq_AC*BASE_FREQ*10), 0, 0, 0);
            signal_response = 0;
            break;
        case SLAVE_RESPOSE_POW_GRID:
            SendMsgCan(SLAVE_RESPOSE_POW_GRID, (uint16_t)Pow_AC_Grid, (uint16_t)Pow_AC_Load, 0, 0);
            signal_response = 0;
            break;
        case SLAVE_RESPOSE_VLOAD:
            SendMsgCan(SLAVE_RESPOSE_VLOAD, (uint16_t)(ClaToCPU.MEASUARE_CPU.VaLRms*Us_max), (uint16_t)(ClaToCPU.MEASUARE_CPU.VbLRms*Us_max), (uint16_t)(ClaToCPU.MEASUARE_CPU.VcLRms*Us_max), 0);
            signal_response = 0;
            break;
//        case SLAVE_RESPOSE_ILOAD:
//            SendMsgCan(SLAVE_RESPOSE_ILOAD, (uint16_t)(ClaToCPU.MEASUARE_CPU.IaLRms*Iload_max*10),(uint16_t)(ClaToCPU.MEASUARE_CPU.IbLRms*Iload_max*10),(uint16_t)(ClaToCPU.MEASUARE_CPU.IcLRms*Iload_max*10), 0);
//            signal_response = 0;
//            break;
        case SLAVE_RESPOSE_POW_LOAD:
            SendMsgCan(SLAVE_RESPOSE_POW_LOAD, (uint16_t)(Pow_AC_Total), 0, 0, 0);
            signal_response = 0;
            time_power = 0;
            break;
        }
    }
    if(send_can_loop == 2)
    {
        if(checking_error == 1)
        {
            SendMsgCan(SLAVE_DCAC_OK_CHECKING, 0, 0, 0, 0);
            checking_error = 0;
        }
        if(checking_error == 2)
        {
//            SendMsgCan(SLAVE_DCAC_OK_CHECKING, checking_state, checking_state, checking_state, checking_state);
            SendMsgCan(SLAVE_DCAC_ERROR_CHECKING, checking_state, checking_state, checking_state, checking_state);
            checking_error = 0;
        }

        if(signal_mode == ST_RUN && running_state == 1)
        {
            run_flag++;
            if (run_flag >= 10) // 10s
            {
                run_flag = 0;
                if (START == 1)
                {
                    SendMsgCan(SLAVE_DCAC_OK_RUN, 0, 0, 0, 0);
                    running_state = 0;
                    signal_mode = 0;
                }
                else
                {
                    SendMsgCan(SLAVE_DCAC_ERROR_RUN, 0, 0, 0, 0);
                    error = 0;
                }

            }
        }
        if(running_state == 0 && error != 0 && check_run_error ==1)// Run error
        {
            switch(error)
            {
            case ERROR_VDC_HIGH:
                SendMsgCan(SLAVE_ERROR_VDC_HIGH, error, error, error, error);
                error = 0;
                break;
            case ERROR_VDC_LOW:
                SendMsgCan(SLAVE_ERROR_VDC_LOW, error, error, error, error);
                error = 0;
                break;
            case ERROR_VGRID_HIGH:
                SendMsgCan(SLAVE_ERROR_VGRID_HIGH, error, error, error, error);
                error = 0;
                break;
            case ERROR_VGRID_LOW:
                SendMsgCan(SLAVE_ERROR_VGRID_LOW, error, error, error, error);
                error = 0;
                break;
            case ERROR_VLOAD_HIGH:
                SendMsgCan(SLAVE_ERROR_VLOAD_HIGH, error, error, error, error);
                error = 0;
                break;
            case ERROR_VLOAD_LOW:
                SendMsgCan(SLAVE_ERROR_VLOAD_LOW, error, error, error, error);
                error = 0;
                break;
            case ERROR_CURRENT_HIGH:
                SendMsgCan(SLAVE_ERROR_IGRID_HIGH, error, error, error, error);
                error = 0;
                break;
            case ERROR_TEMP_HIGH:
                SendMsgCan(SLAVE_ERROR_TEMP_DCAC_HIGH, error, error, error, error);
                error = 0;
                break;
            case ERROR_FAULT_IGBT:
                SendMsgCan(SLAVE_ERROR_FAULT_IGBT, error, error, error, error);
                error = 0;
                break;
            case ERROR_CONTROLLER_VOLT:
                SendMsgCan(SLAVE_ERROR_CONTROLLER_DCAC, error, error, error, error);
                error = 0;
                break;
            case ERROR_CONTROLLER_CURR:
                SendMsgCan(SLAVE_ERROR_CONTROLLER_DCAC, error, error, error, error);
                error = 0;
                break;
            case ERROR_OVER_CURRENT:
                SendMsgCan(SLAVE_ERROR_OVER_CURRENT_DCAC, error, error, error, error);
                error = 0;
                break;
            case ERROR_PEAK_CURRENT:
                SendMsgCan(SLAVE_ERROR_PEAK_CURRENT_DCAC, error, error, error, error);
                error = 0;
                break;
            }
        }
        if(signal_mode == ST_STOP && running_state == 2)
        {
            stop_flag++;
            if (stop_flag >= 13) // 6.5 (s)
            {
                stop_flag = 0;
                if (START == 0)
                {
                    SendMsgCan(SLAVE_DCAC_OK_STOP, 0, 0, 0, 0);
                    running_state = 0;
                    signal_mode = 0;
                }
            }
        }
        send_can_loop = 0;
    }

    //-----------------
    //the next time CpuTimer1 'counter' reaches Period value go to B2
    B_Task_Ptr = &SEND_CAN;
    //-----------------
}

//=================================================================================
//  C - TASKS
//=================================================================================
//------------------------------------------------------
void C1(void)    // Spare
//------------------------------------------------------
{
    // *******************************************************
    // Current limit setting / tuning in Debug environment
    // *******************************************************

    //--------------------------------------------------------
    Limit.VLoadMax = ((float)Setting_PV.VLoadMax)/Us_max;
    Limit.VLoadMin = ((float)Setting_PV.VLoadMin)/Us_max;

    Limit.VgridMax = ((float)Setting_PV.VgridMax)/Us_max;
    Limit.VgridMin = ((float)Setting_PV.VgridMin)/Us_max;

    Limit.FrequencyMax = ((float)Setting_PV.FrequencyMax)/BASE_FREQ;
    Limit.FrequencyMin = ((float)Setting_PV.FrequencyMin)/BASE_FREQ;

    Limit.UdcMax = ((float)Setting_PV.UdcMax)/Udc_max;
    Limit.UdcMin = ((float)Setting_PV.UdcMin)/Udc_max;

//    Limit.ILoadMax = ((float)Setting_PV.ILoadMax)/Iload_max;

    Limit.IgridMax = ((float)Setting_PV.IgridMax)/Is_max;
    Limit.Ipeak = curPeakLimit/Is_max;

    //----------------------------------------------------------
    LED_BLUE_TOGGLE_MARCO
    LED_RED_TOGGLE_MARCO
    // EALLOW ; GpioDataRegs.GPDTOGGLE.bit.GPIO125= 1 ; EDIS;
    //----------------
    //the next time CpuTimer2 'counter' reaches Period value go to C2
    C_Task_Ptr = &C1;
    //-----------------
}

void SendMsgCan(uint16_t ID, uint16_t data1,uint16_t data2 ,uint16_t data3,uint16_t data4)
{
     ucTXMsgData[0] =(uint8_t) ((data1 >> 8) & 0x00FF);
     ucTXMsgData[1] =(uint8_t) ( data1 & 0x00FF);
     ucTXMsgData[2] =(uint8_t) ((data2 >> 8) & 0x00FF);
     ucTXMsgData[3] =(uint8_t) (data2 & 0x00FF);
     ucTXMsgData[4] =(uint8_t) ((data3 >> 8) & 0x00FF);
     ucTXMsgData[5] =(uint8_t) (data3 & 0x00FF);
     ucTXMsgData[6] =(uint8_t) ((data4 >> 8) & 0x00FF);
     ucTXMsgData[7] =(uint8_t) (data4 & 0x00FF);

     sTXCANMessage.ui32MsgID = ID;                      // CAN message ID
     sTXCANMessage.pucMsgData = ucTXMsgData;
     CANMessageSet(CANA_BASE, 1, &sTXCANMessage, MSG_OBJ_TYPE_TX);
}

void ReciveCan(tCANMsgObject *pMsgObject, MsgDataCan *data)
{
    data->ID = pMsgObject->ui32MsgID;

    data->WORD_ONE = *(pMsgObject->pucMsgData);
    data->WORD_ONE <<= 8;
    data->WORD_ONE += *(pMsgObject->pucMsgData + 1);

    data->WORD_SEC = *(pMsgObject->pucMsgData + 2);
    data->WORD_SEC <<= 8;
    data->WORD_SEC += *(pMsgObject->pucMsgData + 3);

    data->WORD_THREE = *(pMsgObject->pucMsgData + 4);
    data->WORD_THREE <<= 8;
    data->WORD_THREE += *(pMsgObject->pucMsgData + 5);

    data->WORD_FOUR = *(pMsgObject->pucMsgData + 6);
    data->WORD_FOUR <<= 8;
    data->WORD_FOUR += *(pMsgObject->pucMsgData + 7);
}


void TripZone_Protection(void)
{
    trip3++;
    //===========================================================================
    //Motor Control Trip Config, EPwm1,2,3
    //===========================================================================
     EALLOW;

      InputXbarRegs.INPUT1SELECT = 16;  //GIPO16
      GpioCtrlRegs.GPAINV.bit.GPIO16 = 1;

      EPwm1Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm2Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm3Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm4Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm5Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
      EPwm6Regs.TZSEL.bit.OSHT1   = 1;  //enable TZ1 for OSHT
    // What do we want the OST/CBC events to do?

      EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low

      EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low

      EPwm3Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm3Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low

      EPwm4Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm4Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low

      EPwm5Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm5Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low

      EPwm6Regs.TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
      EPwm6Regs.TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low


      // Enable TZ interrupt
//    EPwm1Regs.TZEINT.bit.OST = 1;
//    EPwm2Regs.TZEINT.bit.OST = 1;
//    EPwm3Regs.TZEINT.bit.OST = 1;

      EDIS;

      // Clear any spurious OV trip
      EPwm1Regs.TZCLR.bit.OST = 1;
      EPwm2Regs.TZCLR.bit.OST = 1;
      EPwm3Regs.TZCLR.bit.OST = 1;
      EPwm4Regs.TZCLR.bit.OST = 1;
      EPwm5Regs.TZCLR.bit.OST = 1;
      EPwm6Regs.TZCLR.bit.OST = 1;
}

__interrupt void epwm_isr(void)
{
    Pwm_Isr++;
    // Clear INT flag for this timer
    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//----------------- Protection -------------------//
void CMPSS_Protection(void)
{
    EALLOW;

#if(CMPSS_PROTECT_UDC_UPPER == 1)
    Cmpss6Regs.COMPCTL.bit.COMPDACE = 1;//
    Cmpss6Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss6Regs.COMPDACCTL.bit.DACSOURCE = 0;//
    Cmpss6Regs.COMPDACCTL.bit.SWLOADSEL = 0;//

    // Udc Upper protection
#if(PROTECT_CMPSS_MODE == NEW_PROTECT_CMPSS)
    Cmpss6Regs.DACHVALS.bit.DACVAL = (21 + ((CMPSS_Udc_New_Protecion + CMPSS_Udc_Offset_New_Protecion)/Udc_max)*(4096.0 - 21)*0.96 + 42)/1.1;
#endif

#if(PROTECT_CMPSS_MODE == OLD_PROTECT_CMPSS)
    Cmpss6Regs.DACHVALS.bit.DACVAL = (21 + (CMPSS_Udc_Upper_Protecion/Udc_max)*(4096.0 - 21)*0.96 + 42)/1.1;
#endif

    Cmpss6Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss6Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX10 = 0;   // Cmpss6 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX10  = 1;     //VDCH

    Cmpss6Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale;   // set time between samples, max : 1023
    Cmpss6Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;            // # of samples in window, max : 31
    Cmpss6Regs.CTRIPHFILCTL.bit.THRESH         = 18;            // recommended : thresh > sampwin/2
    Cmpss6Regs.CTRIPHFILCTL.bit.FILINIT        = 1;             // Init samples to filter input value
    Cmpss6Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_VaG_UPPER == 1)
    Cmpss3Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss3Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss3Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss3Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //VaG Upper protection
#if(PROTECT_CMPSS_MODE == NEW_PROTECT_CMPSS)
    Cmpss3Regs.DACHVALS.bit.DACVAL = (2145 + (((CMPSS_Udc_New_Protecion/can3)+ CMPSS_VG_Offset_New_Protecion)/Us_max)*(4096.0 - 2145)-230 + 180)/1.1;
#endif

#if(PROTECT_CMPSS_MODE == OLD_PROTECT_CMPSS)
    Cmpss3Regs.DACHVALS.bit.DACVAL = (2145 + (CMPSS_VaG_Upper_Protecion/Us_max)*(4096.0 - 2145)-230 + 180)/1.1;
#endif

    Cmpss3Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss3Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX4 = 0 ;  // Cmpss3 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX4  = 1;    //VagH

    Cmpss3Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss3Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss3Regs.CTRIPHFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss3Regs.CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss3Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_VaG_LOWER == 1)
    Cmpss3Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss3Regs.COMPCTL.bit.COMPLSOURCE = 0;
    Cmpss3Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss3Regs.COMPDACCTL.bit.SWLOADSEL = 0;

//   VaG Lower protecion
#if(PROTECT_CMPSS_MODE == NEW_PROTECT_CMPSS)
    Cmpss3Regs.DACLVALS.bit.DACVAL = (2145 - (((CMPSS_Udc_New_Protecion/can3)+ CMPSS_VG_Offset_New_Protecion)/Us_max)*(4096.0 - 2145) - 230 - 180)/1.1;
#endif

#if(PROTECT_CMPSS_MODE == OLD_PROTECT_CMPSS)
    Cmpss3Regs.DACLVALS.bit.DACVAL = (2145 - (CMPSS_VaG_Lower_Protecion/Us_max)*(4096.0 - 2145) - 230 - 180)/1.1;
#endif

    Cmpss3Regs.COMPCTL.bit.COMPLINV = 1;
    Cmpss3Regs.COMPCTL.bit.CTRIPLSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX5 = 0 ;  // Cmpss3 trip L
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX5  = 1;    //VagL

    Cmpss3Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss3Regs.CTRIPLFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss3Regs.CTRIPLFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss3Regs.CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss3Regs.COMPSTSCLR.bit.LLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_VbG_UPPER == 1)
    Cmpss1Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss1Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss1Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss1Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //VbG Upper protection
#if(PROTECT_CMPSS_MODE == NEW_PROTECT_CMPSS)
    Cmpss1Regs.DACHVALS.bit.DACVAL = (2100 + (((CMPSS_Udc_New_Protecion/can3)+ CMPSS_VG_Offset_New_Protecion)/Us_max)*(4096.0 - 2100)-190 + 183)/1.1;
#endif

#if(PROTECT_CMPSS_MODE == OLD_PROTECT_CMPSS)
    Cmpss1Regs.DACHVALS.bit.DACVAL = (2100 + (CMPSS_VbG_Upper_Protecion/Us_max)*(4096.0 - 2100)-190 + 183)/1.1;
#endif

    Cmpss1Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss1Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX0 = 0 ;  // Cmpss1 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX0  = 1;    //VbgH

    Cmpss1Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss1Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss1Regs.CTRIPHFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss1Regs.CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss1Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_VbG_LOWER == 1)
    Cmpss1Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss1Regs.COMPCTL.bit.COMPLSOURCE = 0;
    Cmpss1Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss1Regs.COMPDACCTL.bit.SWLOADSEL = 0;

//   VbG Lower protecion
#if(PROTECT_CMPSS_MODE == NEW_PROTECT_CMPSS)
    Cmpss1Regs.DACLVALS.bit.DACVAL = (2100 - (((CMPSS_Udc_New_Protecion/can3)+ CMPSS_VG_Offset_New_Protecion)/Us_max)*(4096.0 - 2100) - 190 - 183)/1.1;
#endif

#if(PROTECT_CMPSS_MODE == OLD_PROTECT_CMPSS)
    Cmpss1Regs.DACLVALS.bit.DACVAL = (2100 - (CMPSS_VbG_Lower_Protecion/Us_max)*(4096.0 - 2100) - 190 - 183)/1.1;
#endif

    Cmpss1Regs.COMPCTL.bit.COMPLINV = 1;
    Cmpss1Regs.COMPCTL.bit.CTRIPLSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX1 = 0 ;  // Cmpss1 trip L
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX1  = 1;    //VbgL

    Cmpss1Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss1Regs.CTRIPLFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss1Regs.CTRIPLFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss1Regs.CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss1Regs.COMPSTSCLR.bit.LLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_Ia_inv_UPPER == 1)
    Cmpss2Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss2Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss2Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss2Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ia Upper protection
    Cmpss2Regs.DACHVALS.bit.DACVAL = (2093 + (CMPSS_Ia_inv_Upper_Protecion/Is_max)*2093 - 230 + 169)/1.1;
    Cmpss2Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss2Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX2 = 0 ;  // Cmpss2 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX2  = 1;    //IL3H

    //High protect
    Cmpss2Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale;    // set time between samples, max : 1023
    Cmpss2Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss2Regs.CTRIPHFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss2Regs.CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss2Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_Ia_inv_LOWER == 1)
    Cmpss2Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss2Regs.COMPCTL.bit.COMPLSOURCE = 0;
    Cmpss2Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss2Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ia Lower protecion
    Cmpss2Regs.DACLVALS.bit.DACVAL = (2093 - (CMPSS_Ia_inv_Lower_Protecion/Is_max)*2093 - 230 - 169)/1.1;
    Cmpss2Regs.COMPCTL.bit.COMPLINV = 1;
    Cmpss2Regs.COMPCTL.bit.CTRIPLSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX3 = 0 ;  // Cmpss2 trip L
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX3  = 1;    //IL3L

    Cmpss2Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss2Regs.CTRIPLFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss2Regs.CTRIPLFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss2Regs.CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss2Regs.COMPSTSCLR.bit.LLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif

#if(CMPSS_PROTECT_Ib_inv_UPPER == 1)
    Cmpss4Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss4Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss4Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss4Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ib Upper protection
    Cmpss4Regs.DACHVALS.bit.DACVAL = (2105 + (CMPSS_Ib_inv_Upper_Protecion/Is_max)*2105 - 240 + 180)/1.1;
    Cmpss4Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss4Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX6 = 0 ;  // Cmpss4 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX6  = 1;    //IL1H

    //High protect
    Cmpss4Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale;    // set time between samples, max : 1023
    Cmpss4Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss4Regs.CTRIPHFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss4Regs.CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss4Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_Ib_inv_LOWER == 1)
    Cmpss4Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss4Regs.COMPCTL.bit.COMPLSOURCE = 0;
    Cmpss4Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss4Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ib Lower protecion
    Cmpss4Regs.DACLVALS.bit.DACVAL = (2105 - (CMPSS_Ib_inv_Lower_Protecion/Is_max)*2105 - 232 - 180)/1.1 ;
    Cmpss4Regs.COMPCTL.bit.COMPLINV = 1;
    Cmpss4Regs.COMPCTL.bit.CTRIPLSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX7 = 0 ;  // Cmpss4 trip L
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX7  = 1;    //IL1L

    Cmpss4Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss4Regs.CTRIPLFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss4Regs.CTRIPLFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss4Regs.CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss4Regs.COMPSTSCLR.bit.LLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif

#if(CMPSS_PROTECT_Ic_inv_UPPER == 1)
    Cmpss5Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss5Regs.COMPCTL.bit.COMPHSOURCE = 0;
    Cmpss5Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss5Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ic Upper protection
    Cmpss5Regs.DACHVALS.bit.DACVAL = (2054 + (CMPSS_Ic_inv_Upper_Protecion/Is_max)*2054 - 185 + 162)/1.1;
    Cmpss5Regs.COMPCTL.bit.COMPHINV = 0;
    Cmpss5Regs.COMPCTL.bit.CTRIPHSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX8 = 0 ;  // Cmpss5 trip H
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX8  = 1;    //IL2H

    //High protect
    Cmpss5Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = clkPrescale;    // set time between samples, max : 1023
    Cmpss5Regs.CTRIPHFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss5Regs.CTRIPHFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss5Regs.CTRIPHFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss5Regs.COMPSTSCLR.bit.HLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif
    //-----------------------------------------------------
#if(CMPSS_PROTECT_Ic_inv_LOWER == 1)
    Cmpss5Regs.COMPCTL.bit.COMPDACE = 1;
    Cmpss5Regs.COMPCTL.bit.COMPLSOURCE = 0;
    Cmpss5Regs.COMPDACCTL.bit.DACSOURCE = 0;
    Cmpss5Regs.COMPDACCTL.bit.SWLOADSEL = 0;

    //Ic Lower protecion
    Cmpss5Regs.DACLVALS.bit.DACVAL = (2054 - (CMPSS_Ic_inv_Lower_Protecion/Is_max)*2054 - 180 - 162)/1.1;
    Cmpss5Regs.COMPCTL.bit.COMPLINV = 1;
    Cmpss5Regs.COMPCTL.bit.CTRIPLSEL = 2;

    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX9 = 0 ;  // Cmpss5 trip L
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX9  = 1;    //IL2L

    Cmpss5Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = clkPrescale; // set time between samples, max : 1023
    Cmpss5Regs.CTRIPLFILCTL.bit.SAMPWIN        = 30;     // # of samples in window, max : 31
    Cmpss5Regs.CTRIPLFILCTL.bit.THRESH         = 18;      // recommended : thresh > sampwin/2
    Cmpss5Regs.CTRIPLFILCTL.bit.FILINIT        = 1;           // Init samples to filter input value
    Cmpss5Regs.COMPSTSCLR.bit.LLATCHCLR = 1;   //Clear the status register for latched comparator events
#endif

    // DC Trip select
    EPwm1Regs.DCTRIPSEL.bit.DCAHCOMPSEL =  3 ;      // Tripin4
    EPwm1Regs.TZDCSEL.bit.DCAEVT1 = 4 ;             // DCAL high , DCAH don't care
    EPwm1Regs.DCTRIPSEL.bit.DCALCOMPSEL =  3 ;      // Tripin4

    EPwm2Regs.DCTRIPSEL.bit.DCAHCOMPSEL =  3 ;      // Tripin4
    EPwm2Regs.TZDCSEL.bit.DCAEVT1 = 4 ;             // DCAL high , DCAH don't care
    EPwm2Regs.DCTRIPSEL.bit.DCALCOMPSEL =  3 ;      // Tripin4

    EPwm3Regs.DCTRIPSEL.bit.DCAHCOMPSEL =  3 ;      // Tripin4
    EPwm3Regs.TZDCSEL.bit.DCAEVT1 = 4 ;             // DCAL high , DCAH don't care
    EPwm3Regs.DCTRIPSEL.bit.DCALCOMPSEL =  3 ;      // Tripin4

    EPwm4Regs.DCTRIPSEL.bit.DCAHCOMPSEL =  3 ;      // Tripin4
    EPwm4Regs.TZDCSEL.bit.DCAEVT1 = 4 ;             // DCAL high , DCAH don't care
    EPwm4Regs.DCTRIPSEL.bit.DCALCOMPSEL =  3 ;      // Tripin4

    //Tripzone Select
    EPwm1Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm2Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm3Regs.TZSEL.bit.DCAEVT1 = 1;
    EPwm4Regs.TZSEL.bit.DCAEVT1 = 1;

    // What do we want the DCAEVT1 events to do?
    // TZA events can force EPWMxA
    // TZB events can force EPWMxB

    EPwm1Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm1Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm2Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm2Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm3Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm3Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    EPwm4Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxA will go low
    EPwm4Regs.TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low

    // Clear any spurious OV trip
    EPwm1Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm2Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm3Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm4Regs.TZCLR.bit.DCAEVT1 = 1;

    // Enable TZ interrupt
//     EPwm1Regs.TZEINT.bit.DCAEVT1 = 1;
//     EPwm2Regs.TZEINT.bit.DCAEVT1 = 1;
//     EPwm3Regs.TZEINT.bit.DCAEVT1 = 1;
//     EPwm4Regs.TZEINT.bit.DCAEVT1 = 1;
    EDIS;
}

    // ----------- Data logger ----------------//
    static inline void dlog1(Uint16 value)
    {
        DataLog1[ndx1] = value;
        if(++ndx1 == DLOG_SIZE_1)
        {
            ndx1 = 0;
        }
        //DataLog1[0] = ndx1;
    }

    static inline void dlog2(Uint16 value)
    {
        DataLog2[ndx2] = value;
        if(++ndx2 == DLOG_SIZE_2)
        {
            ndx2 = 0;
        }
        //DataLog2[0] = ndx2;
    }
    static inline void dlog3(Uint16 value)
      {
          DataLog3[ndx3] = value;
          if(++ndx3 == DLOG_SIZE_3)
          {
              ndx3 = 0;
          }
          //DataLog3[0] = ndx3;
      }
