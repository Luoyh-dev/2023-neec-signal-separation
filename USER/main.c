#include "stm32_config.h"
#include "stdio.h"
#include "adc.h"
#include "usart.h"
#include "stm32_dsp.h"
#include "sys.h"
#include "timer.h"
#include "stdbool.h"
#include "AD9833.h"
#include "lcd_init.h"
#include "lcd.h"

//变量声明
int test_i = 0;
int raw_f_sum = 0;
int avg_raw_f = 0;
int FFT_num = 0;
typedef enum {
    FFT_CHANNEL_1,
    // 其他通道
} EN_FFT_CHANNEL;


#define SAMPLS_NUM        	256
#define FFT_CHANNEL_1 0  // 或者其他适当的值

// 在合适的头文件或全局变量定义中添加以下内容
struct GlobalStruct {
    bool adc_finish_fg;
    // 其他成员
};

struct GlobalStruct global;  // 全局变量

// 或者只是定义一个全局变量
bool adc_finish_fg = false;  // 初始化为false

unsigned short maxMagIndex = 0;

u16 ADC_SourceData[SAMPLS_NUM];  // 一维数组，存储实部或虚部
u32 FFT_SourceData[SAMPLS_NUM];  // 修改数据类型为
u32 FFT_OutData[SAMPLS_NUM/2];		//fft输出序列
u32 FFT_Mag[SAMPLS_NUM/2] ;		//幅频特性序列（序号代表频率分量，值代表幅值大小。由于FFT的频谱结果是关于奈奎斯特频率对称的，所以只计算一半的点即可）

void ADC_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);				//使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;		//管脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;						//模拟输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);								//GPIO组
}


void ADC_TIM3_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure; 

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 时钟分频（预分频器值）和周期值设置为满足 256 kHz 采样频率的配置
    TIM_TimeBaseInitStructure.TIM_Period = 279; // 修改为满足采样频率的周期值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1; // 修改为满足采样频率的预分频器值
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    TIM_Cmd(TIM3, ENABLE);
}

void ADC_DMA_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel					 = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ClearITPendingBit(DMA1_IT_TC1);

	DMA_ITConfig(DMA1_Channel1,DMA1_IT_TC1,ENABLE);
}

void ADC_DMA_Configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)ADC_SourceData;
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize         = SAMPLS_NUM;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_Cmd(DMA1_Channel1, ENABLE);//使能DMA	

	ADC_DMA_NVIC_Configuration();
	
	test_i = (u32)ADC_SourceData;
}

void ADC_Init_Configuration(void)//ADC配置函数
{
	ADC_InitTypeDef  ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);

	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;			
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel       = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  1, ADC_SampleTime_7Cycles5);	//AI_VS_A1
	ADC_DMACmd(ADC1, ENABLE);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


void Adc_Init(void)	
{
	ADC_GPIO_Configuration();
	

	ADC_TIM3_Configuration();

	ADC_DMA_Configuration();

	ADC_Init_Configuration();
}

//ADC_DMA中断服务程序
void DMA1_Channel1_IRQHandler(void)
{
	
	if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		global.adc_finish_fg = true;
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		
	}
	
	
}

int input_raw_f = 0;
void GetPowerMag(void)
{
    signed short lX, lY;
    float X, Y, Mag;
    unsigned short i;
    unsigned long maxMag = 0;
    unsigned long secondMaxMag = 0;
    unsigned short maxIndex = 0;
    unsigned short secondMaxIndex = 0;

    for(i = 0; i < SAMPLS_NUM / 2; i++)//扫描周期
    {
        lX = (FFT_OutData[i] << 16) >> 16;
        lY = (FFT_OutData[i] >> 16);

        X = SAMPLS_NUM * ((float)lX) / 32768;
        Y = SAMPLS_NUM * ((float)lY) / 32768;

        Mag = sqrt(X * X + Y * Y) / SAMPLS_NUM;

        FFT_Mag[i] = (unsigned long)(Mag * 65536);

        if (FFT_Mag[i] > maxMag) {
            secondMaxMag = maxMag;
            secondMaxIndex = maxIndex;
            maxMag = FFT_Mag[i];
            maxIndex = i;
			  
        } else if (FFT_Mag[i] > secondMaxMag) {
            secondMaxMag = FFT_Mag[i];
            secondMaxIndex = i;
			  
        }
		  
		  
    }

//    printf("Maximum FFT_Mag value: %lu at index %u\n", maxMag, maxIndex);
//	printf("波形频率:%u\nKHz", secondMaxIndex);
	 
	 //LCD_ShowIntNum(36,50,FFT_Mag[secondMaxIndex],3,RED,WHITE,16);
	 //delay_ms(500);
	 input_raw_f = FFT_Mag[secondMaxIndex];
	 
	 raw_f_sum += input_raw_f;
	 test_i++;
	 
	 
}



void Get_FFT_Source_Data(EN_FFT_CHANNEL channel_idx)
{
	u16 i;

	for(i=0; i<SAMPLS_NUM; i++)
	{
		FFT_SourceData[i] = ((signed short)ADC_SourceData[i]) << 16;
	}
}

void FFT_test(void)
{
    Get_FFT_Source_Data(FFT_CHANNEL_1);
    cr4_fft_256_stm32(FFT_OutData, FFT_SourceData, SAMPLS_NUM);
    GetPowerMag();
//    for(int i = 0; i < SAMPLS_NUM; i++) {
////        printf("FFT_SourceData[%d]: %lu\n", i, FFT_SourceData[i]);
//    }
//    printf("FFT_Mag values:\n");
//    for(int i = 0; i < SAMPLS_NUM / 2; i++) {
////        printf("FFT_Mag[%d]: %lu\n", i, FFT_Mag[i]);
//    }
}





int main(void)
{
    Adc_Init();
    USARTx_Init(115200); // 初始化串口
    delay_init(72);      // 初始化延时函数
    AD9833_Init();		  	//初始化与DDS连接的硬件接口
	 LCD_Init();//LCD初始化
	 LCD_Fill(0,0,LCD_W,LCD_H,WHITE);//液晶屏初始化
	 
	
	
	
	 //AD9833_WaveSeting(f,0,SIN_WAVE,0 );//1KHz,	频率寄存器0，正弦波输出 ,初相位0 
    AD9833_AmpSet(100);	//设置幅值，幅值最大 255
    while(1)
    {
		 
		 //AD9833_WaveSeting(f,0,SIN_WAVE,0 );
		 
        if(global.adc_finish_fg)
        {
            global.adc_finish_fg = true;
            FFT_test(); 
			   if(test_i >= 100)
				{
					avg_raw_f = raw_f_sum/100;
			      raw_f_sum = 0;
					test_i = 0;
					FFT_num ++;
				}
			  LCD_ShowIntNum(72,30,avg_raw_f,3,RED,WHITE,16);
				LCD_ShowIntNum(36,30,FFT_num,3,RED,WHITE,16);
			  
 //           printf("找到的最大值索引: %u\n", maxMagIndex);
			  //LCD_ShowIntNum(36,30,maxMagIndex,3,RED,WHITE,16);
			for (int i = 0; i < SAMPLS_NUM; i++)
            {
//                printf("adc值[%d]：%d\n",i, ADC_SourceData[i]);
			         //LCD_ShowIntNum(72,30,ADC_SourceData[i],3,RED,WHITE,16);
					   //delay_ms(100);
            }

            // 将标志复位为false，等待下一次DMA传输完成触发FFT计算
            global.adc_finish_fg = false;
        }
    }
}