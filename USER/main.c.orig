#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "rc522.h"
#include "led.h"

/**
*   连线说明：
*   1--SDA  <----->PA4
*   2--SCK  <----->PA5
*   3--MOSI <----->PA7
*   4--MISO <----->PA6
*   5--悬空
*   6--GND <----->GND
*   7--RST <----->PB0
*   8--VCC <----->VCC
**/

int main(void)
{
	u8 num = 0;
    delay_init();	    	 //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口初始化为115200
	 LED_Init();
    RC522_Init();       //初始化射频卡模块
    while(1)
    {
        RC522_Handle();
		 
		 if(num % 20 == 0)
			LED0 = !LED0;
		 num++;
    }
}


