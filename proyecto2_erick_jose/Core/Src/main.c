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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "bitmaps.h"
//musica
#include <stdio.h>
#include <string.h>
#include "math.h"
#include "pitches.h"
//----------SD--------//
#include "fatfs_sd.h"

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
	int explosion;
	int frame_explosion;
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
	int explosion;
	int frame_explosion;
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

//audio
#define SND_MENU       0x01
#define SND_JUEGO      0x02
#define SND_DISPARO    0x03
#define SND_EXPLOSION  0x04
#define SND_NAVE_CAE   0x05
#define SND_VICTORIA   0x06
#define SND_PUNTAJE 0x07
#define SND_ALIEN_ROJO 0x08
#define SND_ALIEN_AZUL 0x09
#define SND_VERDE_F1 0x0a
#define SND_VERDE_F2 0x0b

//-------DEF SD----------------//
//SPI_HandleTypeDef hspi2;
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
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
static uint8_t musicaMenu = 0;
static uint8_t musicaJuego = 0;

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
int aliensVivos = 0;

//---Disparo de naves/aliens---//
#define MAX_BALAS 5
Bala disparo_J1[MAX_BALAS];
Bala disparo_J2[MAX_BALAS];
uint32_t timerBalas = 0;
Bala balas_aliens[MAX_BALAS];
uint32_t timerBalasAliens = 0;
int scoreJ1 = 0, scoreJ2 = 0;
int CausaVictoria = 0;
uint32_t timerExplo = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void PantallaInicio(void);
void Dibujar(void);
void InitEnemigos(int stage);
void MoverAliens(void);
void MoverNaves(void);
void InitJuego1(void);
void Disparar(int jugador);
int ChequearColision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
void ActualizarBalas(void);
void InitJuego2(void);
void Ganador_J1(void);
void Ganador_J2(void);
void Marco(void);
void transicion(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void Musica_Comando(uint8_t cmd) {
    HAL_UART_Transmit(&huart3, &cmd, 1, 100);
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
	J1.explosion = 0;
	J1.frame_explosion = 0;
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
	J1.explosion = 0;
	J1.frame_explosion = 0;

	J2.x = 230;
	J2.y = 210;
	J2.w = 17;
	J2.h = 15;
	J2.vivo = 1;
	J2.explosion = 0;
	J2.frame_explosion = 0;
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
		Enemigos[i].explosion = 0;
		Enemigos[i].frame_explosion = 0;

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
	vaiven += 0.035;
	float offsetFormacion = sin(vaiven) * 30.0;


	for(int i = 0; i < MAX_ALIENS; i++){
		if(Enemigos[i].vivo || Enemigos[i].explosion)
		{
			if(Enemigos[i].explosion){
				FillRect((int)Enemigos[i].x, (int)Enemigos[i].y, 30, 30, 0x0000);
				LCD_Sprite((int)Enemigos[i].x, (int)Enemigos[i].y, 21, 19, explosion_alien, 4, Enemigos[i].frame_explosion, 0, 0, 0x0000);
			}else{
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
}

void MoverNaves(void)
{
	if(J1.vivo || J1.explosion)
	{
		if(J1.explosion){
			FillRect((int)J1.x - 5, (int)J1.y - 5, 34, 34, 0x0000);
			LCD_Sprite((int)J1.x, (int)J1.y, 30, 32, explosion_nave, 4, J1.frame_explosion, 0, 0, 0x0000);
		}else{
			float x_prev = J1.x;
			if(nave1_izq && J1.x > 0){
				J1.x -= 2.5;
			}
			if(nave1_der){
				float limite = (modoActual == ESTADO_JUG2) ? 143 : 303;
				if(J1.x < limite)
				{
					J1.x += 2.5;
				}
			}
			int dx = (int)J1.x - (int)x_prev;
			if(dx > 0){
				FillRect((int)x_prev, (int)J1.y, dx, J1.h, 0x0000);
			}else if(dx < 0){
				FillRect((int)J1.x + J1.w, (int)J1.y, -dx, J1.h, 0x0000);
				LCD_Sprite((int)J1.x, (int)J1.y, J1.w, J1.h, navesita, 3, frameActual % 3, 0, 0, 0x0000);
			}
		}
	}

	if(modoActual == ESTADO_JUG2 && (J2.vivo || J2.explosion))
	{
		if(J2.explosion){
			FillRect((int)J2.x - 5, (int)J2.y - 5, 34, 34, 0x0000);
			LCD_Sprite((int)J2.x, (int)J2.y, 30, 32, explosion_nave, 4, J2.frame_explosion, 0, 1, 0x0000);
		}else{
			float x_prev = J2.x;
			if(nave2_izq && J2.x > 162){
				J2.x -= 2.5;
			}
			if(nave2_der && J2.x < 303){
				J2.x += 2.5;
			}
			int dx = (int)J2.x - (int)x_prev;
			if(dx > 0){
				FillRect((int)x_prev, (int)J2.y, dx, J2.h, 0x0000);
			}else if(dx < 0){
				FillRect((int)J2.x + J2.w, (int)J2.y, -dx, J1.h, 0x0000);
				LCD_Sprite((int)J2.x, (int)J2.y, J2.w, J2.h, navesita, 3, frameActual % 3, 1, 0, 0x0000);
			}
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

int ChequearColision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2)
{
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
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
  MX_SPI2_Init();
  MX_FATFS_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &rxByte1, 1);
  HAL_UART_Receive_IT(&huart6, &rxByte2, 1);
  HAL_UART_Transmit(&huart2, (uint8_t*)"Sistema listo\r\n", 15, 1000);
  LCD_Init();
  LCD_Clear(0x00);

  HAL_Delay(1000); // Dale un segundo completo de vida a la SD

  HAL_UART_Transmit(&huart2, (uint8_t*)"SD: Intentando conectar...\r\n", 28, 1000);

    // Hacemos un bucle de 5 intentos para montar
  int intentos = 0;
  do {
	  fres = f_mount(&fs, "", 1);
	  if (fres != FR_OK) {
		  HAL_UART_Transmit(&huart2, (uint8_t*)".", 1, 1000);
		  HAL_Delay(100);
		  intentos++;
        }
    } while (fres != FR_OK && intentos < 5);

  if (fres == FR_OK) {

	  HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nSD: ¡Montada nítido!\r\n", 25, 1000);

        // Abrimos para escribir el Score inicial
	  fres = f_open(&fil, "SCORE.TXT", FA_WRITE | FA_OPEN_ALWAYS);
	  if (fres == FR_OK) {
            // Nos vamos al final del archivo para no borrar lo anterior
		  f_lseek(&fil, f_size(&fil));
		  f_puts("Partida iniciada...\n", &fil);
		  f_close(&fil);
		  HAL_UART_Transmit(&huart2, (uint8_t*)"SD: Log de inicio guardado.\r\n", 30, 1000);
        }
    } else {
    	sprintf(buffer, "\r\nSD: Fallo definitivo (Error: %d)\r\n", fres);
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 1000);
    }

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
		  if(!musicaMenu){
			  Musica_Comando(0x01);
			  musicaMenu = 1;
			  musicaJuego = 0;
		  }
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
		  	  musicaMenu = 0;
		  	  Musica_Comando(0x02);
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

	  case ESTADO_JUG2:
		  //Movimiento de aliens
		  if(tick - timerAliens >= 50){
			  MoverAliens();
			  timerAliens = tick;
		  }
		  //Movimiento de naves
		  if(tick - timerNaves >= 20){
			  MoverNaves();
			  if(nave1_dis && J1.vivo && !J1.explosion){
				  Disparar(1);
				  Musica_Comando(0x03);
				  nave1_dis = 0;
			  }
			  if(modoActual == ESTADO_JUG2 && nave2_dis && J2.vivo && !J2.explosion){
				  Disparar(2);
				  Musica_Comando(0x03);
				  nave2_dis = 0;
			  }
			  timerNaves = tick;
		  }
		  //Disparo de balas
		  if(tick - timerBalas >= 15)
		          {
		              ActualizarBalas();

		              // Flags para saber si hay que redibujar (Deben estar en PV o inicializadas aquí)
		              uint8_t hayHitJ1 = 0;
		              uint8_t hayHitJ2 = 0;

		              for(int j = 1; j <= (modoActual == ESTADO_JUG2 ? 2 : 1); j++)
		              {
		                  Bala* listaBalas = (j == 1) ? disparo_J1 : disparo_J2;
		                  int* scoreActual = (j == 1) ? &scoreJ1 : &scoreJ2;

		                  for(int b = 0; b < MAX_BALAS; b++)
		                  {
		                      if(listaBalas[b].activo)
		                      {
		                          for(int a = 0; a < MAX_ALIENS; a++)
		                          {
		                              if(Enemigos[a].vivo && !Enemigos[a].explosion)
		                              {
		                                  if(ChequearColision(listaBalas[b].x, listaBalas[b].y, 5, 10, Enemigos[a].x, Enemigos[a].y, Enemigos[a].w, Enemigos[a].h))
		                                  {
		                                      listaBalas[b].activo = 0;
		                                      Enemigos[a].explosion = 1;
		                                      Enemigos[a].frame_explosion = 0;
		                                      *scoreActual += 100;

		                                      if(j == 1) hayHitJ1 = 1;
		                                      else hayHitJ2 = 1;

		                                      if(Enemigos[a].tipo == 0){
		                                    	  Musica_Comando(0x08);			//rojo
		                                      }
		                                      else if(Enemigos[a].tipo == 1){
		                                    	Musica_Comando(0x0a);  			//verde
		                                      }
		                                      else if(Enemigos[a].tipo == 2){
		                                    	  Musica_Comando(0x09);			//mosca
		                                      }
		                                      else{
		                                    	  Musica_Comando(0x0b);			//azul
		                                      }
		                                  }
		                              }
		                          }
		                      }
		                  }
		              }

		              // --- ACTUALIZACIÓN DE PANTALLA (Fuera de los ciclos para evitar lag) ---
		              if(hayHitJ1)
		              {
		                  sprintf(Score, "%d", scoreJ1);
		                  LCD_Print(Score, 8, 12, 1, 0xFFFF, 0x0000);
		              }
		              if(hayHitJ2)
		              {
		                  sprintf(Score, "%d", scoreJ2);
		                  LCD_Print(Score, 290, 12, 1, 0xFFFF, 0x0000);
		              }

		              timerBalas = tick;
		          }
		  //Movimiento en formacion de aliens
		  if(tick - timerAnimacionAliens >= 200)
		  {
			  frameActual = (frameActual + 1) % 8;
			  timerAnimacionAliens = tick;
		  }
		  //spawn de aliens desde arriba
		  if(tick - tiempoSpawn >= 800 && AlienActual < MAX_ALIENS)
		  {
			  Enemigos[AlienActual].vivo = 1;
			  Enemigos[AlienActual].y = -20;
			  AlienActual++;
			  tiempoSpawn = tick;
		  }
		  //Colisiones
		  if(tick - timerExplo >= 60) {
			  for(int i=0; i<MAX_ALIENS; i++) {
				  if(Enemigos[i].explosion) {
					  Enemigos[i].frame_explosion++;
					  if(Enemigos[i].frame_explosion >= 4) {
						  FillRect((int)Enemigos[i].x, (int)Enemigos[i].y, 30, 30, 0x0000);
						  Enemigos[i].explosion = 0;
						  Enemigos[i].vivo = 0; }
				  }
			  }
			  //explosion nave 1
			  if(J1.explosion)
			  {
				  J1.frame_explosion++;
				  if(J1.frame_explosion >= 4)
				  {
					  FillRect((int)J1.x - 5, (int)J1.y - 5, 32, 32, 0x0000);
					  J1.explosion = 0;
					  J1.vivo = 0;
					  Musica_Comando(0x07);
					  // --- GUARDAR SCORE FINAL EN SD ---//
					  fres = f_open(&fil, "SCORE.TXT", FA_WRITE | FA_OPEN_ALWAYS);
					  if(fres == FR_OK)
					  {
						  f_lseek(&fil, f_size(&fil));
						  sprintf(buffer, "Final Score J1: %d\n", scoreJ1);
						  f_puts(buffer, &fil);
						  f_close(&fil);
					  }
					  if(modoActual == ESTADO_JUG1){
						  CausaVictoria = 1;
						  modoActual = ESTADO_VICTORIA;
						  Musica_Comando(0x05);
					  }
				  }
			  }
			  if(modoActual == ESTADO_JUG2  && J2.explosion){
				  FillRect((int)J2.x - 5, (int)J2.y - 5, 32, 32, 0x0000);
				  J2.explosion = 0;
				  J2.vivo = 0;
				  CausaVictoria = 2;
				  modoActual = ESTADO_VICTORIA;
				  Musica_Comando(0x05);
			  }
			  timerExplo = tick;
		  }

		  //Contar aliens vivos
		  aliensVivos = 0;
		  for(int i = 0; i < MAX_ALIENS; i++)
		  {
			  if(Enemigos[i].vivo || Enemigos[i].explosion){
				  aliensVivos++;
			  }
		  }

		  //Conteo de aliens vivos
		  if(AlienActual >= MAX_ALIENS && aliensVivos == 0 && modoActual != ESTADO_VICTORIA ){
			  CausaVictoria = 0;
			  modoActual = ESTADO_VICTORIA;
			  Musica_Comando(0x07);
		  }

		  //Colision de aliens contra naves
		  for(int i=0; i<MAX_ALIENS; i++) {
			  if(Enemigos[i].vivo && !Enemigos[i].explosion)
			  {
				  if(J1.vivo && !J1.explosion && ChequearColision(Enemigos[i].x, Enemigos[i].y, Enemigos[i].w, Enemigos[i].h, J1.x, J1.y, J1.w, J1.h)){
					  J1.explosion = 1;
					  J1.frame_explosion = 0;
					  Musica_Comando(0x04);
				  }
				  if(modoActual == ESTADO_JUG2 && J2.vivo && !J2.explosion && ChequearColision(Enemigos[i].x, Enemigos[i].y, Enemigos[i].w, Enemigos[i].h, J2.x, J2.y, J2.w, J2.h)){
					  J2.explosion = 1;
					  J2.frame_explosion = 0;
					  Musica_Comando(0x04);
				  }
			  }
		  }
		  break;
	  case ESTADO_VICTORIA:
		  static uint8_t pantallaGanador = 0;

		  if(!pantallaGanador){
			  if(modoActual == ESTADO_VICTORIA){
				  if(CausaVictoria == 0 || CausaVictoria == 1){
					  Ganador_J1();
				  }else if(CausaVictoria == 2){
					  Ganador_J1();
				  }else{
					  if(scoreJ1 >= scoreJ2){
						  Ganador_J1();
					  }else{
						  Ganador_J2();
					  }
				  }
				  pantallaGanador = 1;
			  }
		  }

		  if(nave1_sel){
			  nave1_sel = 0;
			  pantallaGanador = 0;
			  CausaVictoria = 0;
			  modoActual = ESTADO_MENU;
			  musicaMenu = 0;
			  LCD_Clear(0x0000);
			  PantallaInicio();
			  scoreJ1 = 0;
			  scoreJ2 = 0;
			  AlienActual = 0;
			  aliensVivos = 0;
			  for(int i=0; i<MAX_ALIENS; i++){
				  Enemigos[i].vivo = 0;
				  Enemigos[i].explosion = 0;
			  }
		  }

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
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

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
