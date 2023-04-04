/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0  + STM32 �̼�������
  *********************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32 ȫϵ�п����� 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"

/* ������Ӳ��bspͷ�ļ� */
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "./adc/bsp_adc.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./beep/bsp_beep.h" 
#include "./FatFs_Test/FatFs_test.h"
#include "math.h"
/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */
static TaskHandle_t Test_Task_Handle = NULL;/* LED������ */
static TaskHandle_t Alarm_Task_Handle = NULL;/* KEY������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY������ */
static TaskHandle_t ADC_Task_Handle = NULL;/* KEY������ */
static TaskHandle_t Time_Task_Handle = NULL;/* KEY������ */
/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */
FIL fnew;													/* �ļ����� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FRESULT res_sd;                /* �ļ�������� */
static char dispBuff[50];	
static char Buff[50];	
uint32_t ADC_ConvertedValueLocal; 
uint16_t A[400];
float value;
float value1;
static char Time[20];	
int h=0,m=0,s=0;

/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
extern uint32_t ADC_ConvertedValue;
extern  SD_CardInfo SDCardInfo;

#define  QUEUE_LEN    1   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  QUEUE_SIZE   4   /* ������ÿ����Ϣ��С���ֽڣ� */
/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void Test_Task(void* pvParameters);/* Test_Task����ʵ�� */
static void Alarm_Task(void* pvParameters);/* KEY_Task����ʵ�� */
static void KEY_Task(void* pvParameters);/* KEY_Task����ʵ�� */
static void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */
static void LCD_config(void);
static void ADC_Task(void* pvParameters);
static void Time_Task(void* pvParameters);/* KEY_Task����ʵ�� */
QueueHandle_t Test_Queue =NULL;
/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();
  
  
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{

  
  taskENTER_CRITICAL();           //�����ٽ���
	
	  /* ����Test_Queue */
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                            (UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
    
  /* ����Test_Task���� */
   xTaskCreate((TaskFunction_t )Test_Task, /* ������ں��� */
                        (const char*    )"Test_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )3,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Test_Task_Handle);/* ������ƿ�ָ�� */

	  /* ����Test_Task���� */
   xTaskCreate((TaskFunction_t )Alarm_Task, /* ������ں��� */
                        (const char*    )"Alarm_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Alarm_Task_Handle);/* ������ƿ�ָ�� */

												
  /* ����KEY_Task���� */
  xTaskCreate((TaskFunction_t )KEY_Task,  /* ������ں��� */
                        (const char*    )"KEY_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )5, /* ��������ȼ� */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 


  xTaskCreate((TaskFunction_t )ADC_Task,  /* ������ں��� */
                        (const char*    )"ADC_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )7, /* ��������ȼ� */
                        (TaskHandle_t*  )&ADC_Task_Handle);/* ������ƿ�ָ�� */ 
												
		  /* ����Test_Task���� */
   xTaskCreate((TaskFunction_t )Time_Task, /* ������ں��� */
                        (const char*    )"Time_Task",/* �������� */
                        (uint16_t       )512,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )6,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Time_Task_Handle);/* ������ƿ�ָ�� */
  
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� Test_Task
  * @ ����˵���� Test_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void Test_Task(void* parameter)
{	
  
//  // �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
//  uint32_t ADC_ConvertedValueLocal; 	

  LCD_config();           	//lcd��Ļ��ʼ��//
  f_mount(&fs,"0:",1);	    //�ļ�ϵͳ����-SD����

  while (1)
  {
		//��Ϣ���н��պ���
    xQueueReceive( Test_Queue,    /* ��Ϣ���еľ�� */
                             &value1,      /* ���͵���Ϣ���� */
                             portMAX_DELAY); /* �ȴ�ʱ�� һֱ�� */
//    ADC_ConvertedValueLocal =(ADC_ConvertedValue * 825) >> 10; 

    printf("%.2f   ",value1); 
    printf("%dh%dm%ds\r\n",h,m,s);  //ʱ�����

		sprintf(dispBuff,"            %.2f V",value1);    //��������ת��
	
		sprintf(Buff,"\r\n%.2f    ",value1);
		
		sprintf(Time,"            %dh%dm%ds",h,m,s);
		
		LCD_ClearLine(LINE(7));      //lcd������������
  	ILI9341_DispStringLine_EN_CH(LINE(7),dispBuff);	  //��ʾ������
		LCD_ClearLine(LINE(9));
		ILI9341_DispStringLine_EN_CH(LINE(9),"            ����ʱ��");
		LCD_ClearLine(LINE(10));
		ILI9341_DispStringLine_EN_CH(LINE(10),Time);	
	
		res_sd=f_open(&fnew, "0:k.txt",FA_OPEN_ALWAYS | FA_WRITE);	//
	  f_lseek (&fnew, f_size(&fnew));				
 		f_write(&fnew,Buff,sizeof(Buff),&fnum);
		f_write(&fnew,Time,sizeof(Time),&fnum);
		f_close(&fnew);                                                //�ļ�����-SD
		
    vTaskDelay(500);   /* ��ʱ500��tick */       
  }

		
}
static void Alarm_Task(void* parameter)
{	
  while (1)
  {
  	if(value1<14)
    {
//      BEEP(BEEP_ON);		
			RELAY_ON;
			LCD_ClearLine(LINE(15));
			ILI9341_DispStringLine_EN_CH(LINE(15),"     ��ѹ���ͣ��뼰ʱ����");
		}

		if(value1>=16)
    {
//			BEEP(BEEP_ON);
			RELAY_OFF;
      LCD_ClearLine(LINE(15));			
			ILI9341_DispStringLine_EN_CH(LINE(15),"     ��ѹ���ߣ��뼰ʱ����");
		}

    vTaskDelay(500);   /* ��ʱ500��tick */
//		BEEP(BEEP_OFF);
		LCD_ClearLine(LINE(15));	
		ILI9341_DispStringLine_EN_CH(LINE(15),"           ��ѹֵ����");
  }

		
}
		


/**********************************************************************
  * @ ������  �� Test_Task
  * @ ����˵���� Test_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void KEY_Task(void* parameter)
{	
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )
    {/* K1 ������ */
      printf("����LED����\n");
      vTaskSuspend(Alarm_Task_Handle);/* ���� */
      printf("����LED����ɹ���\n");
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {/* K2 ������ */
      printf("�ָ�LED����\n");
      vTaskResume(Alarm_Task_Handle);/* �ָ� */
      printf("�ָ�LED����ɹ���\n");
    }
    vTaskDelay(50);/* ��ʱ20��tick */
  }
}

static void ADC_Task(void* parameter)
{	
  while (1)
  {
    ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3;
		printf("\r\n The current AD value = 0x%04X \r\n", ADC_ConvertedValue); 
		printf("\r\n The current AD value = %f V \r\n",ADC_ConvertedValueLocal); 
    vTaskDelay(1);   /* ��ʱ500��tick */
  }
}
static void Time_Task(void* parameter)    //�����ʱ��
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
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32�ж����ȼ�����Ϊ4����4bit��������ʾ��ռ���ȼ�����ΧΪ��0~15
	 * ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ�
	 * ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ�
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
 	ILI9341_Init();  
		
	/* LED ��ʼ�� */
	LED_GPIO_Config();
	
	/* ���ڳ�ʼ��	*/
	USART_Config();
  
  /* ������ʼ��	*/
  Key_GPIO_Config();
  
	// ADC ��ʼ��
	ADCx_Init();
	
	//��������ʼ��
	BEEP_GPIO_Config();

	
}


/*���ڲ��Ը���Һ���ĺ���*/
void LCD_config(void)
{

	ILI9341_GramScan ( 6 );	
	LCD_SetFont(&Font8x16);
	LCD_SetColors(WHITE,BLACK);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
	ILI9341_DispStringLine_EN_CH(LINE(3),"          ����ʦ����ѧ");
  ILI9341_DispStringLine_EN_CH(LINE(6),"           ��ѹ��Чֵ");


}



/********************************END OF FILE****************************/
