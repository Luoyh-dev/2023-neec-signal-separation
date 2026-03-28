 #include "adc.h"
 #include "delay.h"
#include "stm32f10x_adc.h"
	   		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  Adc1_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_71Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
		ADC_ExternalTrigConvCmd(ADC1,ENABLE);      //ADC外部触发使能

}


u16 Get_ADC_Value(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);//第一个参数ADC1，第二个参数是选择通道几，第三个是计算参数的精致度的，在库函数里面这个值是最大的，也是算的最精准的，如果不需要这么精准可以选择中间一点的数，具体在.ADC文件里面有
	for(t=0;t<times;t++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //这个是软件使能打开
		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));  //然后上面说到每次转换要打开EOC
		temp_val+=ADC_GetConversionValue(ADC1);       //这个是获取在ADC寄存器保存的通道值
		delay_ms(5); 
	}
	return temp_val/times;             //返回ADC的值这里除以times时间是因为转换需要一定的时间。
 
 
}


void AD_Init(void)
{  
	GPIO_InitTypeDef  GPIO_Initstruct;
	ADC_InitTypeDef   ADC_Initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE); //打开PA AFOI ADC的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //AD的分频因子确保不允许超过14Mhz 72/6=12
	
    GPIO_Initstruct.GPIO_Mode= GPIO_Mode_AIN;  //模拟输入  ADC专属模式
	GPIO_Initstruct.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;
	GPIO_Initstruct.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_Initstruct);//初始化PA2
	
	
	ADC_Initstruct.ADC_Mode=ADC_Mode_Independent;  //模式选择，这里是独立就是不用ADC1和ADC2同时进行
	ADC_Initstruct.ADC_DataAlign=ADC_DataAlign_Right;  //寄存器数据对齐方式右
	ADC_Initstruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//触发方式禁止外部，使用软件触发
	ADC_Initstruct.ADC_ContinuousConvMode=DISABLE;  //关闭连续转换所以是单次转换模式
    ADC_Initstruct.ADC_ScanConvMode=DISABLE; //非多通道扫面模式 单通道模式
    ADC_Initstruct.ADC_NbrOfChannel=1;  //常规则序列通道数量
	ADC_Init(ADC1,&ADC_Initstruct);
	
	ADC_Cmd(ADC1,ENABLE);  //ADC使能校准给ADC上电
	
	//以下的为软件触发程序
	ADC_ResetCalibration(ADC1);//复位指定的ADC的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);//获取ADC复位校准寄存器的状态  判断是否空循环秒如果是那么跳出然后获取标志位校准
	
	ADC_StartCalibration(ADC1);//开始指定ADC的校准状态
    while(ADC_GetCalibrationStatus(ADC1) == SET);//开始获取指定ADC的校准程序
 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);//使能指定的ADC的软件转换启动功能
	
}
 
// Get_ADC_Value(u8 ch,u8 times)
//{
//	u32 temp_val=0;
//	u8 t;
//	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
//	for(t=0;t<times;t++)
//	{
//		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));
//		temp_val+=ADC_GetConversionValue(ADC1);
//		delay_ms(5);
//	}
//	return temp_val/times;
//}












