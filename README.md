# 前言

## 特别声明:

- 本仓库发布的程序，仅用于测试和学习研究，禁止用于商业用途，不能保证其合法性，准确性，完整性和有效性，请根据情况自行判断。

- 本人对任何脚本问题概不负责，包括但不限于由任何脚本错误导致的任何损失或损害。

- 间接使用脚本的任何用户，包括但不限于建立VPS或在某些行为违反国家/地区法律或相关法规的情况下进行传播, 本人对于由此引起的任何隐私泄漏或其他后果概不负责。

- 任何以任何方式查看此项目的人或直接或间接使用该项目的任何程序的使用者都应仔细阅读此声明。本人保留随时更改或补充此免责声明的权利。一旦使用并复制了任何相关脚本或Script项目的规则，则视为您已接受此免责声明。


本文不含任何广告性质，仅供学习参考。**写卡**需谨慎！！！，不然可能会玩崩了。血的教训！！！

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210422124412824.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

参考资料：

[浅谈IC卡数据分析](https://blog.csdn.net/hiwoshixiaoyu/article/details/104048663)

[智能卡 ISO14443 协议 解读](https://blog.csdn.net/wowocpp/article/details/79910800)

[STM32F103ZET--RFID-RC522使用例程(战舰版)](https://blog.csdn.net/qq_28877125/article/details/80437095)

[M1卡使用说明书](https://wenku.baidu.com/view/50f8bff17c1cfad6195fa712.html)

[M1卡介绍](https://www.cnblogs.com/ivantang/p/3904025.html)

[STM32-RC522](http://www.pudn.com/Download/item/id/2857485.html)

源码参考：[RFID-RC522](http://www.pudn.com/Download/item/id/3930282.html)，不能使用，我进行了一定的修改。下载参考下方传送门。

开发板：正点原子 STM32F103 精英版

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421155946240.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

语言：C语言

开发环境：Keil5

**开发板**使用了 LED  SPI USART RFID-RC522模块 钥匙扣卡 M1卡

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421160054898.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421160217759.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

**Win10软件** SSCOM串口调试  FlyMcu烧录（ps:电脑安装驱动CH340）

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021042116032014.png)

**安卓软件** NFC Writer （手机需有NFC功能）

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421160359827.jpg)

## 代码下载：

[码云](https://gitee.com/ikaros-521/STM32_RFID-RC522_ReadWrite_card_demo) [GitHub](https://github.com/Ikaros-521/STM32_RFID-RC522_ReadWrite_card_demo)

## 功能介绍：

寻卡-》防冲撞-》选卡-》验证2扇区密钥-》读取2扇区0区块数据-》写入数据到2扇区0区块-》再读取2扇区0区块数据。

串口打印卡UID，验证结果，读取到的2扇区0区块数据等信息。

**注意**：只有验证成功的扇区，才能对此扇区进行读写操作！


```c
// 验证A密钥 块地址 密码 SN 
// 注意：此处的块地址0x0B即2扇区3区块，此块地址只需要指向某一扇区就可以了，
// 即2扇区为0x08-0x0B这个范围都有效，且只能对验证过的扇区进行读写操作
status = PcdAuthState(0x60, 0x0B, KEY_A, SN);
// 读取M1卡一块数据 块地址 读取的数据
// 注意：因为上面验证的扇区是2扇区，所以只能对2扇区的数据进行读写，即0x08-0x0B这个范围，
// 超出范围读取失败。
status = PcdRead(0x08, DATA);
```

另外：不知为何，3区块的密钥A单片机读取出来是全00，手机是全ff

# 接线

```javascript
1--SDA  <----->PA4
2--SCK  <----->PA5
3--MOSI <----->PA7
4--MISO <----->PA6
5--悬空
6--GND <----->GND
7--RST <----->PB0
8--VCC <----->VCC
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421161954109.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

## STM32

### STM32F1开发指南(精英版)-库函数版本_V1.2

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421170032803.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

### STM32中文参考手册

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421165656116.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421165747239.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

## RFID-RC522

参考：[https://www.cirmall.com/circuit/2149/](https://www.cirmall.com/circuit/2149/)

### RFID射频模块电路原理图

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421170353601.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

# 使用图+效果图

## 测试程序0 RC522_Handle()

### 最终效果 

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210422102505547.gif#pic_center)


### 一、先用手机软件NFC Writer读取空卡看看内容

#### 1、打开软件和NFC（ps：我的手机是小米10）

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421162424273.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)


#### 2、将空卡贴于手机背部，弹出提示发现新卡，点击“好的”

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421162505838.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

#### 3、上面的新卡片左滑到新卡片1，单击这个卡片

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421162538853.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

#### 4、进入卡片信息详细页面

##### 钥匙扣卡

![在这里插入图片描述](https://img-blog.csdnimg.cn/202104211627247.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

##### M1空白卡

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421162944336.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

可以发现2张卡除了卡号和卡号异或值不同外，其他数据都一样，之后的例子都以**钥匙扣卡**举例。

下图参考：[https://blog.csdn.net/hiwoshixiaoyu/article/details/104048663](https://blog.csdn.net/hiwoshixiaoyu/article/details/104048663)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421162823909.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

### 二、编译、烧写程序

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421163244266.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

### 三、将钥匙扣卡发在模块上，打开串口，开始测试

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421164153213.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

串口打印 

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421164445603.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

注意 原卡 2扇区0区块数据为

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421164325942.png)

我们放上卡后，进行了数据写入，之后读取到的数据都为DATA1的数据0.0

```c
unsigned char DATA1[16]= {0x12,0x34,0x56,0x78,0x9A,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
// 0x08 就是2扇区0区块（即第9块）
unsigned char addr=0x08;
status = PcdWrite(addr, DATA1);
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021042116450648.png)

此时数据写入完毕后，我们再将钥匙扣卡贴于手机，看看现在手机读取出来的结果

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210421164826967.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

OK，看样子写入成功了，那么到此例程就结束了。

## 测试程序1 RC522_Handle1()

测试程序1，完成0x0F块 验证KEY_A、KEY_B 读 写RFID1 验证KEY_A1、KEY_B1 读 写RFID2

```c
// 测试用 3区块数据
unsigned char RFID1[16]= {0x10,0x20,0x30,0x40,0x50,0x60,0xff,0x07,0x80,0x29,0x01,0x02,0x03,0x04,0x05,0x06};
unsigned char RFID2[16]= {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
// 测试用 3区块密钥
u8 KEY_A1[6]= {0x10,0x20,0x30,0x40,0x50,0x60};
u8 KEY_B1[6]= {0x01,0x02,0x03,0x04,0x05,0x06};
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210422104833763.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
