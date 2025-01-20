/*
 * PWM_3PHASE_Conf.c
 *
 *  Created on: Oct 13, 2022
 *      Author: Nghialq
 */

#include "PWM_3PHASE_Conf.h"

void PWM_3PHASE_CONF(int period, int deadtime)
{
    EALLOW;

    /* Time-Base Control (TBCTL) */
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;
    EPwm1Regs.TBCTL.bit.PHSDIR = TB_DOWN;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.SWFSYNC = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

    EPwm2Regs.TBCTL.bit.FREE_SOFT = 3;
    EPwm2Regs.TBCTL.bit.PHSDIR = TB_DOWN;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.SWFSYNC = 0;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

    EPwm3Regs.TBCTL.bit.FREE_SOFT = 3;
    EPwm3Regs.TBCTL.bit.PHSDIR = TB_DOWN;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.SWFSYNC = 0;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm3Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

    EPwm4Regs.TBCTL.bit.FREE_SOFT = 3;
    EPwm4Regs.TBCTL.bit.PHSDIR = TB_DOWN;
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm4Regs.TBCTL.bit.SWFSYNC = 0;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

    /* Initialization */
    EPwm1Regs.CMPA.bit.CMPA = period;
    EPwm1Regs.TBPHS.bit.TBPHS = 0;
    EPwm1Regs.TBCTR = 0;
    EPwm1Regs.TBPRD = period;

    EPwm2Regs.CMPA.bit.CMPA = period;
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.TBPRD = period;

    EPwm3Regs.CMPA.bit.CMPA = period;
    EPwm3Regs.TBPHS.bit.TBPHS = 0;
    EPwm3Regs.TBCTR = 0;
    EPwm3Regs.TBPRD = period;

    EPwm4Regs.CMPA.bit.CMPA = period;
    EPwm4Regs.TBPHS.bit.TBPHS = 0;
    EPwm4Regs.TBCTR = 0;
    EPwm4Regs.TBPRD = period;

    /* Counter-Compare (CC) */
    EPwm1Regs.CMPCTL.all = 0x000C;
    EPwm2Regs.CMPCTL.all = 0x000C;
    EPwm3Regs.CMPCTL.all = 0x000C;
    EPwm4Regs.CMPCTL.all = 0x000C;

    /* Action-Qualifier (AQ) */
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;

    /* Dead Band (DB) */
    EPwm1Regs.DBCTL.all = 0x03CB;
    EPwm1Regs.DBFED.bit.DBFED = deadtime;
    EPwm1Regs.DBRED.bit.DBRED = deadtime;

    EPwm2Regs.DBCTL.all = 0x03CB;
    EPwm2Regs.DBFED.bit.DBFED = deadtime;
    EPwm2Regs.DBRED.bit.DBRED = deadtime;

    EPwm3Regs.DBCTL.all = 0x03CB;
    EPwm3Regs.DBFED.bit.DBFED = deadtime;
    EPwm3Regs.DBRED.bit.DBRED = deadtime;

    EPwm4Regs.DBCTL.all = 0x03CB;
    EPwm4Regs.DBFED.bit.DBFED = deadtime;
    EPwm4Regs.DBRED.bit.DBRED = deadtime;

    /* Event Trigger (ET) */
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;           // CTR = 0
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_2ND;                // Generate pulse on 2nd event
    EPwm1Regs.ETCLR.bit.SOCA = 1;
    EPwm1Regs.ETPS.bit.SOCACNT = ET_2ND;

    // Enable CNT_zero interrupt using EPWM1 Time-base
    EPwm1Regs.ETSEL.bit.INTEN = 1;                      // enable EPWM1INT generation
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_PRDZERO;        // enable interrupt CNT_zero event
    EPwm1Regs.ETPS.bit.INTPRD = ET_2ND;                 // generate interrupt on the 2nd event
    EPwm1Regs.ETPS.bit.INTCNT = ET_2ND;
    EPwm1Regs.ETCLR.bit.INT = 1;                        // enable more interrupts

    EDIS;
}
