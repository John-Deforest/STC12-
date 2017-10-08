/*---------------------------------------------------------------------------------------------------------------------------*/ 
/*--************************功能【AD转换，液晶显示】**************************--*/
/*--************************芯片：【STC12C5A60S2】******************************--*/
/*--************************液晶：【LCD1602】***********************************--*/
/*--************************ADC管脚：【P1.0～P1.7 】***************************--*/
/*--************************检测范围：【0.00～4.99V】***************************--*/
/*---------------------------------------------------------------------------------------------------------------------------*/ 
#include "reg52.h"                                                   
#include "intrins.h"  
                                                                                                                           
typedef  unsigned char uchar;                                               
typedef  unsigned int uint;   
#define  _Nop() _nop_()

/*------------------------以下为LCD1602显示模块定义-----------------------*/
unsigned char data_char_table[]= {"0123456789ABCDEF"};			//LCD数据
unsigned char Lcd_Dis1_table[] = {"Position:No.    "};			//第一行显示框架
unsigned char pos_char_table[] = {"             D  "};			//      显示位置
unsigned char Lcd_Dis2_table[] = {"Voltage :      V"};			//第二行显示框架
unsigned char num_char_table[] = {"         9A.CD V"};			//      显示位置


sbit lcd_rs_port = P2^7; 						//定义LCD控制端口,根据硬件调整
sbit lcd_rw_port = P2^6;										
sbit lcd_en_port = P2^4;
#define lcd_data_port P0

void lcd_delay(uchar ms);									//LCD1602 延时
void lcd_busy_wait(); 										//LCD1602 忙等待
void lcd_command_write(uint command); 						//LCD1602 命令字写入
void lcd_system_reset();										//LCD1602 初始化
void lcd_char_write(uint x_pos,y_pos,lcd_dat); 					//LCD1602 字符写入
void lcd_bad_check(); 										//LCD1602 坏点检查
void Num_to_Disp(uchar i, uint Num);							//显示数据处理
void LcdDisp(uchar j, uint num);								//液晶显示函数

/*------------------------以下为ADC相应寄存器初始化及端口定义-------------*/                           
/***** 定义与ADC相关的特殊功能寄存器 *****/ 
                        
sfr  ADC_CONTR =  0xBC;                            //ADC控制寄存器                    
sfr  ADC_RES  =  0xBD;                             //ADC hight 8-bit result register 
sfr  ADC_RESL  =  0xBE;                            //ADC low 2-bit result register 
sfr  P1ASF     =  0x9D;                            //P1口功能控制寄存器P1ASF

/************定义相应操作位***************/ 
#define  ADC_POWER          0x80            //ADC电源控制位，0：关闭，1：打开
#define  ADC_FLAG           0x10            //ADC结束标志位
#define  ADC_START          0x08            //ADC启动控制位 
#define  ADC_SPEEDLL        0x00            //540 clocks___________选择转换速度

/*------------------------以下为相关函数声明------------------------------*/ 
void InitADC();													//ADC初始化
uint GetADCResult(uchar ch); 																					
void Delay(uint n); 											//延时程序
void delay_1ms(uchar x);

/*-------------------------------- 主函数 --------------------------------*/                                              
void main() 
{
		uchar i;
		lcd_system_reset(); 					  	//LCD1602 初始化
		lcd_bad_check(); 						  	//LCD1602 坏点检查
        InitADC();                              	//初始化ADC特殊功能寄存器

        while (1) 
        { 	
			i = 0;
			while(i < 7) 
			{ 		 
           		LcdDisp(i, GetADCResult(i));      //液晶1602显示输入电压值（P1.0 - P1.7）
				Delay(1000);
				i++;
			}			 
        }  
} 

/*-------------------------------- ADC 取值 ------------------------------*/ 
uint GetADCResult(uchar ch) 
{ 
          ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START; 
          _nop_();                                        //Must wait before inquiry 
          _nop_(); 
          _nop_(); 
          _nop_(); 
          while (!(ADC_CONTR & ADC_FLAG));                //Wait complete flag 
          ADC_CONTR &= ADC_FLAG;                          //Close ADC 

          return (ADC_RES*4 + ADC_RESL);                  //Return ADC result 
} 
                                           
/*---------------------------- 初始化ADC特殊功能寄存器 -------------------*/ 

void InitADC( ) 
{ 
          P1ASF = P1 | 0x3f;                //Set  P1.0 - P1.5 as analog input port 
          ADC_RES  = 0;                  //Clear previous result 
		  ADC_RESL = 0; 
          ADC_CONTR = ADC_POWER | ADC_SPEEDLL ; 
          Delay(20);                      //ADC power-on delay and Start A/D conversion 
} 
/*---------------------------- LCD1602相应函数 ---------------------------*/ 

///////////////以下为LCD显示数据处理/////////////////
void Num_to_Disp(uchar i, uint Num)
{
	float NUM;
	int xx, yy, zz;

	NUM = (Num * 5/ 1024.0);	//计算公式：10-bit A/D Conversion Result = 1024 x (Vin / Vcc)
	xx = (int)NUM;
	yy = (int)((NUM - (float)(xx)) * 10);
	zz = (int)((NUM - (float)(xx)) * 100)%10;  

	num_char_table[9] = data_char_table[xx / 10];		//电压值十位
	num_char_table[10]= data_char_table[xx % 10];		//电压值个位
	num_char_table[12]= data_char_table[yy];			//电压值小数点后一位
	num_char_table[13]= data_char_table[zz];			//电压值小数点后两位?

	pos_char_table[13]= data_char_table[i];			//当前ADC接口
}
//////////////////以下为LCD显示////////////////////
void LcdDisp(uchar j, uint num)
{
	uint i=0;

	for (i=0;i<16;i++) 
	{
		lcd_char_write(i,0,Lcd_Dis1_table[i]);
		lcd_char_write(i,1,Lcd_Dis2_table[i]);  		//显示框架 
	}
	Num_to_Disp(j, num);
	lcd_char_write(13,0,pos_char_table[13]);
	for(i = 9; i < 14; i++)
	{
		lcd_char_write(i,1,num_char_table[i]);  		//显示电压
	}											 
	delay_1ms(100);
}
//////////////以下是LCD1602驱动程序////////////////

void lcd_delay(uchar ms) /*LCD1602 延时*/
{
    uchar j;
    while(ms--){
        for(j=0;j<250;j++)
            {;}
        }   
}

void lcd_busy_wait() /*LCD1602 忙等待*/
{
    lcd_rs_port = 0;
    lcd_rw_port = 1;
    lcd_en_port = 1;
    lcd_data_port = 0xff;
    while (lcd_data_port&0x80);
    lcd_en_port = 0; 

}

void lcd_command_write(uint command) /*LCD1602 命令字写入*/
{
    lcd_busy_wait();
    lcd_rs_port = 0;
    lcd_rw_port = 0;
    lcd_en_port = 0;
    lcd_data_port = command;
    lcd_en_port = 1;
    lcd_en_port = 0;     
}

void lcd_system_reset() /*LCD1602 初始化*/
{
    lcd_delay(20);
    lcd_command_write(0x38);
    lcd_delay(100);
    lcd_command_write(0x38);
    lcd_delay(50);
    lcd_command_write(0x38);
    lcd_delay(10);
    lcd_command_write(0x08);
    lcd_command_write(0x01);
    lcd_command_write(0x06);
    lcd_command_write(0x0c); 
}

void lcd_char_write(uint x_pos,y_pos,lcd_dat) /*LCD1602 字符写入*/
{
    x_pos &= 0x0f; /* X位置范围 0~15 */
    y_pos &= 0x01; /* Y位置范围 0~ 1 */
    if(y_pos==1) x_pos += 0x40;
    x_pos += 0x80;
    lcd_command_write(x_pos);
    lcd_busy_wait();
    lcd_rs_port = 1;
    lcd_rw_port = 0;
    lcd_en_port = 0;
    lcd_data_port = lcd_dat;
    lcd_en_port = 1;
    lcd_en_port = 0; 
}
void lcd_bad_check() /*LCD1602 坏点检查*/
{
    char i,j;
    for(i=0;i<2;i++){
        for(j=0;j<16;j++) {
            lcd_char_write(j,i,0xff);
            }
        }
    lcd_delay(200);
    lcd_delay(200);
	lcd_delay(200);
	lcd_delay(100);
	lcd_delay(200);
    lcd_command_write(0x01); /* clear lcd disp */
}
//////////////////以上是LCD1602驱动程序////////////////
/*-----------------------------    延时程序    ---------------------------*/                    
void Delay(uint n) 										
{ 
          uint x; 

          while (n--) 
           { 
                     x = 500; 
                     while (x--); 
           } 
}  
/*1MS为单位的延时程序*/
void delay_1ms(uchar x)
{
    uchar j;
while(x--)
{
        for(j=0;j<125;j++)
            {;}
    }   
}  
