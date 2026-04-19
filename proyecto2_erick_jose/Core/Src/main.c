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
typedef enum{
	ESTADO_MENU,
	ESTADO_JUG1,
	ESTADO_JUG2,
	ESTADO_VICTORIA
}GameState;

typedef struct{
	float x, y;
	int w, h;
	int vivo;
	int frame;
}Nave;

typedef struct{
	float x, y;
	int activo;
}Bala;

typedef enum{
	ENTRANDO,
	FORMACION//,
	//INACTIVO,
	//ENTRADA_SENO,
	//ENTRADA_LOOP,
	//ATAQUE
}EstadoAlien;

typedef struct{
	float x, y;												//posicion
	float x_base, y_base;									//centro de formacion
	int vivo;												//1 alien vivo, 0 alien muerto
	int tipo;												//tipo de alien (de los 4 sprites)
	int w, h;												//width y height
	int frame;												//animacion
	float t;												//tiempo/anngulo para seno
	float amplitud;											//q tan rapido se mueve el alien
	float frecuencia; 										//q tan rapido oscila
	EstadoAlien estado;
}Alien;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM_FREQ 84000000
//#define ARR 100
#define TIM_ARR_VAL 100
#define CMD_IZQ      0x01
#define CMD_IZQ_OFF  0x02
#define CMD_DER      0x03
#define CMD_DER_OFF  0x04
#define CMD_DIS      0x05
#define CMD_DIS_OFF  0x06
#define CMD_SEL		 0x07
#define CMD_SEL_OFF  0x08
#define CMD_ARR      0x09
#define CMD_ARR_OFF  0x0A
#define CMD_ABJ      0x0B
#define CMD_ABJ_OFF  0x0C
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

//-----------Menu-------------//
GameState modoActual = ESTADO_MENU;
int SeleccionMenu = 0;
Nave J1, J2;
uint32_t timerNaves = 0;
static int seleccionPrevia = -1;

//---------Controles---------//
uint8_t rxByte1;


uint8_t rxByte2;

volatile uint8_t nave1_izq = 0;
volatile uint8_t nave1_der = 0;
volatile uint8_t nave1_dis = 0;
volatile uint8_t nave1_sel = 0;
volatile uint8_t nave1_arr = 0;
volatile uint8_t nave1_abj = 0;

volatile uint8_t nave2_izq = 0;
volatile uint8_t nave2_der = 0;
volatile uint8_t nave2_dis = 0;
volatile uint8_t nave2_sel = 0;
volatile uint8_t nave2_arr = 0;
volatile uint8_t nave2_abj = 0;


//-------Puntaje----------------//
char Score[20];

//-------Despliege de aliens----//
#define MAX_ALIENS 8
Alien Enemigos[MAX_ALIENS];
int frameActual = 0;
uint32_t timerAliens = 0;
uint32_t timerAnimacionAliens = 0;
int AlienActual = 0;
uint32_t tiempoSpawn = 0;
static float vaiven = 0;

//---Disparo de naves/aliens---//
#define MAX_BALAS 5
Bala disparo_J1[MAX_BALAS];
Bala disparo_J2[MAX_BALAS];
uint32_t timerBalas = 0;

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
void Dibujar(void);
void InitEnemigos(int stage);
void MoverAliens(void);
void MoverNaves(void);
void InitJuego1(void);
void Disparar(int jugador);
void ActualizarBalas(void);
void InitJuego2(void);
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

void InitJuego1(void)
{
	J1.x = 152;
	J1.y = 210;
	J1.w = 17;
	J1.h = 15;
	J1.vivo = 1;
	InitEnemigos(1);
}

void InitJuego2(void)
{
	//LCD_Clear(0x00);
	J1.x = 70;
	J1.y = 210;
	J1.w = 17;
	J1.h = 15;
	J1.vivo = 1;

	J2.x = 230;
	J2.y = 210;
	J2.w = 17;
	J2.h = 15;
	J2.vivo = 1;
	/*
	for (int y = 0; y < 240; y += 15){
		LCD_Bitmap(160, y, 15, 15, tile);
	}
	*/
	InitEnemigos(1);
}

void InitEnemigos(int stage)
{
	int stages = (stage == 1) ? 2 : (stage == 2) ? 3 : 4;
	int aliensPorTipo = MAX_ALIENS / stages;
	for(int i = 0; i < MAX_ALIENS; i++){
		if(modoActual == ESTADO_JUG2){
			if(i < 4){
				Enemigos[i].x_base = 20 + (i * 30);
			}else{
				Enemigos[i].x_base = 180 + ((i - 4) * 30);
				Enemigos[i].amplitud = 15.0;
			}
		}else{
			Enemigos[i].x_base = 60 + (i * 25);
			Enemigos[i].amplitud = 40.0;
		}
		Enemigos[i].y_base = 80;
		Enemigos[i].x = Enemigos[i].x_base;
		Enemigos[i].y = -20;					//aliens vienen de arriba (escondidos)
		Enemigos[i].vivo = 0;					//spawn secuencial
		Enemigos[i].t = 0;
		Enemigos[i].frecuencia = 0.05;
		Enemigos[i].estado = ENTRANDO;
		Enemigos[i].tipo = i / aliensPorTipo;

		if(Enemigos[i].tipo >= stages)
		{
			Enemigos[i].tipo = stages - 1;
		}

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
	vaiven += 0.02;
	float offsetFormacion = sin(vaiven) * 15;


	for(int i = 0; i < MAX_ALIENS; i++){
		if(Enemigos[i].vivo)
		{
			FillRect((int)Enemigos[i].x, (int)Enemigos[i].y, (int)Enemigos[i].w, (int)Enemigos[i].h, 0x0000);
			int FrameADibujar = frameActual;

			switch(Enemigos[i].estado)
			{
			case ENTRANDO:
				Enemigos[i].y += 1.6;
				//float inicio_alien = (Enemigos[i].tipo == 0) ? 140 : 180;
				float inicio_alien = 140 + (Enemigos[i].tipo * 20);
				float lado = (Enemigos[i].tipo == 0) ? 1.0 : -1.0;
				float progreso = (Enemigos[i].y + 20.0) / (Enemigos[i].y_base + 20);
				if (progreso > 1.0){
					progreso = 1.0;
				}
				float centro_x = inicio_alien + (Enemigos[i].x_base - inicio_alien) * progreso;
				float zigzag = lado * sin(Enemigos[i].y * 0.04) * Enemigos[i].amplitud;
				Enemigos[i].x = centro_x + zigzag;

				if(Enemigos[i].y >= Enemigos[i].y_base)
				{
					Enemigos[i].y = Enemigos[i].y_base;
					Enemigos[i].estado = FORMACION;
				}
				break;
			case FORMACION:
				Enemigos[i].x = Enemigos[i].x_base + offsetFormacion;
				Enemigos[i].y = Enemigos[i].y_base;
				FrameADibujar = frameActual % 2;
				break;
			}


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
			LCD_Sprite((int)Enemigos[i].x, (int)Enemigos[i].y, (int)Enemigos[i].w, (int)Enemigos[i].h, spriteActual, 8, FrameADibujar, 0, 0, 0x0000);
		}
	}
}

void MoverNaves(void)
{
	if(J1.vivo)
	{
		FillRect((int)J1.x, (int)J1.y, J1.w, J1.h, 0x0000);
		 if (nave1_izq && J1.x > 0){
			 J1.x -= 2.5;
		 }
		 if (nave1_der){
			 float limite = (modoActual == ESTADO_JUG2) ? 143 : 303;
			 if(J1.x < limite)
			 {
				 J1.x += 2.5;
			 }
		 }
		 LCD_Sprite((int)J1.x, (int)J1.y, J1.w, J1.h, navesita, 3, frameActual % 3, 0, 0, 0x0000);
	}

	if(modoActual == ESTADO_JUG2 && J2.vivo)
	{
		FillRect((int)J2.x, (int)J2.y, J2.w, J2.h, 0x0000);
		if(nave2_izq && J2.x > 162){
			J2.x -= 2.5;
		}
		if(nave2_der && J2.x < 303){
			J2.x += 2.5;
		}
		LCD_Sprite((int)J2.x, (int)J2.y, J2.w, J2.h, navesita, 3, frameActual % 3, 1, 0, 0x0000);
	}
}

void Marco(void)
{
	Rect(0, 0, 319, 239, 0x6000);
	Rect(1, 1, 317, 237, 0xF800);
	Rect(2, 2, 315, 235, 0xFFFF);
	Rect(3, 3, 313, 233, 0xF800);
}

void Dibujar(void)
{
	if(modoActual == ESTADO_JUG1)
	{
		LCD_Print("1UP", 5, 2, 1, 0xF800, 0x0000);
		LCD_Print("00", 8, 12, 1, 0xFFFF, 0x0000);
		LCD_Print("HIGH SCORE", 240, 2, 1, 0xF800, 0x0000);
		LCD_Print("20000", 260, 12, 1, 0xFFFF, 0x0000);
	}else if(modoActual == ESTADO_JUG2){
		LCD_Print("1UP", 14, 2, 1, 0xF800, 0x0000);
		LCD_Print("00", 18, 12, 1, 0xFFFF, 0x0000);
		LCD_Print("2UP", 300, 2, 1, 0xF800, 0x0000);
		LCD_Print("00", 290, 12, 1, 0xFFFF, 0x0000);
	}
}

void Disparar(int jugador)
{
	Bala* lista = (jugador == 1) ? disparo_J1 : disparo_J2;
	Nave* n = (jugador == 1) ? &J1 : &J2;

	for(int i = 0; i < MAX_BALAS; i++){
		if(!lista[i].activo){
			lista[i].x = n->x + (n->w / 2) - 1;
			lista[i].y = n->y - 5;
			lista[i].activo = 1;
			break;
		}
	}

}

void ActualizarBalas(void)
{
	for(int j = 1; j <= 2; j++){
		Bala* lista = (j == 1) ? disparo_J1 : disparo_J2;
		for(int i = 0; i < MAX_BALAS; i++){
			if(lista[i].activo){
				FillRect((int)lista[i].x, (int)lista[i].y, 5, 10, 0x0000);
				lista[i].y -= 5.0;
				if(lista[i].y < 14){
					lista[i].activo = 0;
				}else{
					LCD_Sprite((int)lista[i].x, (int)lista[i].y, 5, 10, disparo_nave, 2, 0, 0, 0, 0x0000);
				}
			}
		}
	}
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
  HAL_UART_Receive_IT(&huart1, &rxByte1, 1);
  HAL_UART_Receive_IT(&huart6, &rxByte2, 1);
  HAL_UART_Transmit(&huart2, (uint8_t*)"Sistema listo\r\n", 15, 1000);
  LCD_Init();
  LCD_Clear(0x00);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  PantallaInicio();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  uint32_t tick = HAL_GetTick();

	  switch(modoActual)
	  {
	  case ESTADO_MENU:
		  if(nave1_abj)
		  {
		  	  SeleccionMenu = 1;
		  	  nave1_abj = 0;
		  }
		  if(nave1_arr)
		  {
		  	  SeleccionMenu = 0;
		  	  nave1_arr = 0;
		  }
		  int cursor = (SeleccionMenu == 0) ? 140 : 165;
		  if(SeleccionMenu != seleccionPrevia)
		  {
			  LCD_Print(" ", 70, 140, 2, 0x0000, 0x0000);
			  LCD_Print(" ", 70, 165, 2, 0x0000, 0x0000);
			  LCD_Print(">", 70, cursor, 2, 0x07FF, 0x0000);
			  seleccionPrevia = SeleccionMenu;
		  }
		  if(nave1_sel)
		  {
		  	  nave1_sel = 0;
		  	  transicion();
		  	  LCD_Clear(0x0000);
		  	  seleccionPrevia = -1;
		  	  if(SeleccionMenu == 0)
		  	  {
		  		  InitJuego1();
		  		  modoActual = ESTADO_JUG1;
		  	  }else{
		  		  InitJuego2();
		  		  modoActual = ESTADO_JUG2;
		  	  }
		  	  Dibujar();
		  }
		  break;

	  case ESTADO_JUG1:
		  if(tick - timerAliens >= 40)						//los aliens se actualizan cada 30 ms, lo q da 33FPS
		  {
			  MoverAliens();
			  timerAliens = tick;
		  }
		  if(tick - timerNaves >= 20)
		  {
			  MoverNaves();
			  if(nave1_dis){
				  Disparar(1);
				  nave1_dis = 0;
			  }
			  timerNaves = tick;
		  }
		  if(tick - timerBalas >= 15)
		  {
		 	  ActualizarBalas();
		 	  timerBalas = tick;
		  }
		  if(tick - timerAnimacionAliens >= 200)
		  {
			  frameActual = (frameActual + 1) % 8;
			  timerAnimacionAliens = tick;
		  }
		  if(tick - tiempoSpawn >= 800 && AlienActual < MAX_ALIENS)
		  {
			  Enemigos[AlienActual].vivo = 1;
			  Enemigos[AlienActual].y = -20;
			  AlienActual++;
			  tiempoSpawn = tick;
		  }
		  break;
	  case ESTADO_JUG2:
		  if(tick - timerAliens >= 40){
			  MoverAliens();
			  timerAliens = tick;
		  }
		  if(tick - timerNaves >= 20){
			  MoverNaves();
			  if(nave1_dis){
				  Disparar(1);
				  nave1_dis = 0;
			  }
			  if(modoActual == ESTADO_JUG2 && nave2_dis){
				  Disparar(2);
				  nave2_dis = 0;
			  }
			  timerNaves = tick;
		  }
		  if(tick - timerBalas >= 15)
		  {
			  ActualizarBalas();
			  timerBalas = tick;
		  }
		  if(tick - timerAnimacionAliens >= 200)
		  {
			  frameActual = (frameActual + 1) % 8;
			  timerAnimacionAliens = tick;
		  }
		  if(tick - tiempoSpawn >= 800 && AlienActual < MAX_ALIENS)
		  {
			  Enemigos[AlienActual].vivo = 1;
			  Enemigos[AlienActual].y = -20;
			  AlienActual++;
			  tiempoSpawn = tick;
		  }
		  break;
	  case ESTADO_VICTORIA:
		  break;
	  }



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
		 switch (rxByte1) {
		 case CMD_IZQ:
			 nave1_izq = 1;
			 break;
		 case CMD_IZQ_OFF:
			 nave1_izq = 0;
			 break;
		 case CMD_DER:
			 nave1_der = 1;
			 break;
		 case CMD_DER_OFF:
			 nave1_der = 0;
			 break;
		 case CMD_DIS:
			 nave1_dis = 1;
			 break;
		 case CMD_DIS_OFF:
			 nave1_dis = 0;
			 break;
		 case CMD_SEL:
			 nave1_sel = 1;
			 break;
		 case CMD_SEL_OFF:
			 nave1_sel = 0;
			 break;
		 case CMD_ARR:
			 nave1_arr = 1;
			 break;
		 case CMD_ARR_OFF:
			 nave1_arr = 0;
			 break;
		 case CMD_ABJ:
			 nave1_abj = 1;
			 break;
		 case CMD_ABJ_OFF:
			 nave1_abj = 0;
			 break;
		     }
		 HAL_UART_Receive_IT(&huart1, &rxByte1, 1);
	 }

	 if (huart->Instance == USART6){
		 switch (rxByte2) {
		 case CMD_IZQ:
			 nave2_izq = 1;
			 break;
		 case CMD_IZQ_OFF:
			 nave2_izq = 0;
			 break;
		 case CMD_DER:
			 nave2_der = 1;
			 break;
		 case CMD_DER_OFF:
			 nave2_der = 0;
			 break;
		 case CMD_DIS:
			 nave2_dis = 1;
			 break;
		 case CMD_DIS_OFF:
			 nave2_dis = 0;
			 break;
		 case CMD_SEL:
			 nave2_sel = 1;
			 break;
		 case CMD_SEL_OFF:
			 nave2_sel = 0;
			 break;
		 case CMD_ARR:
			 nave2_arr = 1;
			 break;
		 case CMD_ARR_OFF:
			 nave2_arr = 0;
			 break;
		 case CMD_ABJ:
			 nave2_abj = 1;
			 break;
		 case CMD_ABJ_OFF:
			 nave2_abj = 0;
			 break;
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
