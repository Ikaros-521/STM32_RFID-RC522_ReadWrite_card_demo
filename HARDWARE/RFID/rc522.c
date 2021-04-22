#include "sys.h"
#include "rc522.h"
#include "delay.h"
#include "usart.h"
#include "string.h"

//////////////////////////////////////////////////////////
// M1卡分为16个扇区，每个扇区由四个块（块0、块1、块2、块3）组成
// 将16个扇区的64个块按绝对地址编号为：0~63
// 第0个扇区的块0（即绝对地址0块），用于存放厂商代码，已经固化不可更改
// 每个扇区的块0、块1、块2为数据块，可用于存放数据
// 每个扇区的块3为控制块（绝对地址为:块3、块7、块11.....）包括密码A，存取控制、密码B等

/*******************************
*连线说明：
*1--SDA  <----->PA4
*2--SCK  <----->PA5
*3--MOSI <----->PA7
*4--MISO <----->PA6
*5--悬空
*6--GND <----->GND
*7--RST <----->PB0
*8--VCC <----->VCC
************************************/

/*全局变量*/
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char DATA[16];			//存放数据
unsigned char RFID[16];			//存放RFID
unsigned char card0_bit=0;
unsigned char card1_bit=0;
unsigned char card2_bit=0;
unsigned char card3_bit=0;
unsigned char card4_bit=0;
unsigned char total=0;
// 这UID定义在这不知道干啥用的。。。 替换成自己卡的UID
unsigned char card_0[4]= {73,224,5,152};
unsigned char card_1[4]= {105,102,100,152};
unsigned char card_2[4]= {208,121,31,57};
unsigned char card_3[4]= {176,177,143,165};
unsigned char card_4[4]= {5,158,10,136};
u8 KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
u8 KEY_B[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
u8 AUDIO_OPEN[6] = {0xAA, 0x07, 0x02, 0x00, 0x09, 0xBC};
// 测试用 3区块数据
unsigned char RFID1[16]= {0x10,0x20,0x30,0x40,0x50,0x60,0xff,0x07,0x80,0x29,0x01,0x02,0x03,0x04,0x05,0x06};
unsigned char RFID2[16]= {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
// 测试用 3区块密钥
u8 KEY_A1[6]= {0x10,0x20,0x30,0x40,0x50,0x60};
u8 KEY_A2[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
u8 KEY_B1[6]= {0x01,0x02,0x03,0x04,0x05,0x06};
u8 KEY_B2[6]= {0x10,0x20,0x30,0x00,0x00,0x00};
u8 KEY_B3[6]= {0x01,0x02,0x03,0x00,0x00,0x00};
// 置零用
unsigned char DATA0[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char DATA1[16]= {0x12,0x34,0x56,0x78,0x9A,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char status;
// 0x08 就是2扇区0区块（即第9块）
unsigned char addr=0x08;
// unsigned char addr=0x08;

#define   RC522_DELAY()  delay_us( 20 )

// 测试程序0，完成addr读写读
void RC522_Handle(void)
{
    u8 i = 0;
    status = PcdRequest(PICC_REQALL,CT);//寻卡

    // printf("\r\nstatus>>>>>>%d\r\n", status);

    if(status==MI_OK)// 寻卡成功
    {
        status=MI_ERR;
        status = PcdAnticoll(SN);// 防冲撞 获得UID 存入SN
    }

    if (status==MI_OK)// 防冲撞成功
    {
        status = MI_ERR;
        ShowID(SN); // 串口打印卡的ID号 UID

        // 难道就是为了做个判断吗。。。
        if((SN[0]==card_0[0])&&(SN[1]==card_0[1])&&(SN[2]==card_0[2])&&(SN[3]==card_0[3]))
        {
            card0_bit=1;
            printf("\r\nThe User is:card_0\r\n");
        }
        if((SN[0]==card_1[0])&&(SN[1]==card_1[1])&&(SN[2]==card_1[2])&&(SN[3]==card_1[3]))
        {
            card1_bit=1;
            printf("\r\nThe User is:card_1\r\n");
        }
        if((SN[0]==card_2[0])&&(SN[1]==card_2[1])&&(SN[2]==card_2[2])&&(SN[3]==card_2[3]))
        {
            card2_bit=1;
            printf("\r\nThe User is:card_2\r\n");
        }

        if((SN[0]==card_3[0])&&(SN[1]==card_3[1])&&(SN[2]==card_3[2])&&(SN[3]==card_3[3]))
        {
            card3_bit=1;
            printf("\r\nThe User is:card_3\r\n");
        }
        if((SN[0]==card_4[0])&&(SN[1]==card_4[1])&&(SN[2]==card_4[2])&&(SN[3]==card_4[3]))
        {
            card4_bit=1;
            printf("\r\nThe User is:card_4\r\n");
        }
        //total = card1_bit+card2_bit+card3_bit+card4_bit+card0_bit;
        status = PcdSelect(SN);
    }
    else
    {
    }

    if(status == MI_OK)//选卡成功
    {
        status = MI_ERR;
        // 验证A密钥 块地址 密码 SN 
		// 注意：此处的块地址0x0B即2扇区3区块，此块地址只需要指向某一扇区就可以了，即2扇区为0x08-0x0B这个范围都有效，且只能对验证过的扇区进行读写操作
        status = PcdAuthState(0x60, 0x0B, KEY_A, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(A) success\r\n");
        }
        else
        {
            printf("PcdAuthState(A) failed\r\n");
        }
		// 验证B密钥 块地址 密码 SN 
		status = PcdAuthState(0x61, 0x0B, KEY_B, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(B) success\r\n");
        }
        else
        {
            printf("PcdAuthState(B) failed\r\n");
        }
    }

    if(status == MI_OK)//验证成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据 注意：因为上面验证的扇区是2扇区，所以只能对2扇区的数据进行读写，即0x08-0x0B这个范围，超出范围读取失败。
        status = PcdRead(addr, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("RFID:%s\r\n", RFID);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

    if(status == MI_OK)//读卡成功
    {
        status = MI_ERR;
		printf("Write the card after 1 second. Do not move the card!!!\r\n");
		delay_ms(1000);
        // status = PcdWrite(addr, DATA0);
        // 写数据到M1卡一块
        status = PcdWrite(addr, DATA1);
        if(status == MI_OK)//写卡成功
        {
            printf("PcdWrite() success\r\n");
        }
        else
        {
            printf("PcdWrite() failed\r\n");
			delay_ms(3000);
        }
    }

    if(status == MI_OK)//写卡成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据
        status = PcdRead(addr, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("DATA:%s\r\n", DATA);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

    if(status == MI_OK)//读卡成功
    {
        status = MI_ERR;
		printf("RC522_Handle() run finished after 1 second!\r\n");
        delay_ms(1000);
    }
}

// 测试程序1，完成0x0F块 验证KEY_A、KEY_B 读 写RFID1 验证KEY_A1、KEY_B1 读 写RFID2
void RC522_Handle1(void)
{
    u8 i = 0;
    status = PcdRequest(PICC_REQALL,CT);//寻卡

    // printf("\r\nstatus>>>>>>%d\r\n", status);

    if(status==MI_OK)// 寻卡成功
    {
        status=MI_ERR;
        status = PcdAnticoll(SN);// 防冲撞 获得UID 存入SN
    }

    if (status==MI_OK)// 防冲撞成功
    {
        status = MI_ERR;
        ShowID(SN); // 串口打印卡的ID号 UID

        // 难道就是为了做个判断吗。。。
        if((SN[0]==card_0[0])&&(SN[1]==card_0[1])&&(SN[2]==card_0[2])&&(SN[3]==card_0[3]))
        {
            card0_bit=1;
            printf("\r\nThe User is:card_0\r\n");
        }
        if((SN[0]==card_1[0])&&(SN[1]==card_1[1])&&(SN[2]==card_1[2])&&(SN[3]==card_1[3]))
        {
            card1_bit=1;
            printf("\r\nThe User is:card_1\r\n");
        }
        
        status = PcdSelect(SN);
    }
    else
    {
    }

    if(status == MI_OK)//选卡成功
    {
        status = MI_ERR;
        // 验证A密钥 块地址 密码 SN 
		// 注意：此处的块地址0x0F即3扇区3区块，此块地址只需要指向某一扇区就可以了，即3扇区为0x0C-0x0F这个范围都有效，且只能对验证过的扇区进行读写操作
        status = PcdAuthState(0x60, 0x0F, KEY_A, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(A) success\r\n");
        }
        else
        {
            printf("PcdAuthState(A) failed\r\n");
			status = MI_OK;
			goto P1;
        }
		// 验证B密钥 块地址 密码 SN 
		status = PcdAuthState(0x61, 0x0F, KEY_B, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(B) success\r\n");
        }
        else
        {
            printf("PcdAuthState(B) failed\r\n");
        }
    }

    if(status == MI_OK)//验证成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据 注意：因为上面验证的扇区是3扇区，所以只能对2扇区的数据进行读写，即0x0C-0x0F这个范围，超出范围读取失败。
        status = PcdRead(0x0F, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("RFID:%s\r\n", RFID);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

    if(status == MI_OK)//读卡成功
    {
        status = MI_ERR;
        // 写数据到M1卡一块
        status = PcdWrite(0x0F, RFID1);
        if(status == MI_OK)//写卡成功
        {
            printf("PcdWrite(RFID1) success\r\n");
        }
        else
        {
            printf("PcdWrite(RFID1) failed\r\n");
			delay_ms(3000);
        }
    }

P1:	
	if(status == MI_OK)//写卡成功
    {
        status = MI_ERR;
        // 验证A密钥 块地址 密码 SN 
		// 注意：此处的块地址0x0F即3扇区3区块，此块地址只需要指向某一扇区就可以了，即3扇区为0x0C-0x0F这个范围都有效，且只能对验证过的扇区进行读写操作
        status = PcdAuthState(0x60, 0x0F, KEY_A2, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(A1) success\r\n");
        }
        else
        {
            printf("PcdAuthState(A1) failed\r\n");
        }
		// 验证B密钥 块地址 密码 SN 
		status = PcdAuthState(0x61, 0x0F, KEY_B3, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(B1) success\r\n");
        }
        else
        {
            printf("PcdAuthState(B1) failed\r\n");
        }
    }
	
	if(status == MI_OK)//验证成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据 注意：因为上面验证的扇区是3扇区，所以只能对2扇区的数据进行读写，即0x0C-0x0F这个范围，超出范围读取失败。
        status = PcdRead(0x0F, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("RFID:%s\r\n", RFID);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }
	
	if(status == MI_OK)//读卡成功
    {
        status = MI_ERR;
        // 写数据到M1卡一块
        status = PcdWrite(0x0F, RFID2);
        if(status == MI_OK)//写卡成功
        {
            printf("PcdWrite(RFID2) success\r\n");
        }
        else
        {
            printf("PcdWrite(RFID2) failed\r\n");
			delay_ms(3000);
        }
    }

    if(status == MI_OK)//写卡成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据
        status = PcdRead(0x0F, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("DATA:%s\r\n", DATA);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

    if(status == MI_OK)//读卡成功
    {
        status = MI_ERR;
        delay_ms(1000);
    }
}

// 测试用数据爆破程序，仅供学习参考，请勿非法使用 针对card_0进行破解
void RC522_data_break(void)
{
	// 爆破的块地址
	unsigned char break_addr = 0x0F;
	u8 i = 0;
	/*
	u8 key_arr[257] = {	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
						0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
						0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
						0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 
						0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
						0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 
						0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 
						0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 
						0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
						0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
						0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
						0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
						0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
						0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
						0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 
						0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };
	*/
	u8 break_KEY[6]= {0, 0, 0, 0, 0, 0};
	
    status = PcdRequest(PICC_REQALL,CT);//寻卡

    // printf("\r\nstatus>>>>>>%d\r\n", status);

    if(status==MI_OK)// 寻卡成功
    {
        status=MI_ERR;
        status = PcdAnticoll(SN);// 防冲撞 获得UID 存入SN
    }

    if (status==MI_OK)// 防冲撞成功
    {
        status = MI_ERR;
        ShowID(SN); // 串口打印卡的ID号 UID

        // 难道就是为了做个判断吗。。。
        if((SN[0]==card_0[0])&&(SN[1]==card_0[1])&&(SN[2]==card_0[2])&&(SN[3]==card_0[3]))
        {
            card0_bit=1;
            printf("\r\nThe User is:card_0\r\n");
        }
		else
		{
			printf("\r\nThe User isn't:card_0\r\n");
			return;
		}
        
        status = PcdSelect(SN);
    }
    else
    {
    }

    if(status == MI_OK)//选卡成功
    {
        status = MI_ERR;
		
		// 自由发挥 。。。
		
		// 验证A密钥 块地址 密码 SN 
		// 注意：此处的块地址0x0F即3扇区3区块，此块地址只需要指向某一扇区就可以了，即3扇区为0x0C-0x0F这个范围都有效，且只能对验证过的扇区进行读写操作
		status = PcdAuthState(0x60, break_addr, break_KEY, SN);
		if(status == MI_OK)//验证成功
		{
			printf("PcdAuthState(A) success\r\n");
		}
		else
		{
			printf("PcdAuthState(A) failed\r\n");
			status = MI_OK;
		}
		
		// 验证B密钥 块地址 密码 SN 
		status = PcdAuthState(0x61, break_addr, break_KEY, SN);
		if(status == MI_OK)//验证成功
		{
			printf("PcdAuthState(B) success\r\n");
		}
		else
		{
			printf("PcdAuthState(B) failed\r\n");
		}
    }

    if(status == MI_OK)//验证成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据 注意：因为上面验证的扇区是3扇区，所以只能对2扇区的数据进行读写，即0x0C-0x0F这个范围，超出范围读取失败。
        status = PcdRead(break_addr, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("RFID:%s\r\n", RFID);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

    delay_ms(3000);
}

void RC522_Init ( void )
{
    SPI1_Init();

    RC522_Reset_Disable();

    RC522_CS_Disable();

    PcdReset ();

    M500PcdConfigISOType ( 'A' );//设置工作方式

}

void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );//PORTA、B时钟使能
    RCC_APB1PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );												//SPI1时钟使能

    // CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化PF0、PF1

    // SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RST
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  					//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;																	//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;															//设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;																		//串行同步时钟的空闲状态为高电平
    // SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    // SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;																	//串行同步时钟的第一个跳变沿（下降）数据被采样
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;																		//串行同步时钟的第二个跳变沿（上升）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;																			//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    // RC522 SPI通讯时钟周期最小为100ns	即频率最大为10MHZ
    // RC522 数据在下降沿变化
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;					//定义波特率预分频的值:波特率预分频值为256、传输速率36M/256=140.625KHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;														//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;																			//CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure); 						 															//根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI1, ENABLE); //使能SPI外设
}


/*
 * 函数名：SPI_RC522_SendByte
 * 描述  ：向RC522发送1 Byte 数据
 * 输入  ：byte，要发送的数据
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
void SPI_RC522_SendByte ( u8 byte )
{
    u8 counter;

    for(counter=0; counter<8; counter++)
    {
        if ( byte & 0x80 )
            RC522_MOSI_1 ();
        else
            RC522_MOSI_0 ();

        RC522_DELAY();
        RC522_SCK_0 ();
        RC522_DELAY();
        RC522_SCK_1();
        RC522_DELAY();

        byte <<= 1;
    }
}


/*
 * 函数名：SPI_RC522_ReadByte
 * 描述  ：从RC522发送1 Byte 数据
 * 输入  ：无
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
u8 SPI_RC522_ReadByte ( void )
{
    u8 counter;
    u8 SPI_Data;

    for(counter=0; counter<8; counter++)
    {
        SPI_Data <<= 1;

        RC522_SCK_0 ();

        RC522_DELAY();

        if ( RC522_MISO_GET() == 1)
            SPI_Data |= 0x01;

        RC522_DELAY();

        RC522_SCK_1 ();

        RC522_DELAY();
    }

//	printf("****%c****",SPI_Data);
    return SPI_Data;
}


/*
 * 函数名：ReadRawRC
 * 描述  ：读RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 * 返回  : 寄存器的当前值
 * 调用  ：内部调用
 */
u8 ReadRawRC ( u8 ucAddress )
{
    u8 ucAddr, ucReturn;

    ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;

    RC522_CS_Enable();

    SPI_RC522_SendByte ( ucAddr );

    ucReturn = SPI_RC522_ReadByte ();

    RC522_CS_Disable();

    return ucReturn;
}


/*
 * 函数名：WriteRawRC
 * 描述  ：写RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 *         ucValue，写入寄存器的值
 * 返回  : 无
 * 调用  ：内部调用
 */
void WriteRawRC ( u8 ucAddress, u8 ucValue )
{
    u8 ucAddr;

    ucAddr = ( ucAddress << 1 ) & 0x7E;

    RC522_CS_Enable();

    SPI_RC522_SendByte ( ucAddr );

    SPI_RC522_SendByte ( ucValue );

    RC522_CS_Disable();
}


/*
 * 函数名：SetBitMask
 * 描述  ：对RC522寄存器置位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，置位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void SetBitMask ( u8 ucReg, u8 ucMask )
{
    u8 ucTemp;

    ucTemp = ReadRawRC ( ucReg );

    WriteRawRC ( ucReg, ucTemp | ucMask );         // set bit mask
}


/*
 * 函数名：ClearBitMask
 * 描述  ：对RC522寄存器清位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，清位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void ClearBitMask ( u8 ucReg, u8 ucMask )
{
    u8 ucTemp;

    ucTemp = ReadRawRC ( ucReg );

    WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
}


/*
 * 函数名：PcdAntennaOn
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOn ( void )
{
    u8 uc;

    uc = ReadRawRC ( TxControlReg );

    if ( ! ( uc & 0x03 ) )
        SetBitMask(TxControlReg, 0x03);
}


/*
 * 函数名：PcdAntennaOff
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOff ( void )
{
    ClearBitMask ( TxControlReg, 0x03 );
}


/*
 * 函数名：PcdRese
 * 描述  ：复位RC522
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 */
void PcdReset ( void )
{
    RC522_Reset_Disable();

    delay_us ( 1 );

    RC522_Reset_Enable();

    delay_us ( 1 );

    RC522_Reset_Disable();

    delay_us ( 1 );

    WriteRawRC ( CommandReg, 0x0f );

    while ( ReadRawRC ( CommandReg ) & 0x10 );

    delay_us ( 1 );

    WriteRawRC ( ModeReg, 0x3D );            //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363

    WriteRawRC ( TReloadRegL, 30 );          //16位定时器低位
    WriteRawRC ( TReloadRegH, 0 );			 //16位定时器高位

    WriteRawRC ( TModeReg, 0x8D );		      //定义内部定时器的设置

    WriteRawRC ( TPrescalerReg, 0x3E );			 //设置定时器分频系数

    WriteRawRC ( TxAutoReg, 0x40 );				   //调制发送信号为100%ASK
}


/*
 * 函数名：M500PcdConfigISOType
 * 描述  ：设置RC522的工作方式
 * 输入  ：ucType，工作方式
 * 返回  : 无
 * 调用  ：外部调用
 */
void M500PcdConfigISOType ( u8 ucType )
{
    if ( ucType == 'A')                     //ISO14443_A
    {
        ClearBitMask ( Status2Reg, 0x08 );

        WriteRawRC ( ModeReg, 0x3D );//3F

        WriteRawRC ( RxSelReg, 0x86 );//84

        WriteRawRC( RFCfgReg, 0x7F );   //4F

        WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)

        WriteRawRC ( TReloadRegH, 0 );

        WriteRawRC ( TModeReg, 0x8D );

        WriteRawRC ( TPrescalerReg, 0x3E );

        delay_us ( 2 );

        PcdAntennaOn ();//开天线
    }
}


/*
 * 函数名：PcdComMF522
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：ucCommand，RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：内部调用
 */
char PcdComMF522 ( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit )
{
    char cStatus = MI_ERR;
    u8 ucIrqEn   = 0x00;
    u8 ucWaitFor = 0x00;
    u8 ucLastBits;
    u8 ucN;
    u32 ul;

    switch ( ucCommand )
    {
    case PCD_AUTHENT:		//Mifare认证
        ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
        break;

    case PCD_TRANSCEIVE:		//接收发送 发送接收
        ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;

    default:
        break;
    }

    WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    ClearBitMask ( ComIrqReg, 0x80 );			//Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC ( CommandReg, PCD_IDLE );		//写空闲命令
    SetBitMask ( FIFOLevelReg, 0x80 );			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除

    for ( ul = 0; ul < ucInLenByte; ul ++ )
        WriteRawRC ( FIFODataReg, pInData [ ul ] );    		//写数据进FIFOdata

    WriteRawRC ( CommandReg, ucCommand );					//写命令

    if ( ucCommand == PCD_TRANSCEIVE )
        SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效

    ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms

    do 														//认证 与寻卡等待时间
    {
        ucN = ReadRawRC ( ComIrqReg );							//查询事件中断
        ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//退出条件i=0,定时器中断，与写空闲命令

    ClearBitMask ( BitFramingReg, 0x80 );					//清理允许StartSend位

    if ( ul != 0 )
    {
        if ( ! (( ReadRawRC ( ErrorReg ) & 0x1B )) )			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
        {
            cStatus = MI_OK;

            if ( ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
                cStatus = MI_NOTAGERR;

            if ( ucCommand == PCD_TRANSCEIVE )
            {
                ucN = ReadRawRC ( FIFOLevelReg );			//读FIFO中保存的字节数

                ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	//最后接收到得字节的有效位数

                if ( ucLastBits )
                    * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                else
                    * pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效

                if ( ucN == 0 )
                    ucN = 1;

                if ( ucN > MAXRLEN )
                    ucN = MAXRLEN;

                for ( ul = 0; ul < ucN; ul ++ )
                    pOutData [ ul ] = ReadRawRC ( FIFODataReg );
            }
        }
        else
            cStatus = MI_ERR;
//			printf(ErrorReg);
    }

    SetBitMask ( ControlReg, 0x80 );           // stop timer now
    WriteRawRC ( CommandReg, PCD_IDLE );

    return cStatus;
}


/*
 * 函数名：PcdRequest
 * 描述  ：寻卡
 * 输入  ：ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRequest ( u8 ucReq_code, u8 * pTagType )
{
    char cStatus;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ClearBitMask ( Status2Reg, 0x08 );	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
    WriteRawRC ( BitFramingReg, 0x07 );	//	发送的最后一个字节的 七位
    SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

    ucComMF522Buf [ 0 ] = ucReq_code;		//存入 卡片命令字

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//寻卡

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡类型
    {
        * pTagType = ucComMF522Buf [ 0 ];
        * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
    }
    else
        cStatus = MI_ERR;

    return cStatus;
}


/*
 * 函数名：PcdAnticoll
 * 描述  ：防冲撞
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAnticoll ( u8 * pSnr )
{
    char cStatus;
    u8 uc, ucSnr_check = 0;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ClearBitMask ( Status2Reg, 0x08 );		//清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    WriteRawRC ( BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask ( CollReg, 0x80 );			//清ValuesAfterColl所有接收的位在冲突后被清除

    /*
    参考ISO14443协议：https://blog.csdn.net/wowocpp/article/details/79910800
    PCD 发送 SEL = ‘93’，NVB = ‘20’两个字节
    迫使所有的在场的PICC发回完整的UID CLn作为应答。
    */
    ucComMF522Buf [ 0 ] = 0x93;	//卡片防冲突命令
    ucComMF522Buf [ 1 ] = 0x20;

    // 发送并接收数据 接收的数据存储于ucComMF522Buf
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信

    if ( cStatus == MI_OK)		//通信成功
    {
        // 收到的UID 存入pSnr
        for ( uc = 0; uc < 4; uc ++ )
        {
            * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//读出UID
            ucSnr_check ^= ucComMF522Buf [ uc ];
        }

        if ( ucSnr_check != ucComMF522Buf [ uc ] )
            cStatus = MI_ERR;

    }

    SetBitMask ( CollReg, 0x80 );

    return cStatus;
}


/*
 * 函数名：CalulateCRC
 * 描述  ：用RC522计算CRC16
 * 输入  ：pIndata，计算CRC16的数组
 *         ucLen，计算CRC16的数组字节长度
 *         pOutData，存放计算结果存放的首地址
 * 返回  : 无
 * 调用  ：内部调用
 */
void CalulateCRC ( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
    u8 uc, ucN;

    ClearBitMask(DivIrqReg, 0x04);

    WriteRawRC(CommandReg, PCD_IDLE);

    SetBitMask(FIFOLevelReg, 0x80);

    for ( uc = 0; uc < ucLen; uc ++)
        WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );

    WriteRawRC ( CommandReg, PCD_CALCCRC );

    uc = 0xFF;

    do
    {
        ucN = ReadRawRC ( DivIrqReg );
        uc --;
    } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );

    pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
    pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );
}


/*
 * 函数名：PcdSelect
 * 描述  ：选定卡片
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdSelect ( u8 * pSnr )
{
    char cStatus;
    u8 uc;
    u8 ucComMF522Buf [ MAXRLEN ];
    u32  ulLen;

    // 防冲撞 0x93
    ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
    // 假设没有冲突，PCD 指定NVB为70，此值表示PCD将发送完整的UID CLn，与40位UID CLn 匹配的PICC，以SAK作为应答
    ucComMF522Buf [ 1 ] = 0x70;
    ucComMF522Buf [ 6 ] = 0;

    // 3 4 5 6位存放UID，第7位一直异或。。。
    for ( uc = 0; uc < 4; uc ++ )
    {
        ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
        ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
    }

    // CRC(循环冗余校验)
    CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

    ClearBitMask ( Status2Reg, 0x08 );

    // 发送并接收数据
    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x18 ) )
        cStatus = MI_OK;
    else
        cStatus = MI_ERR;

    return cStatus;
}


/*
 * 函数名：PcdAuthState
 * 描述  ：验证卡片密码
 * 输入  ：ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 *         u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAuthState ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = ucAuth_mode;
    ucComMF522Buf [ 1 ] = ucAddr;

    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );

    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );

    // printf("char PcdAuthState ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )\r\n");
    // printf("before PcdComMF522() ucComMF522Buf:%s\r\n", ucComMF522Buf);

    // 验证密钥命令
    cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );

    // printf("after PcdComMF522() ucComMF522Buf:%s\r\n", ucComMF522Buf);

    if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )
    {
//			if(cStatus != MI_OK)
//					printf("666")	;
//			else
//				printf("888");
        cStatus = MI_ERR;
    }

    return cStatus;
}


/*
 * 函数名：PcdWrite
 * 描述  ：写数据到M1卡一块
 * 输入  ：u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdWrite ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_WRITE;
    ucComMF522Buf [ 1 ] = ucAddr;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;

    if ( cStatus == MI_OK )
    {
        memcpy(ucComMF522Buf, pData, 16);
        for ( uc = 0; uc < 16; uc ++ )
            ucComMF522Buf [ uc ] = * ( pData + uc );

        CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

        cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );

        if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
            cStatus = MI_ERR;

    }
    return cStatus;
}


/*
 * 函数名：PcdRead
 * 描述  ：读取M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRead ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
        for ( uc = 0; uc < 16; uc ++ )
            * ( pData + uc ) = ucComMF522Buf [ uc ];
    }
    else
        cStatus = MI_ERR;

    return cStatus;
}


/*
 * 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdHalt( void )
{
    u8 ucComMF522Buf [ MAXRLEN ];
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;

    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    return MI_OK;
}


void IC_CMT ( u8 * UID, u8 * KEY, u8 RW, u8 * Dat )
{
    u8 ucArray_ID [ 4 ] = { 0 };//先后存放IC卡的类型和UID(IC卡序列号)

    PcdRequest ( 0x52, ucArray_ID );//寻卡

    PcdAnticoll ( ucArray_ID );//防冲撞

    PcdSelect ( UID );//选定卡

    PcdAuthState ( 0x60, 0x10, KEY, UID );//校验

    if ( RW )//读写选择，1是读，0是写
        PcdRead ( 0x10, Dat );
    else
        PcdWrite ( 0x10, Dat );

    PcdHalt ();
}

// 显示卡的卡号，以十六进制显示
void ShowID(u8 *p)
{
    u8 num[9];
    u8 i;

    for(i=0; i<4; i++)
    {
        num[i*2] = p[i] / 16;
        num[i*2] > 9 ? (num[i*2] += '7') : (num[i*2] += '0');
        num[i*2+1] = p[i] % 16;
        num[i*2+1] > 9 ? (num[i*2+1] += '7') : (num[i*2+1] += '0');
    }
    num[8] = 0;
    printf("ID>>>%s\r\n", num);
}

