/**********************************************************
                       康威电子
										 
功能：stm32f103rbt6控制，MCP41xx写入值(0-255  ->  0-R)
			参数输入0-255，具体电阻值可根据R总计算
			显示：12864cog
接口：控制接口请参照MCP41xx.h  按键接口请参照key.h
时间：2015/11/10
版本：1.0
作者：康威电子
其他：

更多电子需求，请到淘宝店，康威电子竭诚为您服务 ^_^
店铺：kvdz.taobao.com

**********************************************************/

#include "MCP41xx.h"

void mcp_delay(uint n)
{
	n=n*110;
	while(n--);
}
void MCP410XXInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13);				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC,(GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13));
	PCout(11)=0;
	P_MCP_CS1=1;
	P_MCP_CS2=1;
}
void MCP41xx_1writedata(uchar dat1)		//调整数字电位器1
{
  uchar i,command=0x11;
	MCP41xx_SPI_CS2_H();//cs=1; 
	MCP41xx_SPI_CS1_H();//cs=1; 
	MCP41xx_SPI_CLK_L();//sck=1;
	MCP41xx_SPI_CS1_L();//cs=0;

 	mcp_delay(10);

	for(i=0;i<8;i++)          //写命令
	{ 
		if(command & 0x80)
		{
			MCP41xx_SPI_DAT_H();//si=1;	
		}
		else
		{
			MCP41xx_SPI_DAT_L();//si=0;
		}
		mcp_delay(10);
		MCP41xx_SPI_CLK_L();//sck=1;//sck=1;
		mcp_delay(10);
		MCP41xx_SPI_CLK_H();//sck=0;sck=0;
		mcp_delay(10); 

		command=command<<1;
	}

     
	for(i=0;i<8;i++)          //写数据
	{ 
		
		if(dat1 & 0x80)
		{
			MCP41xx_SPI_DAT_H();//si=1;	
		}
		else
		{
			MCP41xx_SPI_DAT_L();//si=0;
		}
		mcp_delay(10);
		MCP41xx_SPI_CLK_L();//sck=1;//sck=1;
		mcp_delay(10);
		MCP41xx_SPI_CLK_H();//sck=0;sck=0;
		mcp_delay(10);
		dat1=dat1<<1;
	}

	MCP41xx_SPI_CS1_H();//cs=1; //cs=1;

	mcp_delay(10);//_nop_();
}

void MCP41xx_2writedata(uchar dat2)		//调整数字电位器2
{
  uchar i,command=0x11;
	
	MCP41xx_SPI_CS1_H();//cs=1; 
	MCP41xx_SPI_CS2_H();//cs=1; 
	MCP41xx_SPI_CLK_L();//sck=1;
	MCP41xx_SPI_CS2_L();//cs=0;

 	mcp_delay(10);

	for(i=0;i<8;i++)          //写命令
	{ 
		if(command & 0x80)
		{
			MCP41xx_SPI_DAT_H();//si=1;	
		}
		else
		{
			MCP41xx_SPI_DAT_L();//si=0;
		}
		mcp_delay(10);
		MCP41xx_SPI_CLK_L();//sck=1;//sck=1;
		mcp_delay(10);
		MCP41xx_SPI_CLK_H();//sck=0;sck=0;
		mcp_delay(10); 

		command=command<<1;
	}

     
	for(i=0;i<8;i++)          //写数据
	{ 
		
		if(dat2 & 0x80)
		{
			MCP41xx_SPI_DAT_H();//si=1;	
		}
		else
		{
			MCP41xx_SPI_DAT_L();//si=0;
		}
		mcp_delay(10);
		MCP41xx_SPI_CLK_L();//sck=1;//sck=1;
		mcp_delay(10);
		MCP41xx_SPI_CLK_H();//sck=0;sck=0;
		mcp_delay(10);
		dat2=dat2<<1;
	}

	MCP41xx_SPI_CS2_H();//cs=1; //cs=1;

	mcp_delay(10);//_nop_();
}










