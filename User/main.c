/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0  + STM32 固件库例程
  *********************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 全系列开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "./adc/bsp_adc.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./beep/bsp_beep.h" 
#include "./FatFs_Test/FatFs_test.h"
#include "math.h"
/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t Test_Task_Handle = NULL;/* LED任务句柄 */
static TaskHandle_t Alarm_Task_Handle = NULL;/* KEY任务句柄 */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY任务句柄 */
static TaskHandle_t ADC_Task_Handle = NULL;/* KEY任务句柄 */
static TaskHandle_t Time_Task_Handle = NULL;/* KEY任务句柄 */
/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */
FIL fnew;													/* 文件对象 */
UINT fnum;            					  /* 文件成功读写数量 */
FATFS fs;													/* FatFs文件系统对象 */
FRESULT res_sd;                /* 文件操作结果 */
static char dispBuff[50];	
static char Buff[50];	
uint32_t ADC_ConvertedValueLocal; 
uint16_t A[400];
float value;
float value1;
static char Time[20];	
int h=0,m=0,s=0;

/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
extern uint32_t ADC_ConvertedValue;
extern  SD_CardInfo SDCardInfo;

#define  QUEUE_LEN    1   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */
/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void Test_Task(void* pvParameters);/* Test_Task任务实现 */
static void Alarm_Task(void* pvParameters);/* KEY_Task任务实现 */
static void KEY_Task(void* pvParameters);/* KEY_Task任务实现 */
static void BSP_Init(void);/* 用于初始化板载相关资源 */
static void LCD_config(void);
static void ADC_Task(void* pvParameters);
static void Time_Task(void* pvParameters);/* KEY_Task任务实现 */
QueueHandle_t Test_Queue =NULL;
/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  /* 开发板硬件初始化 */
  BSP_Init();
  
  
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{

  
  taskENTER_CRITICAL();           //进入临界区
	
	  /* 创建Test_Queue */
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
    
  /* 创建Test_Task任务 */
   xTaskCreate((TaskFunction_t )Test_Task, /* 任务入口函数 */
                        (const char*    )"Test_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Test_Task_Handle);/* 任务控制块指针 */

	  /* 创建Test_Task任务 */
   xTaskCreate((TaskFunction_t )Alarm_Task, /* 任务入口函数 */
                        (const char*    )"Alarm_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Alarm_Task_Handle);/* 任务控制块指针 */

												
  /* 创建KEY_Task任务 */
  xTaskCreate((TaskFunction_t )KEY_Task,  /* 任务入口函数 */
                        (const char*    )"KEY_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )5, /* 任务的优先级 */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 


  xTaskCreate((TaskFunction_t )ADC_Task,  /* 任务入口函数 */
                        (const char*    )"ADC_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )7, /* 任务的优先级 */
                        (TaskHandle_t*  )&ADC_Task_Handle);/* 任务控制块指针 */ 
												
		  /* 创建Test_Task任务 */
   xTaskCreate((TaskFunction_t )Time_Task, /* 任务入口函数 */
                        (const char*    )"Time_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )6,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Time_Task_Handle);/* 任务控制块指针 */
  
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}



/**********************************************************************
  * @ 函数名  ： Test_Task
  * @ 功能说明： Test_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Test_Task(void* parameter)
{	
  
//  // 局部变量，用于保存转换计算后的电压值 	 
//  uint32_t ADC_ConvertedValueLocal; 	

  LCD_config();           	//lcd屏幕初始化//
  f_mount(&fs,"0:",1);	    //文件系统挂起-SD、、

  while (1)
  {
		//消息队列接收函数
    xQueueReceive( Test_Queue,    /* 消息队列的句柄 */
                             &value1,      /* 发送的消息内容 */
                             portMAX_DELAY); /* 等待时间 一直等 */
//    ADC_ConvertedValueLocal =(ADC_ConvertedValue * 825) >> 10; 

    printf("%.2f   ",value1); 
    printf("%dh%dm%ds\r\n",h,m,s);  //时间参数

		sprintf(dispBuff,"            %.2f V",value1);    //数据类型转换
	
		sprintf(Buff,"\r\n%.2f    ",value1);
		
		sprintf(Time,"            %dh%dm%ds",h,m,s);
		
		LCD_ClearLine(LINE(7));      //lcd清屏，第七行
  	ILI9341_DispStringLine_EN_CH(LINE(7),dispBuff);	  //显示第七行
		LCD_ClearLine(LINE(9));
		ILI9341_DispStringLine_EN_CH(LINE(9),"            开机时长");
		LCD_ClearLine(LINE(10));
		ILI9341_DispStringLine_EN_CH(LINE(10),Time);	
	
		res_sd=f_open(&fnew, "0:k.txt",FA_OPEN_ALWAYS | FA_WRITE);	//
	  f_lseek (&fnew, f_size(&fnew));				
 		f_write(&fnew,Buff,sizeof(Buff),&fnum);
		f_write(&fnew,Time,sizeof(Time),&fnum);
		f_close(&fnew);                                                //文件操作-SD
		
    vTaskDelay(500);   /* 延时500个tick */       
  }

		
}
static void Alarm_Task(void* parameter)
{	
  while (1)
  {
  	if(value1<14)
    {
//      BEEP(BEEP_ON);		
			LED_RED;
			LCD_ClearLine(LINE(15));
			ILI9341_DispStringLine_EN_CH(LINE(15),"     电压过低，请及时处理！");
		}

		if(value1>=16)
    {
//			BEEP(BEEP_ON);
			LED_RED;
      LCD_ClearLine(LINE(15));			
			ILI9341_DispStringLine_EN_CH(LINE(15),"     电压过高，请及时处理！");
		}

    vTaskDelay(500);   /* 延时500个tick */
//		BEEP(BEEP_OFF);
		LED_GREEN;
		LCD_ClearLine(LINE(15));	
		ILI9341_DispStringLine_EN_CH(LINE(15),"           电压值正常");
  }

		
}
		


/**********************************************************************
  * @ 函数名  ： Test_Task
  * @ 功能说明： Test_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void KEY_Task(void* parameter)
{	
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )
    {/* K1 被按下 */
      printf("挂起LED任务！\n");
      vTaskSuspend(Alarm_Task_Handle);/* 挂起 */
      printf("挂起LED任务成功！\n");
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {/* K2 被按下 */
      printf("恢复LED任务！\n");
      vTaskResume(Alarm_Task_Handle);/* 恢复 */
      printf("恢复LED任务成功！\n");
    }
    vTaskDelay(50);/* 延时20个tick */
  }
}

static void ADC_Task(void* parameter)
{	
	int t;
  while (1)
  {
		
    A[t]=(ADC_ConvertedValue * 825) >> 10;
		
		++t;
		
		if(t==100)
		{
			int i;
			u32 sum1=0;
			u32 sum2=0;
			for(i=0;i<100;i++)
			{
				sum1+=(A[i]-1630)*(A[i]-1630);
				sum2=(sqrt(sum1));			
			}
			ADC_ConvertedValueLocal=sum2;
			value=ADC_ConvertedValueLocal;
			value=value/100;
			
			xQueueSend( Test_Queue, /* 消息队列的句柄 */
                  &value,/* 发送的消息内容 */
                            0 );        /* 等待时间 0 */
			t=0;
		}
    vTaskDelay(1);   /* 延时500个tick */
  }
}
static void Time_Task(void* parameter)    //软件定时器
{	

  while (1)
  {
		
		vTaskDelay(1000);
		s++;

		if(s==60)
		{
			m++;
			s=0;
		}	
		if(m==60)
		{
			h++;
			m=0;
		}
		}
  }

/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
 	ILI9341_Init();  
		
	/* LED 初始化 */
	LED_GPIO_Config();
	
	/* 串口初始化	*/
	USART_Config();
  
  /* 按键初始化	*/
  Key_GPIO_Config();
  
	// ADC 初始化
	ADCx_Init();
	
	//蜂鸣器初始化
	BEEP_GPIO_Config();

	
}


/*用于测试各种液晶的函数*/
void LCD_config(void)
{

	ILI9341_GramScan ( 6 );	
	LCD_SetFont(&Font8x16);
	LCD_SetColors(WHITE,BLACK);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
	ILI9341_DispStringLine_EN_CH(LINE(3),"          湖北师范大学");
  ILI9341_DispStringLine_EN_CH(LINE(6),"           电压有效值");


}



/********************************END OF FILE****************************/
