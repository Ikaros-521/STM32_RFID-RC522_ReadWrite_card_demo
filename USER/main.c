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
	u8 KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
	u8 data[16] = {0};
	u8 UID[4]= {0x49, 0xe0, 0x05, 0x98};
    delay_init();	    	 //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口初始化为115200
    LED_Init();
    RC522_Init();       //初始化射频卡模块
	
	delay_ms(3000);
	// RC522_data_break(); // 测试用数据爆破程序，仅供学习参考，请勿非法使用

	// UID为你要修改的卡的UID key_type：0为KEYA，非0为KEYB KEY为密钥 RW:1是读，0是写 data_addr为修改的地址 data为数据内容
	IC_RW ( UID, 0, KEY_A, 1, 0x10, data );
	
    while(1)
    {
		// 重要提醒，写卡操作有风险，请勿随意尝试，不能保证程序安全性，本人对任何程序问题概不负责，不限于由任何程序错误导致的任何损失或损害
		// 测试程序0，完成addr读写读
        RC522_Handle();
		// 谨慎使用 测试程序1，完成0x0F块 验证KEY_A、KEY_B 读 写RFID1 验证KEY_A1、KEY_B1 读 写RFID2
		// RC522_Handle1();

        if(num % 20 == 0)
            LED0 = !LED0;
        num++;
    }
}
