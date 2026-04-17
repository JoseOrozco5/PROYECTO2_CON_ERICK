/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "bitmaps.h"
//musica
#include <stdio.h>
#include <string.h>
#include "math.h"
#include "pitches.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	float x, y;												//posicion
	float x_base;											//centro de formacion
	int vivo;												//1 alien vivo, 0 alien muerto
	int tipo;												//tipo de alien (de los 4 sprites)
	int w, h;												//width y height
	int frame;												//animacion
	float t;												//tiempo/anngulo para seno
	float amplitud;											//q tan rapido se mueve el alien
	float frecuencia; 										//q tan rapido oscila
}Alien;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM_FREQ 84000000
//#define ARR 100
#define TIM_ARR_VAL 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
extern const uint16_t fondo[];
extern const uint16_t espacio[];
extern const uint16_t inicio[];
uint8_t accion[25];
uint8_t indx = 0;
uint8_t accionLista = 0;
uint8_t rxByte;

uint8_t accion2[25];
uint8_t indx2 = 0;
uint8_t accion2Lista = 0;
uint8_t rxByte2;

char Score[20];

//-------Despliege de aliens----//
#define MAX_ALIENS 7
Alien Enemigos[MAX_ALIENS];
int frameActual = 0;
uint32_t timerAliens = 0;
uint32_t timerAnimacionAliens = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void playTone(int *tone, int *duration, int *pause, int size);
int presForFrequency(int frequency);
void noTone(void);
void PantallaInicio(void);
void InitEnemigos(int stage);
void MoverAliens(void);
void JogaBonito2(void);
void Ganador_J1(void);
void Ganador_J2(void);
void Marco(void);
void transicion(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int presForFrequency(int frequency)
{
	if (frequency == 0)
	{
		return 0;
	}
	return ((TIM_FREQ / (TIM_ARR_VAL * frequency)) - 1);
}

void playTone(int *tone, int *duration, int *pause, int size)
{
	for (int i = 0; i < size; i++)
	{
		int prescaler = presForFrequency(tone[i]);				//calcular prescaler nuevo
		int dur = duration[i];									//obtener la duracion
		int pauseBetweenTones = 0;
		if (pause != NULL)
			pauseBetweenTones = pause[i] - duration[i];
		__HAL_TIM_SET_PRESCALER(&htim1, prescaler);
		HAL_Delay(dur);
		noTone();
		HAL_Delay(pauseBetweenTones);
	}
}

void playTonePRE(int *tone, int *duration, int *pause, int size)
{
	for (int i = 0; i < size; i++)
	{
		int prescaler = (tone[i]);				//calcular prescaler nuevo
		int dur = duration[i];									//obtener la duracion
		int pauseBetweenTones = 0;
		if (pause != NULL)
			pauseBetweenTones = pause[i] - duration[i];
		__HAL_TIM_SET_PRESCALER(&htim1, prescaler);
		HAL_Delay(dur);
		noTone();
		HAL_Delay(pauseBetweenTones);
	}
}


void noTone(void)
{
	__HAL_TIM_SET_PRESCALER(&htim1, 0);
}

void PantallaInicio(void)
{
	LCD_Bitmap(0, 0, 320, 240, inicio);

	//Texto pantalla de inicio
	LCD_Print("1UP", 10, 5, 1, 0xF800, 0x0000);
	LCD_Print("00", 18, 15, 1, 0xFFFF, 0x0000);
	LCD_Print("HIGH-SCORE", 110, 5, 1, 0xF800, 0x0000);
	LCD_Print("30000", 123, 15, 1, 0xFFFF, 0x0000);
	LCD_Print("2UP", 280, 5, 1, 0xF800, 0x0000);
	LCD_Print("00", 288, 15, 1, 0xFFFF, 0x0000);
	//Logo
	LCD_Bitmap(80, 35, 160, 95, galagazo);
	LCD_Print("1 PLAYER", 90, 140, 2, 0xFFFF, 0x0000);
	LCD_Print("2 PLAYER", 90, 165, 2, 0xFFFF, 0x0000);
	LCD_Print("Jose Orozco - 231093", 95, 205, 1, 0xFFFF, 0x0000);
	LCD_Print("Erick Perez - 23001", 95, 225, 1, 0xFFFF, 0x0000);

}

void JogaBonito2(void)
{
	//LCD_Clear(0x00);
	for (int y = 0; y < 240; y += 15){
		LCD_Bitmap(160, y, 15, 15, tile);
	}
	// 152 - 17
	for (int x = 0; x <=  128; x++) {
		 int anim = (x/10)%3;
		 //Nave lado izquierdo
		 LCD_Sprite(x, 180, 17, 15, navesita, 3, anim, 0, 0, 0x0000);
		 V_line(x - 1, 180, 15, 0x0000);
		 //Nave lado derecho
		 int x2 = 303 - x;
		 LCD_Sprite(x2, 180, 17, 15, navesita, 3, anim, 1, 0, 0x0000);
		 V_line(x2 + 17, 180, 15, 0x0000);
		 HAL_Delay(15);

	}
	for (int var = 128; var >= 0;  var--) {
		 int anim = (var / 10) % 3;
		 //Nave lado izquierdo de regreso
		 LCD_Sprite(var, 180, 17, 15, navesita, 3, anim, 1, 0, 0x0000);
		 V_line(var + 17, 180, 15, 0x0000);
		 //Nave lado derecho de regreso
		 int var2 = 303 - var;
		 LCD_Sprite(var2, 180, 17, 15, navesita, 3, anim, 0, 0, 0x0000);
		 V_line(var2 - 1, 180, 15, 0x0000);
		 HAL_Delay(15);
	}
}

void InitEnemigos(int stage)
{
	int stages = (stage == 1) ? 2 : (stage == 2) ? 3 : 4;
	for(int i = 0; i < MAX_ALIENS; i++){
		Enemigos[i].x_base = 40 + (i * 35);
		Enemigos[i].y = -20;					//aliens vienen de arriba (escondidos)
		Enemigos[i].vivo = 1;
		Enemigos[i].tipo = i % stages;
		Enemigos[i].t = i * 0.5;
		Enemigos[i].amplitud = 25.0;
		Enemigos[i].frecuencia = 0.05;

		switch(Enemigos[i].tipo)
		{
		case 0:								//alien verde
			Enemigos[i].w = 19;
			Enemigos[i].h = 17;
			break;
		case 1:								//alien rojo
			Enemigos[i].w = 17;
			Enemigos[i].h = 16;
			break;
		case 2:								//mosca
			Enemigos[i].w = 17;
			Enemigos[i].h = 14;
			break;
		case 3:								//alien azul
			Enemigos[i].w = 19;
			Enemigos[i].h = 17;
			break;
		}
	}
}

void MoverAliens(void)
{
	for(int i = 0; i < MAX_ALIENS; i++){
		if(Enemigos[i].vivo)
		{
			FillRect((int)Enemigos[i].x, (int)Enemigos[i].y, (int)Enemigos[i].w, (int)Enemigos[i].h, 0x0000); //limpiar rastro
			Enemigos[i].y += 1.5;
			Enemigos[i].t += Enemigos[i].frecuencia;
			Enemigos[i].x = Enemigos[i].x_base + (sin(Enemigos[i].t) * Enemigos[i].amplitud);
			const uint16_t *spriteActual;
			if(Enemigos[i].tipo	== 0)
			{
				spriteActual = alien_verde;
			}
			else if(Enemigos[i].tipo == 1)
			{
				spriteActual = alien_rojo;
			}
			else if(Enemigos[i].tipo == 2)
			{
				spriteActual = mosca;
			}
			else
			{
				spriteActual = alien_azul;
			}
			LCD_Sprite((int)Enemigos[i].x, (int)Enemigos[i].y, (int)Enemigos[i].w, (int)Enemigos[i].h, spriteActual, 8, frameActual, 0, 0, 0x0000);
		}
	}
}

void Marco(void)
{
	Rect(0, 0, 319, 239, 0x6000);
	Rect(1, 1, 317, 237, 0xF800);
	Rect(2, 2, 315, 235, 0xFFFF);
	Rect(3, 3, 313, 233, 0xF800);
}

void Ganador_J1(void)
{
	LCD_Clear(0x0000);
	Marco();

	LCD_Print("1UP", 40, 10, 1, 0xF800, 0x0000);
	sprintf(Score, "SCORE: %05d", 49480); // Ejemplo de score
	//LCD_Print(Score, 35, 22, 1, 0x07FF, 0x0000); cuando ya este funcionando el juego usar esta
	LCD_Print("123218", 35, 22, 1, 0x07FF, 0x0000);

	LCD_Print("HIGH SCORE", 110, 10, 1, 0xF800, 0x0000);
	//LCD_Print(Score, 35, 22, 1, 0x07FF, 0x0000); cuando ya este funcionando el juego usar esta
	LCD_Print("123218", 130, 22, 1, 0x07FF, 0x0000);

	LCD_Print("PLAYER 1 WINS", 60, 75, 2, 0xF800, 0x0000);

	LCD_Print("TOP 5", 135, 150, 1, 0xFD20, 0x0000);
	LCD_Print("SCORE", 110, 165, 1, 0x07FF, 0x0000);
	LCD_Print("NAME", 185, 165, 1, 0x07FF, 0x0000);

	LCD_Print("1ST  49480   JO", 80, 180, 1, 0xFFFF, 0x0000);
	LCD_Print("2ND  20000   EP", 80, 192, 1, 0xFFFF, 0x0000);
}

void Ganador_J2(void)
{
	LCD_Clear(0x0000);
	Marco();

	LCD_Print("1UP", 40, 10, 1, 0xF800, 0x0000);
	sprintf(Score, "SCORE: %05d", 49480); // Ejemplo de score
	//LCD_Print(Score, 35, 22, 1, 0x07FF, 0x0000); cuando ya este funcionando el juego usar esta
	LCD_Print("123218", 35, 22, 1, 0x07FF, 0x0000);

	LCD_Print("HIGH SCORE", 110, 10, 1, 0xF800, 0x0000);
	//LCD_Print(Score, 35, 22, 1, 0x07FF, 0x0000); cuando ya este funcionando el juego usar esta
	LCD_Print("123218", 130, 22, 1, 0x07FF, 0x0000);

	LCD_Print("PLAYER 2 WINS", 60, 75, 2, 0xF800, 0x0000);

	LCD_Print("TOP 5", 135, 150, 1, 0xFD20, 0x0000);
	LCD_Print("SCORE", 110, 165, 1, 0x07FF, 0x0000);
	LCD_Print("NAME", 185, 165, 1, 0x07FF, 0x0000);

	LCD_Print("1ST  49480   JO", 80, 180, 1, 0xFFFF, 0x0000);
	LCD_Print("2ND  68929   EP", 80, 192, 1, 0xFFFF, 0x0000);
}

void transicion(void)
{
	int ancho = 20;
	for (int i = 0; i < 320; i+= ancho){
		FillRect(i, 0, ancho, 240, 0x0000);
		HAL_Delay(15);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &rxByte, 1);
  HAL_UART_Receive_IT(&huart6, &rxByte2, 1);
  HAL_UART_Transmit(&huart2, (uint8_t*)"Sistema listo\r\n", 15, 1000);
  LCD_Init();
  LCD_Clear(0x00);
  InitEnemigos(1);
  //PantallaInicio();
  //Ganador_J1();
  //Ganador_J2();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  uint32_t tick = HAL_GetTick();
	  if(tick - timerAliens >= 30)						//los aliens se actualizan cada 30 ms, lo q da 33FPS
	  {
	 	  MoverAliens();
		  timerAliens = tick;
	  }
	  if(tick - timerAnimacionAliens >= 200)
	  {
		  frameActual = (frameActual + 1) % 8;
		  timerAnimacionAliens = tick;

	  }

	  if (accionLista){
		  accionLista = 0;

		  HAL_UART_Transmit(&huart2, (uint8_t*)"Control 1: ", 11, 1000);
		  HAL_UART_Transmit(&huart2, accion, strlen((char*)accion), 1000);
		  HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 1000);

		  if (strcmp((char*)accion, "Izquierda") == 0){}
		  else if (strcmp((char*)accion, "Izquierda_off") == 0){}
		  else if (strcmp((char*)accion, "Derecha") == 0){}
		  else if (strcmp((char*)accion, "Derecha_off") == 0){}
		  else if (strcmp((char*)accion, "Accion A") == 0) {}
		  else if (strcmp((char*)accion, "Accion A_off") == 0){}
		  memset(accion, 0, sizeof(accion));
		  indx = 0;
	  }

	  if(accion2Lista){
		  accion2Lista = 0;

		  HAL_UART_Transmit(&huart2, (uint8_t*)"Control 2: ", 11, 1000);
		  HAL_UART_Transmit(&huart2, accion2, strlen((char*)accion2), 1000);
		  HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 1000);

		  if (strcmp((char*)accion2, "Izquierda") == 0){}
		  else if (strcmp((char*)accion2, "Izquierda_off") == 0){}
		  else if (strcmp((char*)accion2, "Derecha") == 0){}
		  else if (strcmp((char*)accion2, "Derecha_off") == 0){}
		  else if (strcmp((char*)accion2, "Accion A") == 0){}
		  else if (strcmp((char*)accion2, "Accion A_off") == 0){}
		  memset(accion2, 0, sizeof(accion2));
		  indx2 = 0;
	  }
	  /*
	  if (accionLista && strcmp((char*)accion, "Accion A") == 0) {
	      accionLista = 0;
	      transicion();  // Tu cortina negra
	      LCD_Clear(0x0000);
	      // Aquí puedes poner un flag como "bool jugando = true;"
	  }
	  */
	  //JogaBonito2();
	  /*
	  for (int x = 0; x < 100 - 69; x++){
		  int anim = (x/10) % 3;
		  LCD_Sprite(x, 150, 69, 68, otra, 3, anim, 0, 0);
		  V_line(x-1, 50, 68,  0x00);
		  HAL_Delay(15);
	  }

	  for (int var = 100 - 69; var > 0; var--){
		  int anim = (var/10) % 3;
		  LCD_Sprite(var, 150, 69, 68, otra, 3, anim, 1, 0);
		  V_line(var + 69, 150, 68, 0x00);
		  HAL_Delay(150);
	  }

	  for (int y = 0; y < 240 - 144; y++){
		  int alien1 = (y/10)%9;
		  LCD_Sprite(50, y, 17, 144, malo1, 9, alien1, 0, 1);
		  V_line(50, y-1, 144,  0x00);
		  HAL_Delay(15);
	  }
	  for (int x = 0; x < 152-17; x++) {
	  		int anim = (x/10)%3;
	  		// anim 0 1 2 3
	  		LCD_Sprite(x, 180, 17, 15, navesita, 3, anim, 0, 0, 0x0000);
	  		V_line(x - 1, 180, 15, 0x0000);
	  		HAL_Delay(15);

	  	}
	  for (int var = 152 - 17; var > 0;  var--) {
	  		int anim = (var / 10) % 3;
	  		LCD_Sprite(var, 180, 17, 15, navesita, 3, anim, 1, 0, 0x0000);
	  		V_line(var + 17, 180, 15, 0x0000);
	  		HAL_Delay(15);
	  	}
	  	*/
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin SD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	 if (huart->Instance == USART1){
		 if (rxByte == '\n' || indx >= 24){
			 accion[indx] = '\0';
			 accionLista = 1;
		 }
		 else if (rxByte != '\r'){
			 accion[indx] = rxByte;
			 indx++;
		 }
		 HAL_UART_Receive_IT(&huart1, &rxByte, 1);
	 }

	 if (huart->Instance == USART6){
		 if (rxByte2 == '\n' || indx2 >= 24){
			 accion2[indx2] = '\0';
			 accion2Lista = 1;
		 }
		 else if (rxByte2 != '\r'){
			 accion2[indx2] = rxByte2;
			 indx2++;
		 }
		 HAL_UART_Receive_IT(&huart6, &rxByte2, 1);
	 }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
