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
