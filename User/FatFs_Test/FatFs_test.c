/**
  ******************************************************************************
  *                              头文件
  ******************************************************************************
  */
#include "./FatFs_Test/FatFs_test.h"

/*
*************************************************************************
*                               变量
*************************************************************************
*/
//FATFS fs;													/* FatFs文件系统对象 */
//FIL fnew;													/* 文件对象 */
//FRESULT res_sd;                /* 文件操作结果 */
//UINT fnum;            					  /* 文件成功读写数量 */
//BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
////BYTE dispBuff[100];
//BYTE WriteBuffer[]=  /* 写缓冲区*/     
//"欢迎使用野火STM32 开发板 今天是个好日子，新建文件系统测试文件\r\n";  

extern  SD_CardInfo SDCardInfo;

/**
  ******************************************************************
  * @brief   文件系统初始化
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  ******************************************************************
  */ 
//void FileSystem_Init(void)
//{
//	//在外部SPI Flash挂载文件系统，文件系统挂载时会对SPI设备初始化
//	res_sd = f_mount(&fs,"0:",1);
//	
////	printf("容量=%lld",SDCardInfo.CardCapacity/1024/1024);
///*----------------------- 格式化测试 ---------------------------*/  
//	/* 如果没有文件系统就格式化创建创建文件系统 */
//	if(res_sd == FR_NO_FILESYSTEM)
//	{
//		printf("》SD卡还没有文件系统，即将进行格式化...\r\n");
//    /* 格式化 */
//		res_sd=f_mkfs("0:",0,0);							
//		
//		if(res_sd == FR_OK)
//		{
//			printf("》SD卡已成功格式化文件系统。\r\n");
//      /* 格式化后，先取消挂载 */
//			res_sd = f_mount(NULL,"0:",1);			
//      /* 重新挂载	*/			
//			res_sd = f_mount(&fs,"0:",1);
//		}
//		else
//		{
//			LED_RED;
//			printf("《《格式化失败。》》\r\n");
//			while(1);
//		}
//	}
//  else if(res_sd!=FR_OK)
//  {
//    printf("！！SD卡挂载文件系统失败。(%d)\r\n",res_sd);
//    printf("！！可能原因：SD卡初始化不成功。\r\n");
//		while(1);
//  }
//  else
//  {
//    printf("》文件系统挂载成功，可以进行读写测试\r\n");
//  }
//}

/**
  ******************************************************************
  * @brief   文件系统读写测试
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  ******************************************************************
  */ 
void FileSystem_Test(void)
{
	
//	res_sd = f_mount(&fs,"0:",1);
//	
//	res_sd = f_open(&fnew, "0:FatFs.txt",FA_CREATE_ALWAYS | FA_WRITE );

//	res_sd=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);

//	f_close(&fnew);	
//  
//	f_mount(NULL,"0:",1);

}


