/*
 * main.c
 */
#pragma diag_push
#pragma CHECK_MISRA("none")
#include "includes/Dio.h"
#include "includes/Can.h"
//#include "includes/Std_Types.h"
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include <InitConsole.h>
#include <includes/CanIf.h>
#include <includes/PduR.h>
#include <includes/Com.h>
#include <KeyPad.h>
#include <PORTF.h>


#include <CanTp/CanTp.h>

//DeclareTask(Task_red);
//DeclareTask(Task_white);

DeclareTask(T1);
DeclareTask(T2);

DeclareEvent(ayEv);
DeclareEvent(ayEv1);
DeclareEvent(ayEv2);

//DeclareAlarm(alar1);

//#pragma RESET_MISRA("all")


// add this to test github for the first time //


//int main(void);

// For test
extern CanTp_ConfigType CanTp_Config;

uint8 flag = 1;
int main(void)
{

    PORTF_Init();

    CanTp_Init(&CanTp_Config);

    StartOS(OSDEFAULTAPPMODE);

	return 0;
}

TASK(T1) // every 1 sec
{

//    GPIO_PORTF_DATA_R |=(1<<1);           //red
	CanTp_MainFunction();
    TerminateTask();
}

TASK(T2) // every 5 sec
{
    // Transmit();
	uint8 Array1[] = {0x10,0x0C,'M','o','h','a','m','e'};   	//{SF,FF
	uint8 Array2[] = {0x21, 'd' ,'F','a','r','a','g'};          /* segment number = 1 */
	PduInfoType FF;
    PduInfoType CF;

	FF.SduDataPtr = Array1;

	FF.SduLength=8;                      /* Lazm ykon >= 8 3shan my7slsh return */
	CF.SduDataPtr = Array2;
	CF.SduLength=7;

    if (flag == 1)
    {
    	CanTp_RxIndication(1,&FF);
        CanTp_RxIndication(1,&CF);

		flag = 0;
    }
//	 GPIO_PORTF_DATA_R |=(1<<2);             //
    TerminateTask();
}

