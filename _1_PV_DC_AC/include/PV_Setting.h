
#ifndef _PV_SETTING_H_
#define _PV_SETTING_H_

/*------------------------------------------------------------------------------
Following is the list of the Build Level choices.
------------------------------------------------------------------------------*/
#define LEVEL1  1      		//   Open Loop
#define LEVEL2  2           //   Open Loop Third Harmonic Injection
#define LEVEL3  3           //   Stand Alone Current Loop
#define LEVEL4  4           //   Stand Alone Voltage Loop
#define LEVEL5  5           //   Grid Connected Current Loop
#define LEVEL6  6			//   Grid Connected Voltage Loop
#define LEVEL7  7			//
#define LEVELALL 8			//

/* This line sets the BUILDLEVEL to one of the available choices.
------------------------------------------------------------------------------*/
#define   BUILDLEVEL LEVEL5

/*------------------------------------------------------------------------------
Following is the list of the Modulation method choices.
------------------------------------------------------------------------------*/
#define SVM3D   1
#define RSPWM   2
#define DPWM    3

/* This line sets the MODULATION method to one of the available choices
------------------------------------------------------------------------------*/
#define   MODULATION SVM3D

#define   THYPHON_HIL 0

/*Che do thay doi bo dieu khien
------------------------------------------------------------------------------*/

#define     NOT_ALLOW_SWITCH_BDK      0
#define     ALLOW_SWITCH_BDK          1

#define     SWITCH_BDK_MODE    NOT_ALLOW_SWITCH_BDK

#define PI 3.14159265358979
#define can2 1.414213562
#define can3 1.732050808
#define can6 2.449489743

// Define the base quantities for PU system conversion
#define NORMAL_FREQ     50.0
#define BASE_FREQ      	150.0               // Base electrical frequency (Hz)
#define Udc_max			800.0		  	    // Max DC Voltage (V)
#define Us_max			400.0			    // Max Phase Voltage (V)
#define Is_max    		81.30               // Base Peak Phase Current (A)

#define Li              0.00033
#define Wref			(2.0*PI*NORMAL_FREQ)
#define Wmax			(2.0*PI*BASE_FREQ)

/*************khoi tao he thong ***************/

#define SYSTEM_FREQUENCY 200

#define T_Udc            0.0002              // time sample voltage
#define T_Us             0.002              // time sample voltage
#define Ti               0.00002             // time sample current 0.0000154
#define T          		 0.00002             // time sample
#define ISR_FREQUENCY    25000              // frequency isr
#define SVM_SYSTEM       50000              // frequency switching
#define SVM_SYSTEM_10K   10000
#define FREQUENCY_CLOCK_PWM	100000000

#define Time_interrupt   100.0
#define NUMBER_SAMPLE    100.0

/*------------------------------------------------------------------------------
Current sensors scaling
------------------------------------------------------------------------------*/
// LEM    1.0pu current ==> 50.0A -> 2048 counts
#define LEM(A)     (2048.0*A/Is_max)

//------------------------------------------------------------------
// ADC
#define ADC_VREFHI 3.0

#define ADC_SCALE_MEASURE_VOLTAGE_DC 1000.0/3.0
#define ADC_SCALE_MEASURE_VOLTAGE_AC 1200.0/3.0
#define ADC_SCALE_MEASURE_CURRENT_AC 100.0/3.0    // 25A max

#define ADC_PU_SCALE_FACTOR_16BIT        0.0000152587890625     	//1/2^12=0.000244140625
#define ADC_PU_SCALE_FACTOR_12BIT        0.000244140625     	    //1/2^12=0.000244140625
#define ADC_PU_PPB_SCALE_FACTOR    		 0.000488281250     		//1/2^11
#define SD_PU_SCALE_FACTOR         		 0.000030517578125  		// 1/2^15

// ADC Related defines
#if (THYPHON_HIL == 1)
#define UDC_HCPL  AdcaResultRegs.ADCRESULT0
#define UDC1_HCPL AdcbResultRegs.ADCRESULT0
#define UDC2_HCPL AdccResultRegs.ADCRESULT0
#define UDC1_HCPL_PPB ((signed int)AdcaResultRegs.ADCPPB1RESULT.all)
#define UDC2_HCPL_PPB ((signed int)AdcbResultRegs.ADCPPB1RESULT.all)
#define UDC_HCPL_PPB ((signed int)AdccResultRegs.ADCPPB2RESULT.all)

#else

#define UDC_HCPL        AdccResultRegs.ADCRESULT0    //ADCIN C2 SOC0

#define VaG_HCPL        AdcbResultRegs.ADCRESULT1    //ADCIN B2 SOC1
#define VbG_HCPL        AdcaResultRegs.ADCRESULT1    //ADCIN A2 SOC4
#define VcG_HCPL        AdcaResultRegs.ADCRESULT2    //ADCIN A0 SOC3

#define IA_INV_LEM      AdcbResultRegs.ADCRESULT0    //ADCIN B5 SOC0
#define IB_INV_LEM      AdccResultRegs.ADCRESULT1    //ADCIN C5 SOC1
#define IC_INV_LEM      AdcaResultRegs.ADCRESULT0    //ADCIN A4 SOC0
#define IZ_INV_LEM      AdcaResultRegs.ADCRESULT3    //ADCIN A5 SOC1

#endif

#define DUTY_SET                0.032

#define KP_PLL                  4.0
#define KI_PLL                  100.0

#if (BUILDLEVEL == LEVEL3)

//#define KP_CURR_LOOP            3.3          // L filter 330uH: 3.3
//#define KI_CURR_LOOP            132000.0     // L filter 330uH: 132000.0

#define KP_CURR_LOOP            3.0          // L filter 330uH: 3.3
#define KI_CURR_LOOP            1500.0     // L filter 330uH: 132000.0

#define KP_CURR_LOOP_Z          3.0         // L filter 330uH: 13.2
#define KI_CURR_LOOP_Z          1500.0     // L filter 330uH: 528000.0

#endif

#if (BUILDLEVEL == LEVEL4)

//#define KP_CURR_LOOP            0.1
//#define KI_CURR_LOOP            10.0

#define KP_CURR_LOOP            3.0          // L filter 330uH: 3.3 (thong so cu)
#define KI_CURR_LOOP            1500.0

#define KP_CURR_LOOP_1            2.0          // Thong so moi vong ap khong tai
#define KI_CURR_LOOP_1            0.18

//#define KP_CURR_LOOP_1            1.0          // Thong so moi vong ap khong tai
//#define KI_CURR_LOOP_1            0.3

//#define KP_CURR_LOOP            0.01          // L filter 330uH: 3.3
//#define KI_CURR_LOOP            0.1

#define KP_VOLT_US_LOOP         0.00001
#define KI_VOLT_US_LOOP         0.1

//#define KP_VOLT_US_LOOP         0.01
//#define KI_VOLT_US_LOOP         0.1

#endif

#if (BUILDLEVEL == LEVEL5)

//#define KP_CURR_LOOP            1.15       // L filter 1.7mH: 17.0   --- L Filter 115uH: 1.15
//#define KI_CURR_LOOP            800.0      // L filter 1.7mH: 800.0  --- L Filter 115uH: 800.0
//
#define KP_CURR_LOOP_Z          3.0       // L filter 1.7mH: 68.0   --- L Filter 115uH: 4.60
#define KI_CURR_LOOP_Z          1500.0     // L filter 1.7mH: 3200.0 --- L Filter 115uH: 3200.0

#define KP_CURR_LOOP            3.0       // L filter 1.7mH: 17.0   --- L Filter 115uH: 1.15
#define KI_CURR_LOOP            1500.0      // L filter 1.7mH: 800.0  --- L Filter 115uH: 800.0

//#define KP_CURR_LOOP_Z          4.60       // L filter 1.7mH: 68.0   --- L Filter 115uH: 4.60
//#define KI_CURR_LOOP_Z          3200.0     // L filter 1.7mH: 3200.0 --- L Filter 115uH: 3200.0

#endif

#if (BUILDLEVEL == LEVEL6)

#define KP_CURR_LOOP            3.0
#define KI_CURR_LOOP            1500.0

#define KP_CURR_LOOP_Z          3.0
#define KI_CURR_LOOP_Z          1500.0

#define KP_VOLT_UDC_LOOP        0.1
#define KI_VOLT_UDC_LOOP        5.0

#endif

#if (BUILDLEVEL == LEVELALL)

#define KP_CURR_LOOP_STAND_ALONE         2.4600000
#define KI_CURR_LOOP_STAND_ALONE         2000.0

#define KP_CURR_LOOP_GRID_CONNECTED       4.2
#define KI_CURR_LOOP_GRID_CONNECTED       400.4759

#define KP_VOLT_UDC_LOOP        0.3       // 0.4459
#define KI_VOLT_UDC_LOOP        50.0

#define KP_VOLT_US_LOOP         0.0142
#define KI_VOLT_US_LOOP         5.0

#endif

//-----------------------------------------------------------------------------
// CONTROL MODE
#define OPEN_LOOP_MODE          0
#define GRID_CONNECTED_MODE     1
#define STAND_ALONE_MODE        2
#define HYBRID_MODE             3

//-----------------------------------------------------------------------------
// SETTING PV DC AC 3 PHASE

#define PV_POWER                  11000
#define PV_VOLTAGE                220
#define PV_CURRENT                17
#define PV_FREQUENCY              50
#define PV_COSPHI                 0.9
#define PV_UDC                    650

#define PV_FREQUENCY_MAX          70
#define PV_FREQUENCY_MIN          0  //30

#define PV_UDC_MAX                1000
#define PV_UDC_MIN                0 //50

#define PV_VGRID_MAX              300
#define PV_VGRID_MIN              0 //20

#define PV_IGRID_MAX              50 //20

#define PV_VLOAD_MAX              300
#define PV_VLOAD_MIN              0
#define PV_ILOAD_MAX              20

#define PV_TEMP_MAX               100

/*-------------------------------------------------------------------------
// --------------------- SETTING PROTECION -------------------------------//
--------------------------------------------------------------------------- */
#define CMPSS_PROTECT_UDC_UPPER         1
#define CMPSS_PROTECT_VaG_UPPER         1
#define CMPSS_PROTECT_VaG_LOWER         1
#define CMPSS_PROTECT_VbG_UPPER         1
#define CMPSS_PROTECT_VbG_LOWER         1
#define CMPSS_PROTECT_Ia_inv_UPPER      1
#define CMPSS_PROTECT_Ia_inv_LOWER      1
#define CMPSS_PROTECT_Ib_inv_UPPER      1
#define CMPSS_PROTECT_Ib_inv_LOWER      1
#define CMPSS_PROTECT_Ic_inv_UPPER      1
#define CMPSS_PROTECT_Ic_inv_LOWER      1


//Che do bao ve moi hay cu
#define     OLD_PROTECT_CMPSS     0
#define     NEW_PROTECT_CMPSS     1

#define CMPSS_Udc_New_Protecion            300.0
#define CMPSS_Udc_Offset_New_Protecion     100.0
#define CMPSS_VG_Offset_New_Protecion      80.0

#define     PROTECT_CMPSS_MODE      NEW_PROTECT_CMPSS

// CMPSS (absolute values)
#define CMPSS_Udc_Upper_Protecion      150.0
#define CMPSS_VaG_Upper_Protecion      100.0
#define CMPSS_VaG_Lower_Protecion      100.0
#define CMPSS_VbG_Upper_Protecion      100.0
#define CMPSS_VbG_Lower_Protecion      200.0
#define CMPSS_Ia_inv_Upper_Protecion   15.0
#define CMPSS_Ia_inv_Lower_Protecion   15.0
#define CMPSS_Ib_inv_Upper_Protecion   15.0
#define CMPSS_Ib_inv_Lower_Protecion   15.0
#define CMPSS_Ic_inv_Upper_Protecion   15.0
#define CMPSS_Ic_inv_Lower_Protecion   15.0

//------------------------------------------------------------------------------
#define DO_HIGH     { EALLOW ; GpioDataRegs.GPBSET.bit.GPIO32 =1 ; EDIS;	}
#define DO_LOW		{ EALLOW ; GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1 ; EDIS;	}
#define DO_TOGGLE	{ EALLOW ; GpioDataRegs.GPBTOGGLE.bit.GPIO32= 1 ; EDIS;	}

#endif
