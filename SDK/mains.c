#include <stdio.h>
#include <stdint.h>
#include "xil_io.h"
#include "sleep.h"
#include "xparameters.h"
#include "xadcps.h"




#define PWM_BASE_ADDR  XPAR_MYIP_SONAR_0_S00_AXI_BASEADDR
#define XADC_DEVICE_ID       XPAR_XADCPS_0_DEVICE_ID

#define PERIOD_OFFSET       0x00
#define DUTY_CYCLE_OFFSET   0x04
#define PHASE_OFFSET        0x08
#define TRIGGER_OFFSET      0x0C

#define SYSTEM_CLOCK         100000000UL   // 100 MHz
#define PWM_FREQUENCY        40000UL       // kHz
#define DUTY_CYCLE_PERCENT   50            // %
#define PHASE_SHIFT_DEGREES  20            // ° phase shift
#define TRIGGER_VALUE        0x00000001

#define SAMPLES 3200


u16 buff[SAMPLES];

XAdcPs_Config *ConfigPtr;
static XAdcPs XAdcInst;
XAdcPs* XAdcInstPtr = &XAdcInst;

uint32_t calculate_period(uint32_t system_clock, uint32_t pwm_frequency) {
    return system_clock / pwm_frequency;
}

uint32_t calculate_duty_cycle(uint32_t period, uint32_t duty_cycle_percent) {
    return (period * duty_cycle_percent) / 100;
}

uint32_t calculate_phase(uint32_t period, uint32_t phase_degrees) {
    return (period * phase_degrees + 180) / 360;
}

int main(void) {
    uint32_t period_val, duty_val, phase_val;



    ConfigPtr = XAdcPs_LookupConfig(XADC_DEVICE_ID);
    XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

    XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SINGCHAN);

    XAdcPs_SetSeqChEnables(XAdcInstPtr, (1 << XADCPS_CH_VPVN));


    period_val = calculate_period(SYSTEM_CLOCK, PWM_FREQUENCY);
    duty_val   = calculate_duty_cycle(period_val, DUTY_CYCLE_PERCENT);
    phase_val  = calculate_phase(period_val, PHASE_SHIFT_DEGREES);


    Xil_Out32(PWM_BASE_ADDR + PERIOD_OFFSET, period_val);
    Xil_Out32(PWM_BASE_ADDR + DUTY_CYCLE_OFFSET, duty_val);
    Xil_Out32(PWM_BASE_ADDR + PHASE_OFFSET, phase_val);

    while(1) {

        Xil_Out32(PWM_BASE_ADDR + TRIGGER_OFFSET, TRIGGER_VALUE);
        usleep(50);
        Xil_Out32(PWM_BASE_ADDR + TRIGGER_OFFSET, 0x00000000);
        usleep(325);


        for (int i = 0; i < SAMPLES; i++)
        {
        volatile u16 adc_data = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VPVN);
        buff[i] = adc_data;
        }

        for (int j = 0; j < SAMPLES; j++)
        {
        printf("%04X", buff[j]);
        }
        printf("\n");

        //usleep(100000);
        }

    return 0;
}
