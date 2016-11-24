/*
*   SW_Marestro_Project_1_Smart Car_IoVA
*
*  File Name : main.c
*  Author    : B W KANG
*  Version   : Ver 2.7
*  Date      : 2016.11.19
*
*/



#include<stm32f10x.h>
#include<stdio.h>
#include"delay.h"
#include"LCD.h"

#include<math.h>
#include<stdlib.h>

////////////////////////////////////////////////////////////////////////////////
//                       Using for printf in UART                             //

// USART_1 & printf 관련 define
#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ascii)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ascii, FILE *f)
#endif 
#define BUFFER_SIZE 200

// BlueTooth 수신 데이터 저장 변수----------------------------------------------
volatile char control_data='S';
u8 RxBuf[BUFFER_SIZE];                    
u8 TxBuf[BUFFER_SIZE]; 
u8 TxInCnt=0;   
u8 TxOutCnt=0;     
u8 RxCnt=0;
volatile u16 a=0;

u16 mot_phs[8]={0x0900,0x0100,0x0500,0x0400,0x0600,0x0200,0x0A00,0x0800};
int Phase;


PUTCHAR_PROTOTYPE
{
    TxBuf[TxInCnt] = ascii;
    if(TxInCnt<BUFFER_SIZE-1) TxInCnt++;
    else TxInCnt = 0;     
    
    //Enable the USART1 Transmit interrupt     
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

    return ascii; 
}
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void Rcc_Initialize(void);
void Gpio_Initialize(void);
void Nvic_Initialize(void);
void UART1_Initialize(void);
void USART1_IRQHandler(void);
//void EXTI_Initialize(void);
//void EXTI15_10_IRQHandler(void);

void TIM2_Initialize(void); //딜레이 
void TIM2_IRQHandler(void);
void Timer2_Delay(u16 Delay);

void UART2_Initialize(void);
void USART2_IRQHandler(void);

void UART3_Initialize(void);
void USART3_IRQHandler(void);

void UART4_Initialize(void);
void USART4_IRQHandler(void);


void TIM3_Initialize(void); //시스템 주기 1000ms
void TIM3_IRQHandler(void);

int system_flag=0;

//void ADC1_Initialize(void);


void position_150(void);
void position_0(void);
void Send_Packet(u8 first,u8 second,u8 Checksum);

/*
u8 F_Sensor_string[10]={0,0,0,0,0,0,0,0,0,0};
u8 F_Sensor_char= 0;
u8 F_Sensor_cnt=0;
*/

u8 F_sensor_char =0;
u8 F_sensor_string[40][10];
int F_sensor_cnt[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int F_sensor_check[40]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int sensing_num =0;

char str[7]= {'0','0','0','0','0','0','\n'};
double F_sensor_double[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int valid_num =0;
double F_sensor_sum=0;
double F_sensor_average =0;

double F_sensor_MAX = 0;
double F_sensor_min = 130;
double average_percent =0;
int valid_percent_num =0;
double valid_sum =0;
double valid_average =0;

/*
u8 B_Sensor_string[10]={0,0,0,0,0,0,0,0,0,0};
u8 B_Sensor_char= 0;
u8 B_Sensor_cnt=0;
*/
u8 B_sensor_char =0;
u8 B_sensor_string[40][10];
int B_sensor_cnt[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int B_sensor_check[40]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int B_sensing_num =0;

char B_str[7]= {'0','0','0','0','0','0','\n'};
double B_sensor_double[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int B_valid_num =0;
double B_sensor_sum=0;
double B_sensor_average =0;

double B_sensor_MAX = 0;
double B_sensor_min = 130;
double B_average_percent =0;
int B_valid_percent_num =0;
double B_valid_sum =0;
double B_valid_average =0;




//-----------------------------------------
int D_time = 500*10;
//-------    삼각함수 관련 변수 ---------//



//---------------------------------------//


void main(void)
{
  Rcc_Initialize();
  Gpio_Initialize();
  Nvic_Initialize();
  Lcd_Initialize();
  UART1_Initialize();
  USART1_IRQHandler();
  //EXTI_Initialize();
  TIM2_Initialize();
  UART2_Initialize();
  UART3_Initialize();
  UART4_Initialize();
  
  TIM3_Initialize( );//시스템 주기 
  
  //ADC1_Initialize( );
  
  
  Delay_ms(100);
  
  //Lcd_Data_String(0x80,"LASER_2 ");  
  //Lcd_Data_String(0xc0,"  TEST  ");
      
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
  TIM_Cmd(TIM3, ENABLE); 
  
  int _t = 0;
  
   while(1)
   {
     
     
     if(system_flag==1)//시스템 주기 1s
     {
       
        //F_check 변수 초기화 
        sensing_num =0;
        valid_num=0;
        F_sensor_sum=0;
        F_sensor_average =0;
        F_sensor_MAX = 0;
        F_sensor_min = 130;
        average_percent =0;
        valid_percent_num =0;
        valid_sum =0;
        valid_average =0;
        
        for(int k=0;k<40;k++)
          {
            F_sensor_check[k]=1;
            F_sensor_cnt[k]=0;
            F_sensor_double[k]=0;
          }
        
        //B_check 변수 초기화
        B_sensing_num =0;
        B_valid_num=0;
        B_sensor_sum=0;
        B_sensor_average =0;
        B_sensor_MAX = 0;
        B_sensor_min = 130;
        B_average_percent =0;
        B_valid_percent_num =0;
        B_valid_sum =0;
        B_valid_average =0;
        
        for(int k=0;k<40;k++)
          {
            B_sensor_check[k]=1;
            B_sensor_cnt[k]=0;
            B_sensor_double[k]=0;
          }
        
        //------------------
          
       
       //printf("%c",control_data);
          
       
       
       if(control_data=='C') 
       {
         printf("Laser");
       }
       
       if(control_data=='S') 
       {
         
         //printf("[%d]\n",_t);
         //_t+=2;
         

           
         
         for(int i=0;i<40;i++)
         {         
           //레이저 거리센서(앞)   
           USART_SendData(USART2,0x64);
           while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
         
           //레이저 거리센서(뒤)
           USART_SendData(USART3,0x64);
           while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
           
           Delay_ms(24);
           //Timer2_Delay(250);
         } 
       
     
         for(int i=0;i<40;i++)
         {
                
           //앞
           if(F_sensor_check[i]==1) // 유효한 경우 string 자리 맞추기 
           {
             
             if(F_sensor_cnt[i]==6)// x.xx
             {
               str[0] = '0';
               str[1] = '0';
               str[2] = F_sensor_string[i][1];
               str[3] = F_sensor_string[i][2];
               str[4] = F_sensor_string[i][3];
               str[5] = F_sensor_string[i][4];
             }
             
             else if(F_sensor_cnt[i]==7)// xx.xx
             {
               str[0] = '0';
               str[1] = F_sensor_string[i][1];
               str[2] = F_sensor_string[i][2];
               str[3] = F_sensor_string[i][3];
               str[4] = F_sensor_string[i][4];
               str[5] = F_sensor_string[i][5];
             }
             
             else if(F_sensor_cnt[i]==8)// xxx.xx
             {
               str[0] = F_sensor_string[i][1];
               str[1] = F_sensor_string[i][2];
               str[2] = F_sensor_string[i][3];
               str[3] = F_sensor_string[i][4];
               str[4] = F_sensor_string[i][5];
               str[5] = F_sensor_string[i][6];
             }
             
             
           }
           
           else if(F_sensor_check[i]==0)//유효하지 않은 경우 = 0
           {
             for(int g=0;g<6;g++)
             {
               str[g]='0';
             }
           }
           /////////////////
           
           //뒤
           if(B_sensor_check[i]==1) // 유효한 경우 string 자리 맞추기 
           {
             
             if(B_sensor_cnt[i]==6)// x.xx
             {
               B_str[0] = '0';
               B_str[1] = '0';
               B_str[2] = B_sensor_string[i][1];
               B_str[3] = B_sensor_string[i][2];
               B_str[4] = B_sensor_string[i][3];
               B_str[5] = B_sensor_string[i][4];
             }
             
             else if(B_sensor_cnt[i]==7)// xx.xx
             {
               B_str[0] = '0';
               B_str[1] = B_sensor_string[i][1];
               B_str[2] = B_sensor_string[i][2];
               B_str[3] = B_sensor_string[i][3];
               B_str[4] = B_sensor_string[i][4];
               B_str[5] = B_sensor_string[i][5];
             }
             
             else if(B_sensor_cnt[i]==8)// xxx.xx
             {
               B_str[0] = B_sensor_string[i][1];
               B_str[1] = B_sensor_string[i][2];
               B_str[2] = B_sensor_string[i][3];
               B_str[3] = B_sensor_string[i][4];
               B_str[4] = B_sensor_string[i][5];
               B_str[5] = B_sensor_string[i][6];
             }
             
             
           }
           
           else if(B_sensor_check[i]==0)//유효하지 않은 경우 = 0
           {
             for(int g=0;g<6;g++)
             {
               B_str[g]='0';
             }
           }
           
           /////////////////

            //앞 
             char* temp = str;
             F_sensor_double[i] =atof(temp); // string to double 
             
             F_sensor_MAX =(F_sensor_MAX>F_sensor_double[i])?F_sensor_MAX:F_sensor_double[i];
            
             if(F_sensor_double[i]>0)
             {
               F_sensor_min = (F_sensor_min<F_sensor_double[i])?F_sensor_min:F_sensor_double[i];
             }
             
             F_sensor_sum += F_sensor_double[i];
             
             
           //뒤
             char* B_temp = B_str;
             B_sensor_double[i] =atof(B_temp); // string to double 
             
             B_sensor_sum += B_sensor_double[i];
            
             //printf("%lf ", F_sensor_double[i]);
             //Delay_ms(5);
                       
         }
         
         F_sensor_average = F_sensor_sum /(double)valid_num;
          B_sensor_average = B_sensor_sum /(double)B_valid_num;
         
         //printf(" %d ",valid_num);
         
         
         if(valid_num>0)
         {
           printf("( %.2f)",F_sensor_average);
         }
         
         if(B_valid_num>0)
         {
           printf("{ %.2f}",B_sensor_average);
         }
         
          /*
         
         //printf("MAX: %lf\n",F_sensor_MAX);
         //printf("min: %lf\n",F_sensor_min);
         
         average_percent = (F_sensor_average-F_sensor_min)*100/(F_sensor_MAX-F_sensor_min);
         
         //printf("AVR per: %lf\n",average_percent);
         
         for(int i=0;i<40;i++)
         {
           if(F_sensor_double[i]>0)
           {
             if(  fabs((  (F_sensor_double[i]-F_sensor_min)*100/(F_sensor_MAX-F_sensor_min)) - average_percent) < 10 )
             {
               valid_percent_num++;
               valid_sum += F_sensor_double[i];
             }
           }
         }
         
         valid_average= valid_sum/valid_percent_num;
         
         if(valid_percent_num>0)
         {
          //printf("( %lf )\n",valid_average);
         }
         
         else if(valid_percent_num==0)
         {
           //printf("( 0 )\n");
         }
         
         */
                 
       }
       
       else if(control_data=='R')
       {
         
         //레이저 거리센서(앞)   
        USART_SendData(USART2,0x64);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
       
     /*     
     Send_Packet(0x03,0x03,0xD1);  Timer2_Delay(5000);
     
     Send_Packet(0x8C,0x02,0x49);  Timer2_Delay(D_time);
     
     Send_Packet(0x59,0x02,0x7C);  Timer2_Delay(D_time);
     
     Send_Packet(0x40,0x02,0x95);  Timer2_Delay(D_time);
     
     Send_Packet(0x32,0x02,0xA3);  Timer2_Delay(D_time);
     
     Send_Packet(0x29,0x02,0xAC);  Timer2_Delay(D_time);
     
     Send_Packet(0x22,0x02,0xB3);  Timer2_Delay(D_time);
     
     Send_Packet(0x1E,0x02,0xB7);  Timer2_Delay(D_time);
     
     Send_Packet(0x1A,0x02,0xBB);  Timer2_Delay(D_time);
     
     Send_Packet(0x17,0x02,0xBE);  Timer2_Delay(D_time);
     
     Send_Packet(0x15,0x02,0xC0);  Timer2_Delay(D_time);
     
     Send_Packet(0x13,0x02,0xC2);  Timer2_Delay(D_time);
     
     Send_Packet(0x12,0x02,0xC3);  Timer2_Delay(D_time);
     
     Send_Packet(0x10,0x02,0xC5);  Timer2_Delay(D_time);
     
     Send_Packet(0x0F,0x02,0xC6);  Timer2_Delay(D_time);
     
     
     Send_Packet(0x0E,0x02,0xC7);  Timer2_Delay(D_time);
     */ 
       }
       
       
       else if(control_data=='L')
       {
          //레이저 거리센서(앞)   
        USART_SendData(USART2,0x64);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
       
       
         
       }
       
       else;
       
       
       system_flag =0;
     }
     /*
     //send 'd'
     
     
     Send_Packet(0x03,0x03,0xD1);  Timer2_Delay(D_time);
     
     Send_Packet(0x8C,0x02,0x49);  Timer2_Delay(D_time);
     
     Send_Packet(0x59,0x02,0x7C);  Timer2_Delay(D_time);
     
     Send_Packet(0x40,0x02,0x95);  Timer2_Delay(D_time);
     
     Send_Packet(0x32,0x02,0xA3);  Timer2_Delay(D_time);
     
     Send_Packet(0x29,0x02,0xAC);  Timer2_Delay(D_time);
     
     Send_Packet(0x22,0x02,0xB3);  Timer2_Delay(D_time);
     
     Send_Packet(0x1E,0x02,0xB7);  Timer2_Delay(D_time);
     
     Send_Packet(0x1A,0x02,0xBB);  Timer2_Delay(D_time);
     
     Send_Packet(0x17,0x02,0xBE);  Timer2_Delay(D_time);
     
     Send_Packet(0x15,0x02,0xC0);  Timer2_Delay(D_time);
     
     Send_Packet(0x13,0x02,0xC2);  Timer2_Delay(D_time);
     
     Send_Packet(0x12,0x02,0xC3);  Timer2_Delay(D_time);
     
     Send_Packet(0x10,0x02,0xC5);  Timer2_Delay(D_time);
     
     Send_Packet(0x0F,0x02,0xC6);  Timer2_Delay(D_time);
     
     Send_Packet(0x0E,0x02,0xC7);  Timer2_Delay(D_time);
     
     //레이저 거리센서(앞)
     
     USART_SendData(USART2,0x64);
     while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
     
     
     //레이저 거리센서(뒤)
     USART_SendData(USART3,0x64);
     while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
     
     */
     
   
   }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                           RCC Setting                                //
void Rcc_Initialize(void)
{
  ErrorStatus HSEStartUpStatus;
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_ON);
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if (HSEStartUpStatus == SUCCESS)
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);//HCLK = SYSCLK
    
    RCC_PCLK2Config(RCC_HCLK_Div1); // 72Mhz
    RCC_PCLK1Config(RCC_HCLK_Div4); // 72/4 x2 = 36Mhz
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //PLL 설정8Mhz*9=72Mhz
    RCC_PLLCmd(ENABLE);
    
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);
  }
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE); //GPIO E 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); //GPIO C
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); //GPIO B  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //GPIO A 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//블루투스 UART1 clock허용.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);// 타이머2 클럭허용
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);// 레이저 센서(앞) UART2 clock 허용 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);// 레이저 센서(뒤) UART3 clock 허용
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);// 서보모터  UART4 clock 허용
 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);// 타이머3 클럭허용
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);// ADC1클럭허용
  
 
  
}


//                                                                      //
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                          GPIO Setting                                //
	
void Gpio_Initialize(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   
   
   /*
   //LCD Rs E
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;//핀 8번 9번 추가
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    //LCD
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;// 핀 1~7추가
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOE, &GPIO_InitStructure);

   //부저.
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //핀 7
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
   */
   
   //블루투스 TX
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//핀 9추가 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   //블루투스 RX
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//핀 10추가 
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   /*
   //버튼
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//핀 10,11,12번 추가 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
   */

    //레이저 센서(앞) TX 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //레이저 센서(앞) RX
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   //레이저 센서(뒤) TX 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    //레이저 센서(뒤) RX
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
   
   
   /*
   //모터 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11; //핀 8,9,10,11
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
   
    //ADC1  A포트 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;// 핀 0  
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // Analog Input mode. 
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   */
   
   //서보모터  TX 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
   

}
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                          NVIC Setting                                //
void Nvic_Initialize(void)
{ 
   NVIC_InitTypeDef NVIC_InitStructure;
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
   
   //USART1_IRQ enable
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn ;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   /*
   // EXTI15_10_IRQ enable
   NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn ;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   */
   
   
    //TIM2_IRQn enable
   NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //enable the TIM2 Interrupt
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
    //USART2_IRQn enable
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //enable the USART2 Interrupt
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   //USART3_IRQn enable
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; //enable the USART3 Interrupt
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   //USART4_IRQn enable
   NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //enable the USART4 Interrupt
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   
   // Tim3_IRQ enable
   NVIC_InitStructure.NVIC_IRQChannel =  TIM3_IRQn ;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   
   
}
//                                                                      //
//////////////////////////////////////////////////////////////////////////  


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             UART1 Setting                            //

void UART1_Initialize(void){ //PCLK2를 사용한다. 
  
   USART_InitTypeDef USART_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
   
   
   USART_InitStructure.USART_BaudRate = 9600; 
   // baud rate= 데이터 통신에서 직렬 전송의 변조 속도를 1초간에 전송되는 신호의 수로 나타낸 값.
   
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   //8비트 또는 9비트로 설정 할 수있다. 
   
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   //하나의 단어(word)의 끝을 표시하기 위해 최후에 부가하는 2비트(1 1.5 2 로 설정 가능)
   
   USART_InitStructure.USART_Parity = USART_Parity_No;
   //1”의 비트의 개수가 짝수인지 홀수인지를 결정

   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   // 수신측에서 수신불가 상태인 경우 송신측에서 데이터 전송하지 않도록 하고 다시 수신가능 상태가 되었을 때만 데이터 전송하는 방식. 
   
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   //USART_CR1 레지스터에서 bit3 TE Transmitter enable 과 Bit 2 RE Receiver enable 를 설정하기 위함. 해당비트위치가 1이 되도록함. 
   
   USART_Init(USART1, &USART_InitStructure);
   //변수선언하고 0으로 초기화. 
   
   USART_Cmd(USART1, ENABLE); // 사용하려면 ENABLE 해줘야됨 
   //USART1 Enable
   
   
   NVIC_InitStructure.NVIC_IRQChannel = 37;  //USART1_IRQChannel
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   
}
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             UART1 Handler                            //


void USART1_IRQHandler(void)
//UART1으로 수신된 값을 읽는 것. 
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)// 비트에 0이 아닐 경우에 데이터가 수신.
    {         
       
        control_data  = USART_ReceiveData(USART1); //0이 아닐 경우에 데이터를 읽는다. control_data= 버퍼에 값을 저장하는 역할을 하는 변수.
        
        //Buffer룰 구현해야하는 이유.
        
        //수신시간에비해 처리시간이 길어서 인터럽트 발생부분에서는 버퍼에 단순히 저장만하고 처리는 다른곳에서 한다.
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); 
        // pending bit를 clear.(안해주면 인터럽트가 처리되었는지 알수없고 다시 인터럽트가 발생한것으로 인지해서 계속 핸들러 호출) 
       
    }
    
    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)//비트에 0이 아닐 경우에 데이터가 송신 
    {         
     
        USART_SendData(USART1,TxBuf[TxOutCnt]);
        
        if(TxOutCnt<BUFFER_SIZE-1) TxOutCnt++; // Txoutcount가 buffer size 보다 작으면 +1씩 샌다. 
        else TxOutCnt = 0;      
            
        if(TxOutCnt == TxInCnt)// Txoutcount가 Txincount가 되면 tx 끄고 RX 킨다. 
        {
          USART_ITConfig(USART1, USART_IT_TXE, DISABLE); 
          USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        }         
    }        
   
}



//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             UART2 Handler                            //

void USART2_IRQHandler(void)
{
   if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
   {
      USART_ClearITPendingBit(USART2, USART_IT_RXNE);
      //printf("%c",USART_ReceiveData(USART2));
      
      F_sensor_char = USART_ReceiveData(USART2);
      
      
      if(F_sensor_char !=0x0a)
      {
        F_sensor_string[sensing_num][F_sensor_cnt[sensing_num]] =F_sensor_char;
        F_sensor_cnt[sensing_num]++;
      }
      
      if(F_sensor_char ==0x0a)
      {
       
        if(F_sensor_string[sensing_num][1]=='0')
        {
          F_sensor_check[sensing_num] = 0;
        }
        
        else if((F_sensor_string[sensing_num][1]=='1')&&(F_sensor_string[sensing_num][2]=='3')&&(F_sensor_string[sensing_num][3]=='0'))
        {
          F_sensor_check[sensing_num] = 0;
        }
        
        else
        {
          F_sensor_check[sensing_num] = 1;
          valid_num++;
        }
        
        
       
        sensing_num++;
        
      }
      
      /*--------------------------------------------
      F_Sensor_char = USART_ReceiveData(USART2);
      
      if(F_Sensor_char != 0x0a)
      {
        F_Sensor_string[F_Sensor_cnt] = F_Sensor_char; 
        F_Sensor_cnt++;
      }
      
      if(F_Sensor_char == 0x0a)
      {
        printf("(");
        
        for(int i=0;i<F_Sensor_cnt-1;i++)
        {
          printf("%c",F_Sensor_string[i]);
        }    
        
        printf(")");
   
        F_Sensor_cnt=0;
      }
      */
   }
}


//                                                                      //
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             EXTI Settings                            //
/*

void EXTI_Initialize(void)
{
   EXTI_InitTypeDef EXTI_InitStructure;
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
   //GPIO C 10,11,12 

   GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10);
   EXTI_InitStructure.EXTI_Line = EXTI_Line10;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource11);
   EXTI_InitStructure.EXTI_Line = EXTI_Line11;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);
   
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);
   EXTI_InitStructure.EXTI_Line = EXTI_Line12;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);
   
}
*/
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             EXTI Handler                             //
/*
void EXTI15_10_IRQHandler(void) //10.11.12.
{
   if(EXTI_GetITStatus(EXTI_Line10) != RESET) {
 
     
     
     GPIOC -> ODR =0x0080;   //Buser on

     
     
     
     
     Delay_ms(100);
     GPIOC -> ODR =0x0000;   //Buser off
   
     Delay_ms(500);
     
     
        
    EXTI_ClearITPendingBit(EXTI_Line10);
        
   }
   if(EXTI_GetITStatus(EXTI_Line11) != RESET) {

   
     GPIOC -> ODR =0x0080;   //Buser on
     

     
     
     Delay_ms(100);
     GPIOC -> ODR =0x0000;   //Buser off
   
     Delay_ms(500);

      EXTI_ClearITPendingBit(EXTI_Line11);
   
   }
   if(EXTI_GetITStatus(EXTI_Line12) != RESET) {

     
     GPIOC -> ODR =0x0080;   //Buser on
    

     
     Delay_ms(100);
     GPIOC -> ODR =0x0000;   //Buser off
   
     Delay_ms(500);
     
  
      EXTI_ClearITPendingBit(EXTI_Line12);
   }

   
} 
*/
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             TIM_2 Settings                           //


void TIM2_Initialize(void) //0.1ms
{   
   
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

   TIM_TimeBaseStructure.TIM_Period = 99; //1~65535  
   TIM_TimeBaseStructure.TIM_Prescaler = 35;     
   //시간 계산 법 = 36MHZ 공급받는다.36000,000  x 1/period  x 1/prescaler  [1200, 300=   10ms]
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  
   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); //TIM enable
   TIM_Cmd(TIM2, DISABLE); //TIM2 enable

}

//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             TIM_2 Handler                            //



void TIM2_IRQHandler(void) //per 1s
{
 
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);               // 타이머 리셋
  a++;                                      //타이머 카운트 증가시킨다.
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);         //인터럽트 리셋
}

//                                                                      //
//////////////////////////////////////////////////////////////////////////



//(Packet Delay) TIM2 Delay 함수------------------------------------------------

void Timer2_Delay(u16 Delay)
{
   

  a=0;
   TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);
   TIM_Cmd(TIM2, ENABLE);
 
   while(1)
   {
     if(a>Delay) break;
      //printf("h\n");
   }
 
   TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
   TIM_Cmd(TIM2, DISABLE);

}

//------------------------------------------------------------------------------



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            UART2 Setting                             //


void UART2_Initialize(void)//레이저 거리센서(앞)
{
 
   USART_InitTypeDef USART_InitStructure;
   USART_InitStructure.USART_BaudRate = 19200 ;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode =USART_Mode_Rx |USART_Mode_Tx;
   USART_Init(USART2, &USART_InitStructure);
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
   USART_Cmd(USART2, ENABLE);  
}
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            UART3 Setting                             //


void UART3_Initialize(void)//레이저 거리센서(뒤)
{
 
   USART_InitTypeDef USART_InitStructure;
   USART_InitStructure.USART_BaudRate = 19200 ;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode =USART_Mode_Rx |USART_Mode_Tx;
   USART_Init(USART3, &USART_InitStructure);
   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
   USART_Cmd(USART3, ENABLE);  
}
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             UART3 Handler                            //

void USART3_IRQHandler(void)
{
   if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
   {
      USART_ClearITPendingBit(USART3, USART_IT_RXNE);
      //printf("%c",USART_ReceiveData(USART3)); 
      
      B_sensor_char = USART_ReceiveData(USART3);
      
      if(B_sensor_char != 0x0a)
      {
        B_sensor_string[B_sensing_num][B_sensor_cnt[B_sensing_num]] =B_sensor_char;
        B_sensor_cnt[B_sensing_num]++;
      }
      
      if(B_sensor_char ==0x0a)
      {
        if(B_sensor_string[B_sensing_num][1]=='0')
        {
          B_sensor_check[B_sensing_num] = 0;
        }
        
         else if((B_sensor_string[B_sensing_num][1]=='1')&&(B_sensor_string[B_sensing_num][2]=='3')&&(B_sensor_string[B_sensing_num][3]=='0'))
        {
          B_sensor_check[B_sensing_num] = 0;
        }
        
        else
        {
          B_sensor_check[B_sensing_num] = 1;
          B_valid_num++;
        }
        
         B_sensing_num++;
      }
      /*
      B_Sensor_char = USART_ReceiveData(USART3);
      
      if(B_Sensor_char != 0x0a)
      {
        B_Sensor_string[B_Sensor_cnt] = B_Sensor_char; 
        B_Sensor_cnt++;
      }
      
      if(B_Sensor_char == 0x0a)
      {
        printf("{");
        
        for(int i=0;i<B_Sensor_cnt-1;i++)
        {
          printf("%c",B_Sensor_string[i]);
          
        }    
        

        //Lcd_Data_String(0xc0,Lcd_string);
        B_Sensor_cnt=0;
      
      
      }
      
      */
   }
}


//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            UART4 Setting                             //


void UART4_Initialize(void)//레이저 거리센서(뒤)
{
 
    USART_InitTypeDef USART_InitStructure;
   USART_InitStructure.USART_BaudRate = 1000000 ;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode =USART_Mode_Tx;
   USART_Init(UART4, &USART_InitStructure);
   USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
   USART_Cmd(UART4, ENABLE);  
}
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                             UART4 Handler                            //

void USART4_IRQHandler(void)
{
   
}


//                                                                      //
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            TIM3 Setting                              //

void TIM3_Initialize(void)// 시스템 주기 1000ms

{

   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

   TIM_TimeBaseStructure.TIM_Period = 12000-1; //1~65535  
   TIM_TimeBaseStructure.TIM_Prescaler = 3000-1;     
   //시간 계산 법 = 36MHZ 공급받는다.36000,000  x 1/period  x 1/prescaler  [1200, 300=   10ms]
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  
   TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE); //TIM3 enable
   TIM_Cmd(TIM3, DISABLE); //TIM3 enable
   
  
} 

//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            TIM3 Handler                              //

void TIM3_IRQHandler(void)//시스템 주기 

{

  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  
  system_flag=1;
 

} 

//                                                                      //
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//                            ADC1 Settins                              //

/*
void ADC1_Initialize(void)
{
   ADC_InitTypeDef ADC_InitStructure;

   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
   //ADC를 독립모드로 사용. single mode 
   ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
   //비트를 0으로 만들면 DISABLE, 비트를 1로 만들면 ENABLE
   //ENABLE하게 되면 채널들을 스캔하게된다. 각각채널에서 conversion을 수행한다. 
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   //0으로하면 ADC변환을 한번만 수행, 1로 설정하면 반복해서 수행.
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   //regular채널에 대한 conversion을 시작하도록하는 trigger를 지정 할 수 있는 것. 
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
   //16비트의 데이터 레지스터에 어떻게 저장할 것인가?  
   ADC_InitStructure.ADC_NbrOfChannel = 1; 
   //변환할 채널수 14개. 본인이 사용하는 ADC만큼 설정해줍니다.
   ADC_Init(ADC1, &ADC_InitStructure);
   //ADC_TempSensorVrefintCmd(ENABLE); //온도센서. 

   
   ADC_Cmd(ADC1, ENABLE); //Enable   ADC1 
   
   
   ADC_ResetCalibration(ADC1);//측정 오차를 줄일 수있다. 
   while(ADC_GetResetCalibrationStatus(ADC1));//0으로 바뀔때까지 기다림. 

   ADC_StartCalibration(ADC1);
   while(ADC_GetCalibrationStatus(ADC1)); 
  
  
}
*/
//                                                                      //
//////////////////////////////////////////////////////////////////////////


void position_150(void)
{
   //전체를  512속도로 150도 에 위치 시킨다.
  //조립전에 한번 해준다. 
  

  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //ID
  USART_SendData(UART4,0xFE);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //LENGTH 
  USART_SendData(UART4,0x07);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //INSTRUCTION   DATA_WRITE
  USART_SendData(UART4,0x03);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  GOAL_POSITON
  USART_SendData(UART4,0x1E);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  
  USART_SendData(UART4,0x01);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  //PARAMETERS
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  //PARAMETERS
  USART_SendData(UART4,0x01);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);

  
  //CHECKSUM
  USART_SendData(UART4,0xD9);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

//------------------------------------------------------------------------------

void position_0(void)
{
   //전체를  512속도로 0도 에 위치 시킨다.
  //조립전에 한번 해준다. 
  

  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //ID
  USART_SendData(UART4,0xFE);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //LENGTH 
  USART_SendData(UART4,0x07);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //INSTRUCTION   DATA_WRITE
  USART_SendData(UART4,0x03);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  GOAL_POSITON
  USART_SendData(UART4,0x1E);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS
  USART_SendData(UART4,0x00);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  
  USART_SendData(UART4,0x00);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  //PARAMETERS
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  //PARAMETERS
  USART_SendData(UART4,0x01);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);

  
  //CHECKSUM
  USART_SendData(UART4,0xD9);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

//-----------------------------------------------------------------------------

void Send_Packet(u8 first,u8 second,u8 Checksum)
{
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //ID
  USART_SendData(UART4,0xFE);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //LENGTH 
  USART_SendData(UART4,0x07);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //INSTRUCTION   DATA_WRITE
  USART_SendData(UART4,0x03);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  GOAL_POSITON
  USART_SendData(UART4,0x1E);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS
  USART_SendData(UART4,first);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS  
  USART_SendData(UART4,second);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //PARAMETERS
  USART_SendData(UART4,0xFF);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  //PARAMETERS
  USART_SendData(UART4,0x03);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
  //CHECKSUM
  USART_SendData(UART4,Checksum);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}