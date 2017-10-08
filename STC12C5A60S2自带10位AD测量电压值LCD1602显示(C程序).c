/*---------------------------------------------------------------------------------------------------------------------------*/ 
/*--************************���ܡ�ADת����Һ����ʾ��**************************--*/
/*--************************оƬ����STC12C5A60S2��******************************--*/
/*--************************Һ������LCD1602��***********************************--*/
/*--************************ADC�ܽţ���P1.0��P1.7 ��***************************--*/
/*--************************��ⷶΧ����0.00��4.99V��***************************--*/
/*---------------------------------------------------------------------------------------------------------------------------*/ 
#include "reg52.h"                                                   
#include "intrins.h"  
                                                                                                                           
typedef  unsigned char uchar;                                               
typedef  unsigned int uint;   
#define  _Nop() _nop_()

/*------------------------����ΪLCD1602��ʾģ�鶨��-----------------------*/
unsigned char data_char_table[]= {"0123456789ABCDEF"};			//LCD����
unsigned char Lcd_Dis1_table[] = {"Position:No.    "};			//��һ����ʾ���
unsigned char pos_char_table[] = {"             D  "};			//      ��ʾλ��
unsigned char Lcd_Dis2_table[] = {"Voltage :      V"};			//�ڶ�����ʾ���
unsigned char num_char_table[] = {"         9A.CD V"};			//      ��ʾλ��


sbit lcd_rs_port = P2^7; 						//����LCD���ƶ˿�,����Ӳ������
sbit lcd_rw_port = P2^6;										
sbit lcd_en_port = P2^4;
#define lcd_data_port P0

void lcd_delay(uchar ms);									//LCD1602 ��ʱ
void lcd_busy_wait(); 										//LCD1602 æ�ȴ�
void lcd_command_write(uint command); 						//LCD1602 ������д��
void lcd_system_reset();										//LCD1602 ��ʼ��
void lcd_char_write(uint x_pos,y_pos,lcd_dat); 					//LCD1602 �ַ�д��
void lcd_bad_check(); 										//LCD1602 ������
void Num_to_Disp(uchar i, uint Num);							//��ʾ���ݴ���
void LcdDisp(uchar j, uint num);								//Һ����ʾ����

/*------------------------����ΪADC��Ӧ�Ĵ�����ʼ�����˿ڶ���-------------*/                           
/***** ������ADC��ص����⹦�ܼĴ��� *****/ 
                        
sfr  ADC_CONTR =  0xBC;                            //ADC���ƼĴ���                    
sfr  ADC_RES  =  0xBD;                             //ADC hight 8-bit result register 
sfr  ADC_RESL  =  0xBE;                            //ADC low 2-bit result register 
sfr  P1ASF     =  0x9D;                            //P1�ڹ��ܿ��ƼĴ���P1ASF

/************������Ӧ����λ***************/ 
#define  ADC_POWER          0x80            //ADC��Դ����λ��0���رգ�1����
#define  ADC_FLAG           0x10            //ADC������־λ
#define  ADC_START          0x08            //ADC��������λ 
#define  ADC_SPEEDLL        0x00            //540 clocks___________ѡ��ת���ٶ�

/*------------------------����Ϊ��غ�������------------------------------*/ 
void InitADC();													//ADC��ʼ��
uint GetADCResult(uchar ch); 																					
void Delay(uint n); 											//��ʱ����
void delay_1ms(uchar x);

/*-------------------------------- ������ --------------------------------*/                                              
void main() 
{
		uchar i;
		lcd_system_reset(); 					  	//LCD1602 ��ʼ��
		lcd_bad_check(); 						  	//LCD1602 ������
        InitADC();                              	//��ʼ��ADC���⹦�ܼĴ���

        while (1) 
        { 	
			i = 0;
			while(i < 7) 
			{ 		 
           		LcdDisp(i, GetADCResult(i));      //Һ��1602��ʾ�����ѹֵ��P1.0 - P1.7��
				Delay(1000);
				i++;
			}			 
        }  
} 

/*-------------------------------- ADC ȡֵ ------------------------------*/ 
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
                                           
/*---------------------------- ��ʼ��ADC���⹦�ܼĴ��� -------------------*/ 

void InitADC( ) 
{ 
          P1ASF = P1 | 0x3f;                //Set  P1.0 - P1.5 as analog input port 
          ADC_RES  = 0;                  //Clear previous result 
		  ADC_RESL = 0; 
          ADC_CONTR = ADC_POWER | ADC_SPEEDLL ; 
          Delay(20);                      //ADC power-on delay and Start A/D conversion 
} 
/*---------------------------- LCD1602��Ӧ���� ---------------------------*/ 

///////////////����ΪLCD��ʾ���ݴ���/////////////////
void Num_to_Disp(uchar i, uint Num)
{
	float NUM;
	int xx, yy, zz;

	NUM = (Num * 5/ 1024.0);	//���㹫ʽ��10-bit A/D Conversion Result = 1024 x (Vin / Vcc)
	xx = (int)NUM;
	yy = (int)((NUM - (float)(xx)) * 10);
	zz = (int)((NUM - (float)(xx)) * 100)%10;  

	num_char_table[9] = data_char_table[xx / 10];		//��ѹֵʮλ
	num_char_table[10]= data_char_table[xx % 10];		//��ѹֵ��λ
	num_char_table[12]= data_char_table[yy];			//��ѹֵС�����һλ
	num_char_table[13]= data_char_table[zz];			//��ѹֵС�������λ?

	pos_char_table[13]= data_char_table[i];			//��ǰADC�ӿ�
}
//////////////////����ΪLCD��ʾ////////////////////
void LcdDisp(uchar j, uint num)
{
	uint i=0;

	for (i=0;i<16;i++) 
	{
		lcd_char_write(i,0,Lcd_Dis1_table[i]);
		lcd_char_write(i,1,Lcd_Dis2_table[i]);  		//��ʾ��� 
	}
	Num_to_Disp(j, num);
	lcd_char_write(13,0,pos_char_table[13]);
	for(i = 9; i < 14; i++)
	{
		lcd_char_write(i,1,num_char_table[i]);  		//��ʾ��ѹ
	}											 
	delay_1ms(100);
}
//////////////������LCD1602��������////////////////

void lcd_delay(uchar ms) /*LCD1602 ��ʱ*/
{
    uchar j;
    while(ms--){
        for(j=0;j<250;j++)
            {;}
        }   
}

void lcd_busy_wait() /*LCD1602 æ�ȴ�*/
{
    lcd_rs_port = 0;
    lcd_rw_port = 1;
    lcd_en_port = 1;
    lcd_data_port = 0xff;
    while (lcd_data_port&0x80);
    lcd_en_port = 0; 

}

void lcd_command_write(uint command) /*LCD1602 ������д��*/
{
    lcd_busy_wait();
    lcd_rs_port = 0;
    lcd_rw_port = 0;
    lcd_en_port = 0;
    lcd_data_port = command;
    lcd_en_port = 1;
    lcd_en_port = 0;     
}

void lcd_system_reset() /*LCD1602 ��ʼ��*/
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

void lcd_char_write(uint x_pos,y_pos,lcd_dat) /*LCD1602 �ַ�д��*/
{
    x_pos &= 0x0f; /* Xλ�÷�Χ 0~15 */
    y_pos &= 0x01; /* Yλ�÷�Χ 0~ 1 */
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
void lcd_bad_check() /*LCD1602 ������*/
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
//////////////////������LCD1602��������////////////////
/*-----------------------------    ��ʱ����    ---------------------------*/                    
void Delay(uint n) 										
{ 
          uint x; 

          while (n--) 
           { 
                     x = 500; 
                     while (x--); 
           } 
}  
/*1MSΪ��λ����ʱ����*/
void delay_1ms(uchar x)
{
    uchar j;
while(x--)
{
        for(j=0;j<125;j++)
            {;}
    }   
}  
