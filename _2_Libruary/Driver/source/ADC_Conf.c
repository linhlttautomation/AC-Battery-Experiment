#include "ADC_Conf.h"

//
// AdcSetMode - Set the resolution and signalmode for a given ADC. This will
//              ensure that the correct trim is loaded.
//
void AdcSetMode(Uint16 adc, Uint16 resolution, Uint16 signalmode)
{
    Uint16 adcOffsetTrimOTPIndex; //index into OTP table of ADC offset trims
    Uint16 adcOffsetTrim;         //temporary ADC offset trim
    //
    //re-populate INL trim
    //
    CalAdcINL(adc);

    if(0xFFFF != *((Uint16*)GetAdcOffsetTrimOTP))
    {
        //
        //offset trim function is programmed into OTP, so call it
        //

        //
        //calculate the index into OTP table of offset trims and call
        //function to return the correct offset trim
        //
        adcOffsetTrimOTPIndex = 4*adc + 2*resolution + 1*signalmode;
        adcOffsetTrim = (*GetAdcOffsetTrimOTP)(adcOffsetTrimOTPIndex);
    }
    else
    {
        //
        //offset trim function is not populated, so set offset trim to 0
        //
        adcOffsetTrim = 0;
    }

    //
    //Apply the resolution and signalmode to the specified ADC.
    //Also apply the offset trim and, if needed, linearity trim correction.
    //
    switch(adc)
    {
        case ADC_ADCA:
            AdcaRegs.ADCCTL2.bit.RESOLUTION = resolution;
            AdcaRegs.ADCCTL2.bit.SIGNALMODE = signalmode;
            AdcaRegs.ADCOFFTRIM.all = adcOffsetTrim;
            if(ADC_RESOLUTION_12BIT == resolution)
            {
                //
                //12-bit linearity trim workaround
                //
                AdcaRegs.ADCINLTRIM1 &= 0xFFFF0000;
                AdcaRegs.ADCINLTRIM2 &= 0xFFFF0000;
                AdcaRegs.ADCINLTRIM4 &= 0xFFFF0000;
                AdcaRegs.ADCINLTRIM5 &= 0xFFFF0000;
            }
        break;
        case ADC_ADCB:
            AdcbRegs.ADCCTL2.bit.RESOLUTION = resolution;
            AdcbRegs.ADCCTL2.bit.SIGNALMODE = signalmode;
            AdcbRegs.ADCOFFTRIM.all = adcOffsetTrim;
            if(ADC_RESOLUTION_12BIT == resolution)
            {
                //
                //12-bit linearity trim workaround
                //
                AdcbRegs.ADCINLTRIM1 &= 0xFFFF0000;
                AdcbRegs.ADCINLTRIM2 &= 0xFFFF0000;
                AdcbRegs.ADCINLTRIM4 &= 0xFFFF0000;
                AdcbRegs.ADCINLTRIM5 &= 0xFFFF0000;
            }
        break;
        case ADC_ADCC:
            AdccRegs.ADCCTL2.bit.RESOLUTION = resolution;
            AdccRegs.ADCCTL2.bit.SIGNALMODE = signalmode;
            AdccRegs.ADCOFFTRIM.all = adcOffsetTrim;
            if(ADC_RESOLUTION_12BIT == resolution)
            {
                //
                //12-bit linearity trim workaround
                //
                AdccRegs.ADCINLTRIM1 &= 0xFFFF0000;
                AdccRegs.ADCINLTRIM2 &= 0xFFFF0000;
                AdccRegs.ADCINLTRIM4 &= 0xFFFF0000;
                AdccRegs.ADCINLTRIM5 &= 0xFFFF0000;
            }
        break;
        case ADC_ADCD:
            AdcdRegs.ADCCTL2.bit.RESOLUTION = resolution;
            AdcdRegs.ADCCTL2.bit.SIGNALMODE = signalmode;
            AdcdRegs.ADCOFFTRIM.all = adcOffsetTrim;
            if(ADC_RESOLUTION_12BIT == resolution)
            {
                //
                //12-bit linearity trim workaround
                //
                AdcdRegs.ADCINLTRIM1 &= 0xFFFF0000;
                AdcdRegs.ADCINLTRIM2 &= 0xFFFF0000;
                AdcdRegs.ADCINLTRIM4 &= 0xFFFF0000;
                AdcdRegs.ADCINLTRIM5 &= 0xFFFF0000;
            }
        break;
    }
}
//
// CalAdcINL - Loads INL trim values from OTP into the trim registers of the
//             specified ADC. Use only as part of AdcSetMode function, since
//             linearity trim correction is needed for some modes.
//
void CalAdcINL(Uint16 adc)
{
    switch(adc)
    {
        case ADC_ADCA:
            if(0xFFFF != *((Uint16*)CalAdcaINL))
            {
                //
                //trim function is programmed into OTP, so call it
                //
                (*CalAdcaINL)();
            }
            else
            {
                //
                //do nothing, no INL trim function populated
                //
            }
            break;
        case ADC_ADCB:
            if(0xFFFF != *((Uint16*)CalAdcbINL))
            {
                //
                //trim function is programmed into OTP, so call it
                //
                (*CalAdcbINL)();
            }
            else
            {
                //
                //do nothing, no INL trim function populated
                //
            }
            break;
        case ADC_ADCC:
            if(0xFFFF != *((Uint16*)CalAdccINL))
            {
                //
                //trim function is programmed into OTP, so call it
                //
                (*CalAdccINL)();
            }
            else
            {
                //
                //do nothing, no INL trim function populated
                //
            }
            break;
        case ADC_ADCD:
            if(0xFFFF != *((Uint16*)CalAdcdINL))
            {
                //
                //trim function is programmed into OTP, so call it
                //
                (*CalAdcdINL)();
            }
            else
            {
                //
                //do nothing, no INL trim function populated
                //
            }
            break;
    }
}

//
// End of file
//


// Init Moudle ADC A
void Init_ADC_A()
{
       Uint16 i;

        EALLOW;

        //
        //write configurations
        //
        AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

        //
        //Set pulse positions to late
        //
        AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

        //
        //power up the ADC
        //
        AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

        //
        //delay for > 1ms to allow ADC time to power up
        //
        for(i = 0; i < 1000; i++)
        {
            asm("   RPT#255 || NOP");
        }
        EDIS;

        EALLOW;
        //
        //Select the channels to convert and end of conversion flag ADCA
        //

        // Ic_inv
        AdcaRegs.ADCSOC0CTL.bit.CHSEL = 4;          //SOC0 will convert pin A4 -> Ic_inv
        AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

        // In_inv
        AdcaRegs.ADCSOC3CTL.bit.CHSEL = 5;          //SOC1 will convert pin A5 -> In_inv
        AdcaRegs.ADCSOC3CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

        // Vb_G
        AdcaRegs.ADCSOC1CTL.bit.CHSEL = 2;          //SOC2 will convert pin A2 -> Vb_G
        AdcaRegs.ADCSOC1CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

        // Vc_G
        AdcaRegs.ADCSOC2CTL.bit.CHSEL = 0;          //SOC3 will convert pin A0 -> Vc_G
        AdcaRegs.ADCSOC2CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

        // Trigger CLA
        AdcaRegs.ADCINTSOCSEL1.all=0x0000;          // No ADCInterrupt will trigger SOCx
        AdcaRegs.ADCINTSOCSEL2.all=0x0000;
        AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 3;      // EOC3 is trigger for ADCINT1
        AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        // enable ADC interrupt 1
        AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 1;     // ADCINT1 pulses are generated whenever an EOC pulse is generated irrespective of whether the flag bit is cleared or not.
                                                    // 0 No further ADCINT2 pulses are generated until ADCINT2 flag (in ADCINTFLG register) is cleared by user.
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      //make sure INT1 flag is cleared
        EDIS;


}

// Init ADC B
void Init_ADC_B()
{
       Uint16 i;

        EALLOW;

        //
        //write configurations
        //
        AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

        //
        //Set pulse positions to late
        //
        AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

        //
        //power up the ADC
        //
        AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

        //
        //delay for > 1ms to allow ADC time to power up
        //
        for(i = 0; i < 1000; i++)
        {
            asm("   RPT#255 || NOP");
        }
        EDIS;

        EALLOW;

        // Ia_inv
        AdcbRegs.ADCSOC0CTL.bit.CHSEL = 5;          //SOC3 will convert pin B5 -> Ib_L
        AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C

        // Va_G
        AdcbRegs.ADCSOC1CTL.bit.CHSEL = 2;          //SOC0 will convert pin B2 -> Va_G
        AdcbRegs.ADCSOC1CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C



        EDIS;
}

// Init ADC C
void Init_ADC_C()
{
       Uint16 i;

        EALLOW;

        //
        //write configurations
        //
        AdccRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
        AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

        //
        //Set pulse positions to late
        //
        AdccRegs.ADCCTL1.bit.INTPULSEPOS = 0;

        //
        //power up the ADC
        //
        AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;

        //
        //delay for > 1ms to allow ADC time to power up
        //
        for(i = 0; i < 1000; i++)
        {
            asm("   RPT#255 || NOP");
        }

        EDIS;

        //
        //Select the channels to convert and end of conversion flag ADCA
        //
        EALLOW;
        //U_dc
        AdccRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin C2 -> U_dc
        AdccRegs.ADCSOC0CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOC

        //Ib_inv
        AdccRegs.ADCSOC1CTL.bit.CHSEL = 5;          //SOC1 will convert pin C2 -> Ib_inv
        AdccRegs.ADCSOC1CTL.bit.ACQPS = 19;         //sample window is 20 SYSCLK cycles
        AdccRegs.ADCSOC1CTL.bit.TRIGSEL = 0x05;     //trigger on ePWM1 SOCA/C



}
