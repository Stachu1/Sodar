#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xil_io.h"
#include "sleep.h"
#include "xparameters.h"
#include "xadcps.h"
#include "xgpiops.h"
#include "xuartps.h"

#define UART_DEVICE_ID      XPAR_PS7_UART_1_DEVICE_ID
#define XADC_DEVICE_ID      XPAR_XADCPS_0_DEVICE_ID
#define PWM_BASE_ADDR 		XPAR_SODAR_IP_0_S00_AXI_BASEADDR

#define PERIOD_OFFSET       0x00            // Signal period [clk cycles]
#define ON_CYCLES_OFFSET    0x04            // On time [clk cycles]
#define PHASE_OFFSET        0x08            // Phase shift [clk cycles]
#define SYSTEM_CLOCK        100000000UL   	// clk speed = 100 MHz
#define PWM_FREQUENCY       40000UL       	// tx_freq = 40 kHz

#define FIR_TAPS            30              // FIR low-pass taps
#define SAMPLES             5000			// Echo ADC samples (5000 ~ 2m)
#define SAMPLE_DIVIDER      50				// Send every 50th sample
#define BURST_LEN           10				// 40Khz square-wave cycles
#define PULSES              3				// Send n pulses and average the results (noise rejection)
#define BAUD_RATE           230400          // Serial transmit speed


#define EVER (;;)

#define EN_PIN 13
#define LED_PIN 10

XGpioPs GpioInstance;

static XAdcPs XAdcInst;
XAdcPs* XAdcInstPtr = &XAdcInst;

u32 adc_buff[SAMPLES];
u32 out_buff[SAMPLES];

enum Mode {SRC, TRK} mode = TRK;


const float fir_coeffs[FIR_TAPS] = {
    0.0044617840, 0.0051700875, 0.0071170537, 0.0102782692, 0.0145587964,
    0.0197966313, 0.0257702789, 0.0322100566, 0.0388124958, 0.0452570056,
    0.0512238106, 0.0564120830, 0.0605571659, 0.0634458335, 0.0649286479,
    0.0649286479, 0.0634458335, 0.0605571659, 0.0564120830, 0.0512238106,
    0.0452570056, 0.0388124958, 0.0322100566, 0.0257702789, 0.0197966313,
    0.0145587964, 0.0102782692, 0.0071170537, 0.0051700875, 0.0044617840
};


float sin_0_30[31] = {
    0.0000, 0.0175, 0.0349, 0.0523, 0.0698, 0.0872, 0.1045, 0.1219, 0.1392, 
    0.1564, 0.1736, 0.1908, 0.2079, 0.2249, 0.2419, 0.2588, 0.2756, 0.2924, 
    0.3090, 0.3256, 0.3420, 0.3584, 0.3746, 0.3907, 0.4067, 0.4226, 0.4384, 
    0.4540, 0.4695, 0.4848, 0.5000
};


void set_beam_angle(int8_t beam_angle)
{
    // 209.9 is the constant of our array used for calculating phase shift
	float phase_shift = 209.9 * sin_0_30[abs(beam_angle)];

    // Account for negative sin values
	if (beam_angle < 0) phase_shift *= -1;

    // Calculate phase as cycles difference
	int32_t phase_val = (int32_t)(SYSTEM_CLOCK / PWM_FREQUENCY * ((180 - phase_shift) / 360));

    // Set signal generators
    Xil_Out32(PWM_BASE_ADDR + PHASE_OFFSET, phase_val);
}


void apply_fir_filter(u32 *input, u32 *output) {
    for (int i = 0; i < SAMPLES; i++) {
        float acc = 0.0f;
        for (int j = 0; j < FIR_TAPS; j++) {
            if (i >= j) {
                acc += fir_coeffs[j] * (float)input[i - j];
            }
        }
        output[i] += acc;
    }
}


void initialize_gpio(XGpioPs *GpioInstance) {
    XGpioPs_Config *ConfigPtr;
    // Lookup and initialize GPIO configuration
    ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
    XGpioPs_CfgInitialize(GpioInstance, ConfigPtr, ConfigPtr->BaseAddr);

    // Set direction and enable output for JE1 and JE2
    XGpioPs_SetDirectionPin(GpioInstance, EN_PIN, 1);
    XGpioPs_SetOutputEnablePin(GpioInstance, EN_PIN, 1);
    XGpioPs_SetDirectionPin(GpioInstance, LED_PIN, 1);
    XGpioPs_SetOutputEnablePin(GpioInstance, LED_PIN, 1);
}


void set_uart_speed(void)
{
	XUartPs Uart_Ps;
	XUartPs_Config *Config;
	Config = XUartPs_LookupConfig(UART_DEVICE_ID);
	XUartPs_CfgInitialize(&Uart_Ps, Config, Config->BaseAddress);
	XUartPs_SetBaudRate(&Uart_Ps, BAUD_RATE);
}


void transmit_and_sample(int8_t pulses, int8_t burst_len)
{
    // Clear buffers
    for (int i = 0; i < SAMPLES; i++) adc_buff[i] = 0;
    for (int i = 0; i < SAMPLES; i++) out_buff[i] = 0;

    // Send & sample pulses
    for (u8 pulse = 0; pulse < pulses; pulse++) {
        XGpioPs_WritePin(&GpioInstance, EN_PIN, 1);
        usleep(25*burst_len);
        XGpioPs_WritePin(&GpioInstance, EN_PIN, 0);

        for (int i = 0; i < SAMPLES; i++)
        {
            adc_buff[i] += XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VPVN);
        }
    }

    // Calculate average
    u64 avg = 0;
    for (int i = 0; i < SAMPLES; i++) avg += adc_buff[i];
    avg /= SAMPLES;

    // Remove offset & take abs of adc_buff data
    for (int i = 0; i < SAMPLES; i++)
    {
        if (adc_buff[i] >= avg)
        {
            adc_buff[i] -= avg;
        }
        else
        {
            adc_buff[i] = avg - adc_buff[i];
        }
    }
}


u32 echo_max_index(u32 *buff, u32 offset){
    u32 val = 0;
    u32 index = 0;
    for (int i = offset; i < SAMPLES; i++)
    {
        if (buff[i] > val)
        {
            val = buff[i];
            index = i;
        }
    }
    return index;
}

u32 max(u32 val1, u32 val2)
{
	if (val1 >= val2)
	{
		return val1;
	}
	else
	{
		return val2;
	}
}


int main(void) {
    // Set uart speed
    set_uart_speed();


    // Init GPIO
    initialize_gpio(&GpioInstance);

    // Init ADC
    XAdcPs_Config *ConfigPtr;
    ConfigPtr = XAdcPs_LookupConfig(XADC_DEVICE_ID);
    XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
    XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SINGCHAN);
    XAdcPs_SetSeqChEnables(XAdcInstPtr, (1 << XADCPS_CH_VPVN));

    // Init signal generators
    Xil_Out32(PWM_BASE_ADDR + PERIOD_OFFSET, SYSTEM_CLOCK / PWM_FREQUENCY);
    Xil_Out32(PWM_BASE_ADDR + ON_CYCLES_OFFSET, SYSTEM_CLOCK / PWM_FREQUENCY / 2);

	int8_t beam_angle = 0;
    for EVER {        
        if (mode == SRC)
        {
            set_beam_angle(beam_angle);    
            transmit_and_sample(PULSES, BURST_LEN);
			apply_fir_filter(adc_buff, out_buff);
    
            // Send the data
            printf("%02X", 0x7F & (u8)(beam_angle+30));
            for (int i = 0; i < SAMPLES / SAMPLE_DIVIDER; i++)
            {
                printf("%04X", (u16)(out_buff[SAMPLE_DIVIDER * i] / PULSES));
            }
            printf("\n");

            if (out_buff[echo_max_index(out_buff, 1e3)] / 65.535 > 60) mode = TRK;
    
            // Shift beam angle
            XGpioPs_WritePin(&GpioInstance, LED_PIN, 0);
            beam_angle++;
            if(beam_angle == 31)
            {
                beam_angle = -30;
                XGpioPs_WritePin(&GpioInstance, LED_PIN, 1);
            }
        }
        if (mode == TRK)
        {
            u32 index = 0;
            u32 index_left = 0;
            u32 index_right = 0;

            set_beam_angle(beam_angle - 1);
            transmit_and_sample(PULSES*3, BURST_LEN*2);
			apply_fir_filter(adc_buff, out_buff);
            index_left = echo_max_index(out_buff, 1e3);
            u32 echo_left = out_buff[index_left];

            set_beam_angle(beam_angle + 1);
            transmit_and_sample(PULSES*3, BURST_LEN*2);
			apply_fir_filter(adc_buff, out_buff);
            index_right = echo_max_index(out_buff, 1e3);
            u32 echo_right = out_buff[index_right];
            
            if (echo_left * 0.9 > echo_right)
            {
                beam_angle -= 1;
                index = index_left;
            }
            
            if (echo_right * 0.9 > echo_left)
            {
                beam_angle += 1;
                index = index_right;
            }

            // Send the data
            if (index != 0)
            {
                printf("%02X", 0x80 | (u8)(beam_angle+30));
                printf("%04X", (u16)index / SAMPLE_DIVIDER);
                printf("\n");
                
            }

            if (max(echo_left, echo_right) / 65.535 < 15) mode = SRC;
        }
    }
    return 0;
}
