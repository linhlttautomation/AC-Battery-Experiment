/*
 * Init_GPIO.c
 *
 *  Created on: 27 Nov 2019
 *      Author: Dinh Ngoc
 */


#include "Init_GPIO.h"
#include "delay.h"
#include "clb.h"
#include "clb_config.h"

void GPIO_Init(void)
{

    EALLOW;
    //  GPIO-16 - PIN FUNCTION = FAULT
    GpioCtrlRegs.GPAGMUX2.bit.GPIO16 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0;    // 0=GPIO,
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;     // Enable pull-up

    //  GPIO-31 - PIN FUNCTION = LED_BLUE
    GpioCtrlRegs.GPAGMUX2.bit.GPIO31 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;    // 0=GPIO,
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;     // 1=OUTput,  0=INput
    GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;     // Enable pull-up

    //  GPIO-34 - PIN FUNCTION = LED RED
    GpioCtrlRegs.GPBGMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;    // 0=GPIO,
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;     // 1=OUTput,  0=INput
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;     // Enable pull-up

    //  GPIO-56 - PIN FUNCTION = ENABLE_LOAD
    GpioCtrlRegs.GPBGMUX2.bit.GPIO56 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 0;    // 0=GPIO,
    GpioCtrlRegs.GPBDIR.bit.GPIO56 = 1;     // 1=OUTput
    GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0;     // Enable pull-up

    //  GPIO-67 - PIN FUNCTION = ENABLE_PWM
    GpioCtrlRegs.GPCGMUX1.bit.GPIO67 = 0;
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;      /*output  */
    GpioCtrlRegs.GPCPUD.bit.GPIO67 = 0;      // Enable pull-up

    //  GPIO-95 - PIN FUNCTION = CHARGE_CAP
    GpioCtrlRegs.GPCGMUX2.bit.GPIO95 = 0;
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;   // 0=GPIO,
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 1;    // 1=OUTput,  0=INput
    GpioCtrlRegs.GPCPUD.bit.GPIO95 = 0;    // Enable pull-up

    //  GPIO-104 - PIN FUNCTION = RESET_V
     GpioCtrlRegs.GPDGMUX1.bit.GPIO104 = 0;
     GpioCtrlRegs.GPDMUX1.bit.GPIO104 = 0;   // 0=GPIO,
     GpioCtrlRegs.GPDDIR.bit.GPIO104 = 1;    // 1=OUTput,  0=INput
     GpioCtrlRegs.GPDPUD.bit.GPIO104 = 0;    // Enable pull-up


     //  GPIO-105 - PIN FUNCTION = RESET_U
     GpioCtrlRegs.GPDGMUX1.bit.GPIO105 = 0;
     GpioCtrlRegs.GPDMUX1.bit.GPIO105 = 0;   // 0=GPIO,
     GpioCtrlRegs.GPDDIR.bit.GPIO105 = 1;    // 1=OUTput,  0=INput
     GpioCtrlRegs.GPDPUD.bit.GPIO105 = 0;    // Enable pull-up

     //  GPIO-139 - PIN FUNCTION = ENABLE_GRID
     GpioCtrlRegs.GPEGMUX1.bit.GPIO139 = 0;   //
     GpioCtrlRegs.GPEMUX1.bit.GPIO139 = 0;    // 0=GPIO,
     GpioCtrlRegs.GPEDIR.bit.GPIO139 = 1;     // 1=OUTput,  0=INput
     GpioCtrlRegs.GPEPUD.bit.GPIO139 = 0;     // Enable pull-up

    EDIS;
    LED_BLUE_ON_MARCO;
    LED_RED_ON_MARCO;

    CHARGE_CAP_OFF_MARCO;

    ENABLE_GRID_OFF_MARCO;
    ENABLE_LOAD_OFF_MARCO;
    DISABLE_PWM_MARCO;

    RESET_U_ON_MARCO;
    RESET_V_ON_MARCO;
    RESET_W_ON_MARCO;

    DelayMs(100);
    RESET_U_OFF_MARCO;
    RESET_V_OFF_MARCO;
    RESET_W_OFF_MARCO;

    DelayMs(100);
    RESET_U_ON_MARCO;
    RESET_V_ON_MARCO;
    RESET_W_ON_MARCO;

}

void ResetIGBT(void)
{

    RESET_U_ON_MARCO;
    RESET_V_ON_MARCO;
    RESET_W_ON_MARCO;

    DelayMs(100);
    RESET_U_OFF_MARCO;
    RESET_V_OFF_MARCO;
    RESET_W_OFF_MARCO;

    DelayMs(100);
    RESET_U_ON_MARCO;
    RESET_V_ON_MARCO;
    RESET_W_ON_MARCO;

    EALLOW; // Clear flag TripZone (Cycle-By-Cycle Flag Clear)
    EPwm1Regs.TZCLR.bit.OST = 1;
    EPwm2Regs.TZCLR.bit.OST = 1;
    EPwm3Regs.TZCLR.bit.OST = 1;
    EPwm4Regs.TZCLR.bit.OST = 1;
    EPwm5Regs.TZCLR.bit.OST = 1;
    EPwm6Regs.TZCLR.bit.OST = 1;
    EDIS;
}

void Setup_CLB(void)
{
    CLB_enableCLB(CLB4_BASE);
    initTILE1(CLB4_BASE);

    //
    // Select Global input instead of local input for all CLB IN
    //
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN0, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN1, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN2, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN3, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN4, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN5, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN6, CLB_LOCAL_IN_MUX_GLOBAL_IN);
    CLB_configLocalInputMux(CLB4_BASE, CLB_IN7, CLB_LOCAL_IN_MUX_GLOBAL_IN);

    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN0, CLB_GLOBAL_IN_MUX_EPWM1A_AQ);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN1, CLB_GLOBAL_IN_MUX_EPWM2A_AQ);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN2, CLB_GLOBAL_IN_MUX_EPWM3A_AQ);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN3, CLB_GLOBAL_IN_MUX_EPWM1_CTRDIR);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN4, CLB_GLOBAL_IN_MUX_EPWM1_CTRDIR);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN5, CLB_GLOBAL_IN_MUX_EPWM1_CTRDIR);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN6, CLB_GLOBAL_IN_MUX_EPWM1_CTRDIR);
    CLB_configGlobalInputMux(CLB4_BASE, CLB_IN7, CLB_GLOBAL_IN_MUX_EPWM1_CTRDIR);

    //
    // Inputs set to GP register
    //
    CLB_configGPInputMux(CLB4_BASE, CLB_IN0, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN1, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN2, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN3, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN4, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN5, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN6, CLB_GP_IN_MUX_EXTERNAL);
    CLB_configGPInputMux(CLB4_BASE, CLB_IN7, CLB_GP_IN_MUX_EXTERNAL);

    CLB_setOutputMask(CLB4_BASE, 0x10, true);
}
