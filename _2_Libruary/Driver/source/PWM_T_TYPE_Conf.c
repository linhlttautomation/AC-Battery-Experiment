/*
 * PWM_T_TYPE_Conf.c
 *
 *  Created on: 28 June 2022
 *      Author: Dota Do
 */

#include "PWM_T_TYPE_Conf.h"
 // 300 = 3us   // 100 counts = 1 usec (delay) * 100 count/usec (for TBCLK = SYSCLK/1)
#define dead_time 1               // deadtime: 3us
#define Periodmax 1000

//---------------------------------------------------------------------------
void PWM_3PHASE_T_TYPE(int period, int deadtime)
{
    EALLOW;
// n = the ePWM module number, i.e. selects the target module for init.
// ePWM(n) init.  Note EPWM(n) is the Master
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;          // set Immediate load
    EPwm1Regs.TBPRD = period;
    EPwm1Regs.CMPA.bit.CMPA = period;
    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    EPwm1Regs.TBCTR = 0;
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;                 // Free run


    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;        //used to sync EPWM(n+1) "down-stream"
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm1Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm1Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm1Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm1Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;          // Active Hi Complimentary
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm1Regs.DBRED.bit.DBRED = deadtime;              // dummy value for now
    EPwm1Regs.DBFED.bit.DBFED = deadtime;              // dummy value for now

    EPwm1Regs.ETSEL.bit.SOCAEN   = 1;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRD;       /* CTR = 0*/
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_2ND;            /* Generate pulse on 2nd event*/
    EPwm1Regs.ETCLR.bit.SOCA = 1;
    EPwm1Regs.ETPS.bit.SOCACNT = ET_2ND ;


    // Enable CNT_zero interrupt using EPWM1 Time-base
    EPwm1Regs.ETSEL.bit.INTEN = 1;                      // enable EPWM1INT generation
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;           // enable interrupt CNT_zero event
    EPwm1Regs.ETPS.bit.INTPRD = ET_2ND;                 // generate interrupt on the 2nd event
    EPwm1Regs.ETPS.bit.INTCNT = ET_2ND;
    EPwm1Regs.ETCLR.bit.INT = 1;                        // enable more interrupts

    //-----------------------------------------------------
// ePWM(n+1) init.  EPWM(n+1) is a slave
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm2Regs.TBPRD = period;
    EPwm2Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm2Regs.TBPHS.bit.TBPHS = 0;            // zero phase initially
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Sync "flow through" mode
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm2Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm2Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm2Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm2Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm2Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm2Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now

    //-----------------------------------------------------
// ePWM(n+2) init.  EPWM(n+2) is a slave
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm3Regs.TBPRD = period;
    EPwm3Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm3Regs.TBPHS.bit.TBPHS = period;            // zero phase initially
    EPwm3Regs.TBCTR = 0;
    EPwm3Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Sync "flow through" mode
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm3Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm3Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm3Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm3Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm3Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm3Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm3Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now
    //-----------------------------------------------------
// ePWM(n+3) init.  EPWM(n+3) is a slave
    EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm4Regs.TBPRD = period;
    EPwm4Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm4Regs.TBPHS.bit.TBPHS = 0;            // zero phase initially
    EPwm4Regs.TBCTR = 0;
    EPwm4Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Sync "flow through" mode
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm4Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm4Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm4Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm4Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm4Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm4Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm4Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now
    //-----------------------------------------------------
// ePWM(n+4) init.  EPWM(n+4) is a slave
    EPwm5Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm5Regs.TBPRD = period;
    EPwm5Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm5Regs.TBPHS.bit.TBPHS = 0;            // zero phase initially
    EPwm5Regs.TBCTR = 0;
    EPwm5Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Sync "flow through" mode
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm5Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm5Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm5Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm5Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm5Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm5Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm5Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm5Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm5Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm5Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm5Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now
    //-----------------------------------------------------
// ePWM(n+5) init.  EPWM(n+5) is a slave
    EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;        // set Immediate load
    EPwm6Regs.TBPRD = period;
    EPwm6Regs.CMPA.bit.CMPA = period;             // Fix duty at 100%
    EPwm6Regs.TBPHS.bit.TBPHS = 0;            // zero phase initially
    EPwm6Regs.TBCTR = 0;
    EPwm6Regs.TBCTL.bit.FREE_SOFT = 3;                   // Free run

    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;       // Sync "flow through" mode
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm6Regs.TBCTL.bit.PHSDIR = TB_DOWN;

    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

//    EPwm6Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//    EPwm6Regs.AQCTLA.bit.CAD = AQ_SET;
//    EPwm6Regs.AQCTLB.bit.CAU = AQ_CLEAR;
//    EPwm6Regs.AQCTLB.bit.CAD = AQ_SET;

    // activate shadow mode for DBCTL
    EPwm6Regs.DBCTL2.bit.SHDWDBCTLMODE = 0x1;
    // reload on CTR = 0
    EPwm6Regs.DBCTL2.bit.LOADDBCTLMODE = 0x0;

    EPwm6Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm6Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;        // Active Hi Complimentary
    EPwm6Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm6Regs.DBRED.bit.DBRED = deadtime;                            // dummy value for now
    EPwm6Regs.DBFED.bit.DBFED = deadtime;                            // dummy value for now

    //EDIS;
    EPwm1Regs.TBCTL.all=0xC01A;
    EPwm1Regs.TBCTR=0;
    EPwm1Regs.TBPRD= Periodmax;

    EPwm2Regs.TBCTL.all=0xC01A;
    EPwm2Regs.TBCTR=0;
    EPwm2Regs.TBPRD=Periodmax;

    EPwm3Regs.TBCTL.all=0xC01A;
    EPwm3Regs.TBCTR=0;
    EPwm3Regs.TBPRD=Periodmax;

    EPwm4Regs.TBCTL.all=0xC01A;
    EPwm4Regs.TBCTR=0;
    EPwm4Regs.TBPRD=Periodmax;
/*Counter-Compare (CC)*/                          \
    EPwm1Regs.CMPCTL.all=0x000C;
    EPwm2Regs.CMPCTL.all=0x000C;
    EPwm3Regs.CMPCTL.all=0x000C;
    EPwm4Regs.CMPCTL.all=0x000C;
/*Action-Qualifier (AQ)*/                         \
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

/*Dead Band (DB)*/                                \
   EPwm1Regs.DBCTL.all=0x03CB;
   EPwm1Regs.DBFED.bit.DBFED=dead_time;
   EPwm1Regs.DBRED.bit.DBRED=dead_time;
   EPwm2Regs.DBCTL.all=0x03CB;
   EPwm2Regs.DBFED.bit.DBFED=dead_time;
   EPwm2Regs.DBRED.bit.DBRED=dead_time;
   EPwm3Regs.DBCTL.all=0x03CB;
   EPwm3Regs.DBFED.bit.DBFED=dead_time;
   EPwm3Regs.DBRED.bit.DBRED=dead_time;
   EPwm4Regs.DBCTL.all=0x03CB;
   EPwm4Regs.DBFED.bit.DBFED=dead_time;
   EPwm4Regs.DBRED.bit.DBRED=dead_time;
/*ET*/\
/*Cho phép ngắt PWM, bắt đầu đọc ADC_ SOCA & SOCB*/ \
   //EPwm1Regs.ETSEL.all=0x0B0B;   /*0x000B*/  /*0A09*/    \
   //EPwm1Regs.ETPS.all=0x0501;   /*0x0001*/  /*0501*/    \
   /* ngats EPWM_giua_va_dau_chu_ki*/           \
EDIS;
}   // END


