#include <stdio.h>
#include <stdint.h>
#include "xil_io.h"
#include "sleep.h"
#include "xparameters.h"
#include "xadcps.h"
#include "xgpiops.h"



#define PWM_BASE_ADDR 		 XPAR_MYIP_SODARCP_0_S00_AXI_BASEADDR
#define XADC_DEVICE_ID       XPAR_XADCPS_0_DEVICE_ID

#define PERIOD_OFFSET       0x00
#define DUTY_CYCLE_OFFSET   0x04
#define PHASE_OFFSET        0x08

#define SYSTEM_CLOCK         100000000UL   // 100 MHz
#define PWM_FREQUENCY        40000UL       // kHz
#define DUTY_CYCLE_PERCENT   50            // %
#define PHASE_SHIFT_DEGREES  72            // ° phase shift
#define TAPS 30

#define SAMPLES 5000


u16 buff[SAMPLES];
u16 out_buff[SAMPLES];

const float fir_coeffs[TAPS] = {
    0.0044617840, 0.0051700875, 0.0071170537, 0.0102782692, 0.0145587964,
    0.0197966313, 0.0257702789, 0.0322100566, 0.0388124958, 0.0452570056,
    0.0512238106, 0.0564120830, 0.0605571659, 0.0634458335, 0.0649286479,
    0.0649286479, 0.0634458335, 0.0605571659, 0.0564120830, 0.0512238106,
    0.0452570056, 0.0388124958, 0.0322100566, 0.0257702789, 0.0197966313,
    0.0145587964, 0.0102782692, 0.0071170537, 0.0051700875, 0.0044617840
};


XAdcPs_Config *ConfigPtr;
static XAdcPs XAdcInst;
XAdcPs* XAdcInstPtr = &XAdcInst;

uint32_t calculate_period(uint32_t system_clock, uint32_t pwm_frequency) {
    return system_clock / pwm_frequency;
}

uint32_t calculate_duty_cycle(uint32_t period, uint32_t duty_cycle_percent) {
    return (period * duty_cycle_percent) / 100;
}

int32_t calculate_phase(uint32_t period, int32_t phase_degrees) {
    return ((int32_t)period * phase_degrees + 180) / 360;
}

void apply_fir_filter(u16 *input, u16 *output) {
    for (int i = 0; i < SAMPLES; i++) {
        float acc = 0.0f;
        for (int j = 0; j < TAPS; j++) {
            if (i >= j) {
                acc += fir_coeffs[j] * (float)input[i - j];
            }
        }
        output[i] = (u16)acc;
    }
}

#define JE1_PIN 13
#define JE2_PIN 10

void initialize_gpio(XGpioPs *GpioInstance) {
	   XGpioPs_Config *ConfigPtr;
	    int Status;

	    // Lookup and initialize GPIO configuration
	    ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	    Status = XGpioPs_CfgInitialize(GpioInstance, ConfigPtr, ConfigPtr->BaseAddr);
	    if (Status != XST_SUCCESS) {
	        return;
	    }

	    // Set direction and enable output for JE1 and JE2
	    XGpioPs_SetDirectionPin(GpioInstance, JE1_PIN, 1);  // Output
	    XGpioPs_SetOutputEnablePin(GpioInstance, JE1_PIN, 1);;

}

int main(void) {
    uint32_t period_val, duty_val, phase_val;

    	XGpioPs GpioInstance;

   	    initialize_gpio(&GpioInstance);


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



//	int32_t current_phase = -108;

    while(1) {


//    		int32_t current_phase_val = calculate_phase(period_val, current_phase);
//    		Xil_Out32(PWM_BASE_ADDR + PHASE_OFFSET, current_phase_val);
//    		u16 actual_phase_shift = (((current_phase/18)*5)+30);

    	XGpioPs_WritePin(&GpioInstance, JE1_PIN, 1);
    	 usleep(375);
        XGpioPs_WritePin(&GpioInstance, JE1_PIN, 0);

        for (int i = 0; i < SAMPLES; i++)
        	{
        		buff[i] = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VPVN);
        	}

        	u64 avg = 0;
        	for (int i = 0; i < SAMPLES; i++) avg += buff[i];
        	avg /= SAMPLES;

        	for (int i = 0; i < SAMPLES; i++)
        	{
        		if (buff[i] >= avg)
        		{
        			buff[i] -= avg;
        		}
        		else
        		{
        			buff[i] = avg - buff[i];
        		}
        	}

        	// Filter the data
        	apply_fir_filter(buff, out_buff);


        	//Send the data with phase
//        	for (int j = 0; j < SAMPLES/10; j++)
//        	{
//        	   	printf("%04X%03X", out_buff[10*j],actual_phase_shift);
//        	}
//        	printf("\n");




        	// Send the data
        	for (int j = 0; j < SAMPLES/10; j++)
        	{
        		printf("%04X", out_buff[10*j]);
        	}
        	printf("\n");


//    		if(current_phase >=105)
//    		{
//    			current_phase = -108;
//    		}
//    		else
//     		{
//     			current_phase = current_phase + 18;
//     		}
    }
    return 0;
}
