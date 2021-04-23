#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "rc522.h"
#include "led.h"

/**
*   ����˵����
*   1--SDA  <----->PA4
*   2--SCK  <----->PA5
*   3--MOSI <----->PA7
*   4--MISO <----->PA6
*   5--����
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
    delay_init();	    	 //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
    LED_Init();
    RC522_Init();       //��ʼ����Ƶ��ģ��
	
	delay_ms(3000);
	// RC522_data_break(); // ���������ݱ��Ƴ��򣬽���ѧϰ�ο�������Ƿ�ʹ��

	// UIDΪ��Ҫ�޸ĵĿ���UID key_type��0ΪKEYA����0ΪKEYB KEYΪ��Կ RW:1�Ƕ���0��д data_addrΪ�޸ĵĵ�ַ dataΪ��������
	IC_RW ( UID, 0, KEY_A, 1, 0x10, data );
	
    while(1)
    {
		// ��Ҫ���ѣ�д�������з��գ��������Ⳣ�ԣ����ܱ�֤����ȫ�ԣ����˶��κγ�������Ų����𣬲��������κγ�������µ��κ���ʧ����
		// ���Գ���0�����addr��д��
        RC522_Handle();
		// ����ʹ�� ���Գ���1�����0x0F�� ��֤KEY_A��KEY_B �� дRFID1 ��֤KEY_A1��KEY_B1 �� дRFID2
		// RC522_Handle1();

        if(num % 20 == 0)
            LED0 = !LED0;
        num++;
    }
}
