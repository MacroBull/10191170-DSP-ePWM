//#############################################################################
//
//  Reference:   f2802x_examples_ccsv4/epwm_up_aq/Example_F2802xEPwmUpAQ.c
//
//!   View the EPWM1B, EPWM2A, EPWM3A, EPWM4A  waveforms
//!   via an oscilloscope:
//!      EPWM1B is on GPIO1
//!      EPWM2A is on GPIO2
//!      EPWM3A is on GPIO4 connected to GPIO3(high resistance)
//!      EPWM4A is on GPIO6 connected to GPIO0(high resistance)
//
// slow mode for debug = 10MHz * 1 / 1 / 10 / 128 / 8000= 0.9765625 Hz
// stand mode for release = 10MHz * 1 / 1 / 10 / 1 / 8000 = 1 / 8ms
//#############################################################################

#include "DSP28x_Project.h" // Device Headerfile and Examples Include File

#include "f2802x_common/include/cpu.h"
#include "f2802x_common/include/clk.h"
#include "f2802x_common/include/gpio.h"
#include "f2802x_common/include/pll.h"
#include "f2802x_common/include/pwm.h"
#include "f2802x_common/include/wdog.h"

CPU_Handle myCpu;
CLK_Handle myClk;
GPIO_Handle myGpio;
PWM_Handle myPwm1, myPwm2, myPwm3, myPwm4;

typedef struct {
	int PERIOD, ST_CNT, PHASE;
	int CMPA, CMPB;
	PWM_Number_e NUM;
} pwm_info;

const pwm_info pwmProfiles[]={
	{ 8000, 0000, 0000, 0000, 5000, PWM_Number_1 },
	{ 8000, 0000, 0000, 5000, 0000, PWM_Number_2 },
	{ 8000, 0000, 0000, 5000, 0000, PWM_Number_3 },
	{ 8000, 0000, 0000, 5000, 0000, PWM_Number_4 }
};

void initSysCtrl(){

	// Perform basic system initialization
	CLK_enableAdcClock(myClk);
	(*Device_cal)();
	CLK_disableAdcClock(myClk);

	//Select the internal oscillator 1 as the clock source
	CLK_setOscSrc(myClk, CLK_OscSrc_Internal);

	WDOG_Handle myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));
	PLL_Handle myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
	// Setup the PLL for x1 /1 which will yield 10Mhz = 10Mhz * 1 / 1
	WDOG_disable(myWDog);
	PLL_setup(myPll, PLL_Multiplier_1, PLL_DivideSelect_ClkIn_by_1);

	CPU_disableGlobalInts(myCpu);
	CPU_clearIntFlags(myCpu);
}

void initGPIO(){
	// Initalize GPIO
	GPIO_setPullUp(myGpio, GPIO_Number_1, GPIO_PullUp_Disable);
	GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_EPWM1B);

	GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);
	GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_EPWM2A);

	GPIO_setPullUp(myGpio, GPIO_Number_4, GPIO_PullUp_Disable);
	GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_EPWM3A);

	GPIO_setPullUp(myGpio, GPIO_Number_6, GPIO_PullUp_Disable);
	GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_EPWM4A);
}

void InitEPWM(PWM_Handle pwm, pwm_info pi){

	CLK_enablePwmClock(myClk, pi.NUM);
	PWM_setHighSpeedClkDiv(pwm, PWM_HspClkDiv_by_10); // Clock ratio to SYSCLKOUT
#ifdef _DEBUG
	PWM_setClkDiv(pwm, PWM_ClkDiv_by_128);
#else
	PWM_setClkDiv(pwm, PWM_ClkDiv_by_1);
#endif
	// Setup TBCLK
	PWM_setCounterMode(pwm, PWM_CounterMode_Up); // Count up
	PWM_setPeriod(pwm, pi.PERIOD); // Set timer period
	PWM_setPhase(pwm, pi.PHASE);
	PWM_setCount(pwm, pi.ST_CNT); // Set counter

	// Set Compare values
	PWM_setCmpA(pwm, pi.CMPA); // Set compare A value
	PWM_setCmpB(pwm, pi.CMPB); // Set Compare B value

	// Set actions
	PWM_setActionQual_Zero_PwmA(pwm, PWM_ActionQual_Set); // Set PWM1A on Zero
	PWM_setActionQual_CntUp_CmpA_PwmA(pwm, PWM_ActionQual_Clear); // Clear PWM1A on event A, up count
	PWM_setActionQual_Zero_PwmB(pwm, PWM_ActionQual_Set); // Set PWM1B on Zero
	PWM_setActionQual_CntUp_CmpB_PwmB(pwm, PWM_ActionQual_Clear); // Clear PWM1B on event B, up count

	// Setup shadow register load on ZERO
	PWM_setShadowMode_CmpA(pwm, PWM_ShadowMode_Shadow);
	PWM_setShadowMode_CmpB(pwm, PWM_ShadowMode_Shadow);
	PWM_setLoadMode_CmpA(pwm, PWM_LoadMode_Zero);
	PWM_setLoadMode_CmpB(pwm, PWM_LoadMode_Zero);

	// Interrupt where we will change the Compare Values
	PWM_setIntMode(pwm, PWM_IntMode_CounterEqualZero); // Select INT on Zero event
	PWM_enableInt(pwm); // Enable INT

}
void main(void)
{
	// Initialize all the handles
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));

    initSysCtrl();
    initGPIO();

    // Initialize ePWMs
    CLK_disableTbClockSync(myClk);

	myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
	myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));
	myPwm3 = PWM_init((void *)PWM_ePWM3_BASE_ADDR, sizeof(PWM_Obj));
	myPwm4 = PWM_init((void *)PWM_ePWM4_BASE_ADDR, sizeof(PWM_Obj));

    InitEPWM(myPwm1, pwmProfiles[0]);
    InitEPWM(myPwm2, pwmProfiles[1]);
    InitEPWM(myPwm3, pwmProfiles[2]);
    InitEPWM(myPwm4, pwmProfiles[3]);
    CLK_enableTbClockSync(myClk);

    CPU_enableDebugInt(myCpu);

    // Create phase shift
    PWM_setCount(myPwm2, ((PWM_Obj *)myPwm1)->TBCTR - 2000);
    PWM_setCount(myPwm3, ((PWM_Obj *)myPwm1)->TBCTR - 4000);
    PWM_setCount(myPwm4, ((PWM_Obj *)myPwm1)->TBCTR - 6000);

    for(;;) ;
}

