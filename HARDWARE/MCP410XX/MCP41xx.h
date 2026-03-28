#ifndef __MCP41xx_h
#define __MCP41xx_h
#include "stm32f10x.h"
#include "sys.h"
#define uint unsigned int
#define uchar unsigned char

#define DATA_CD 0x11
			
#define P_MCP_CS1	 PCout(8)
#define P_MCP_CS2	 PCout(9)
#define P_MCP_CLK	 PCout(13)
#define P_MCP_DAT	 PCout(12)

#define MCP41xx_SPI_CLK_H()     P_MCP_CLK=1	//GPIOB->BSRR = MCP41xx_SPI_CLK_BIT;		   
#define MCP41xx_SPI_CLK_L()     P_MCP_CLK=0	//GPIOB->BRR =  MCP41xx_SPI_CLK_BIT;

#define MCP41xx_SPI_DAT_H()     P_MCP_DAT=1	//GPIOB->BSRR = MCP41xx_SPI_DAT_BIT;
#define MCP41xx_SPI_DAT_L()     P_MCP_DAT=0	//GPIOB->BRR =  MCP41xx_SPI_DAT_BIT;

#define MCP41xx_SPI_CS1_H()      P_MCP_CS1=1	//GPIOB->BSRR = MCP41xx_SPI_CS1_BIT;
#define MCP41xx_SPI_CS1_L()      P_MCP_CS1=0	//GPIOB->BRR =  MCP41xx_SPI_CS1_BIT;

#define MCP41xx_SPI_CS2_H()      P_MCP_CS2=1	//GPIOB->BSRR = MCP41xx_SPI_CS2_BIT;
#define MCP41xx_SPI_CS2_L()      P_MCP_CS2=0	//GPIOB->BRR =  MCP41xx_SPI_CS2_BIT;

void MCP410XXInit(void);
void mcp_delay(uint n);
void MCP41xx_1writedata(uchar dat1);
void MCP41xx_2writedata(uchar dat2);

#endif
