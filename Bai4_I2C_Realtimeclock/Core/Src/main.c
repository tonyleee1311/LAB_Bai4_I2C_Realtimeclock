/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  /* USER CODE END Header */
  /* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FIRSTMODE 		1
#define SECONDMODE 		2
#define THIRDMODE 		3
#define MODIFYDAY    	10
#define MODIFYTIME		15
#define SAVEANDCHANGE   20
#define UPDATEALARM		35
#define UPDATETIME		30
#define ALARM_HOUR		0
#define ALARM_MIN		1
#define ALARM_SEC 		2


//Button
#define UP 			3
#define DOWN 		7
#define E 			12
#define CHANGEMODE  0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t count_led_debug = 0;
uint8_t counter_time = 0;
uint8_t counter_2hz = 0;
uint16_t counter_realtime = 0;
uint16_t counter_alarm = 0;
int status;
int prestatus;
int mode;
char str[100] = "Mode: ";
char str1[100] = "Status: ";
char str2[100] = "This is the counter_time of the system: ";
char str3[100] = "This is the mode for modify the alarm.";
uint8_t timeArray[] = {};
uint8_t alarmArray[3] = { 0x00, 0x00, 0x00 };
uint8_t addressRegTime[7] = { ADDRESS_DAY, ADDRESS_DATE, ADDRESS_MONTH, ADDRESS_YEAR, ADDRESS_HOUR, ADDRESS_MIN, ADDRESS_SEC };
uint8_t bufferTime[7] = {};
enum time_day { day, date, month, year, hour, minute, second };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void displayTime();
void updateTime();
void updateBufferTimeValue();
void fsmSystemRun();
void saveAndChange();
uint8_t isButtonChangeMode();
uint8_t isButtonUp();
uint8_t isButtonDown();
uint8_t isButtonUpMoreThan2S();
uint8_t isButtonSave();
uint8_t isAlarm();
void displayMode();
void modifyTime();
void toggleSpecificTime();
void buttonLongPressProcessing();
void processIncrease();
void displayAlarmModify();
void displayOnScreen(uint8_t);
void processRealTime();
void displayToggle();
void updateAlarm();
void checkAlarm();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
	MX_TIM2_Init();
	MX_SPI1_Init();
	MX_FSMC_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	system_init();
	alarmArray[ALARM_HOUR] = ds3231_alarm_hours;
	alarmArray[ALARM_MIN] = ds3231_alarm_min;
	alarmArray[ALARM_SEC] = ds3231_alarm_sec;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	//lcd_Clear(WHITE);
	while (1)
	{
		lcd_ShowStr(30, 50, " MODE", WHITE, BLACK, 24, 0);
		if (mode == THIRDMODE) {
			updateAlarm();
		}
		//lcd_ShowStr(30, 200," STATUS", WHITE, BLACK, 24, 0);
		//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
		//lcd_ShowStr(30, 50,str, GREEN, BLACK, 24, 0);
		//lcd_ShowStr(30, 50, "This is the mode of the system: ", GREEN, BLACK, 12, 0);
		while (!flag_timer2);
		flag_timer2 = 0;
		button_Scan();
		fsmSystemRun();
		//checkAlarm();
		//test_LedDebug();
		//ds3231_ReadTime();
		//displayTime();
	  /* USER CODE END WHILE */
	  /* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void system_init() {
	HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	timer_init();
	led7_init();
	button_init();
	lcd_init();
	ds3231_init();
	status = FIRSTMODE;
	mode = FIRSTMODE;
	lcd_Clear(BLACK);
	updateTime();
	ds3231_ReadTime();
	ds3231_ReadAlarm();
	setTimer2(50);
}

void test_LedDebug() {
	count_led_debug = (count_led_debug + 1) % 20;
	if (count_led_debug == 0) {
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_7seg() {
	led7_SetDigit(0, 0, 0);
	led7_SetDigit(5, 1, 0);
	led7_SetDigit(4, 2, 0);
	led7_SetDigit(7, 3, 0);
}
void test_button() {
	for (int i = 0; i < 16; i++) {
		if (button_count[i] == 1) {
			led7_SetDigit(i / 10, 2, 0);
			led7_SetDigit(i % 10, 3, 0);
		}
	}
}

void updateTime() {
	ds3231_Write(ADDRESS_YEAR, 23);
	ds3231_Write(ADDRESS_MONTH, 10);
	ds3231_Write(ADDRESS_DATE, 20);
	ds3231_Write(ADDRESS_DAY, 6);
	ds3231_Write(ADDRESS_HOUR, 20);
	ds3231_Write(ADDRESS_MIN, 11);
	ds3231_Write(ADDRESS_SEC, 23);
}
void updateAlarm() {
	ds3231_ReadAlarm();
	lcd_ShowStr(20, 200, " ALARM", WHITE, BLACK, 24, 0);
	lcd_ShowIntNum(100, 200, ds3231_alarm_hours, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(140, 200, ds3231_alarm_min, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(180, 200, ds3231_alarm_sec, 2, GREEN, BLACK, 24);
}
uint8_t isButtonUp()
{
	if (button_count[UP] == 1)
		return 1;
	else
		return 0;
}
uint8_t isButtonDown()
{
	if (button_count[DOWN] == 1)
		return 1;
	else
		return 0;
}
uint8_t isButtonChangeMode() {
	if (button_count[CHANGEMODE] == 1)
		return 1;
	else
		return 0;
}
uint8_t isButtonSave() {
	if (button_count[E] == 1)
		return 1;
	else
		return 0;
}
uint8_t isButtonUpMoreThan2S() {
	if (button_count[UP] >= 40)
		return 1;
	else
		return 0;
}
uint8_t isAlarm() {
	return 1;
}
void updateBufferTimeValue() {
	bufferTime[0] = ds3231_day;
	bufferTime[1] = ds3231_date;
	bufferTime[2] = ds3231_month;
	bufferTime[3] = ds3231_year;
	bufferTime[4] = ds3231_hours;
	bufferTime[5] = ds3231_min;
	bufferTime[6] = ds3231_year;
}
void displayTime() {
	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
	//Display mode
	lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
	//lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 0);

}
void displayAlarmModify() {
	//lcd_Clear(WHITE);
	lcd_ShowIntNum(70, 100, alarmArray[ALARM_HOUR], 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(110, 100, alarmArray[ALARM_MIN], 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(150, 100, alarmArray[ALARM_SEC], 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);

	//Display mode
	lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
	//lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 1);
	//lcd_ShowStr(30, 70, str3, GREEN, BLACK, 12, 1);

}
void displayMode() {
	lcd_ShowIntNum(130, 50, mode, 2, GREEN, BLACK, 24);
	lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 1);
	//lcd_ShowStr(x, y, str, fc, bc, sizey, mode)
}
void displayOnScreen(uint8_t mode) {
	if (mode == FIRSTMODE) {
		displayTime();
	}
	else if (mode == SECONDMODE) {
		displayTime();
	}
	else {
		displayAlarmModify();
	}
}
void toggleSpecificTime() {
	/*	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);*/
	static int counter_toggle = 0;
	counter_2hz = (counter_2hz + 1) % 10;
	if (counter_2hz == 0) {
		//displayOnScreen(mode);
		switch (counter_time) {
		case day:
			if (counter_toggle % 2 == 0)
				lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			else
				//lcd_ShowIntNum(20, 130, 0x00, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(20, 130, " AA", BLACK, BLACK, 24, 0);
			//lcd_ShowChar(20, y, num, fc, bc, sizey, mode)
			break;
		case date:
			if (counter_toggle % 2 == 0)
				lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(70, 130, " AAA", BLACK, BLACK, 24, 0);
			break;
		case month:
			if (counter_toggle % 2 == 0)
				lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(110, 130, " AAA", BLACK, BLACK, 24, 0);
			break;
		case year:
			if (counter_toggle % 2 == 0)
				lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(150, 130, " AAA", BLACK, BLACK, 24, 0);
			break;
		case hour:
			if (counter_toggle % 2 == 0) {
				if (mode == SECONDMODE)
					lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
				else if (mode == THIRDMODE)
					lcd_ShowIntNum(70, 100, alarmArray[ALARM_HOUR], 2, GREEN, BLACK, 24);
			}
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(70, 100, " AAA", BLACK, BLACK, 24, 0);
			break;
		case minute:
			if (counter_toggle % 2 == 0) {
				if (mode == SECONDMODE)
					lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
				else if (mode == THIRDMODE)
					lcd_ShowIntNum(110, 100, alarmArray[ALARM_MIN], 2, GREEN, BLACK, 24);

			}
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(110, 100, " AAA", BLACK, BLACK, 24, 0);
			break;
		case second:
			if (counter_toggle % 2 == 0) {
				if (mode == SECONDMODE)
					lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
				else if (mode == THIRDMODE)
					lcd_ShowIntNum(150, 100, alarmArray[ALARM_SEC], 2, GREEN, BLACK, 24);
			}
			else
				//lcd_ShowIntNum(20, 130, 0xff, 2, YELLOW, BLACK, 24);
				lcd_ShowStr(150, 100, " AAA", BLACK, BLACK, 24, 0);
			break;
		default:
			break;
		}
		counter_toggle++;
		displayToggle();
	}
}
void saveAndChange() {
	if (mode == SECONDMODE) {
		switch (counter_time) {
		case day:
			ds3231_Write(ADDRESS_DAY, ds3231_day);
			break;
		case date:
			ds3231_Write(ADDRESS_DATE, ds3231_date);
			break;
		case month:
			ds3231_Write(ADDRESS_MONTH, ds3231_month);
			break;
		case year:
			ds3231_Write(ADDRESS_YEAR, ds3231_year);
			break;
		case hour:
			ds3231_Write(ADDRESS_HOUR, ds3231_hours);
			break;
		case minute:
			ds3231_Write(ADDRESS_MIN, ds3231_min);
			break;
		case second:
			ds3231_Write(ADDRESS_SEC, ds3231_sec);
			break;
		default:
			break;
		}
		prestatus = status;
		counter_time = (counter_time + 1) % 7;

		if (counter_time == minute || counter_time == second || counter_time == hour) {
			status = MODIFYTIME;
			//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
			//lcd_ShowIntNum(120, 240, counter_time, 2, YELLOW, BLACK, 24);
		}
		else {
			status = MODIFYDAY;
			//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
			//lcd_ShowIntNum(120, 240, counter_time, 2, YELLOW, BLACK, 24);
		}
	}
	else if (mode == THIRDMODE) {
		//
		switch (counter_time) {
		case hour:
			ds3231_Write(ADDRESS_ALARM1_HOUR, alarmArray[ALARM_HOUR]);
			break;
		case minute:
			ds3231_Write(ADDRESS_ALARM1_MIN, alarmArray[ALARM_MIN]);
			break;
		case second:
			ds3231_Write(ADDRESS_ALARM1_SEC, alarmArray[ALARM_SEC]);
			break;
		default:
			break;
		}
		prestatus = status;
		counter_time = (counter_time + 1) % 7;
		if (counter_time == day)
			counter_time = hour;
		status = MODIFYTIME;
	}
	HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	//ds3231_ReadTime();

}
void buttonLongPressProcessing() {

	//lcd_DrawCircle(xc, yc, c, r, fill)
	//processIncrease();
	if (isButtonUpMoreThan2S()) {
		//lcd_DrawCircle(100, 280, RED, 20,1);
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
		if (((button_count[UP])) % 4 == 0) {
			//lcd_ShowIntNum(100, 200, button_count[UP], 2, YELLOW, BLACK, 24);
			processIncrease();
		}
	}
	//displayOnScreen(mode);
}
void processRealTime() {
	counter_realtime = (counter_realtime + 1) % 20;
	if (counter_realtime == 0) {
		ds3231_sec += 1;
		if (ds3231_sec > 59) {
			ds3231_sec = 0;
			ds3231_min += 1;
			if (ds3231_min > 59) {
				ds3231_min = 0;
				ds3231_hours += 1;
				if (ds3231_hours > 23) {
					ds3231_hours = 0;
				}
			}
		}
		ds3231_Write(ADDRESS_HOUR, ds3231_hours);
		ds3231_Write(ADDRESS_MIN, ds3231_min);
		ds3231_Write(ADDRESS_SEC, ds3231_sec);
		displayOnScreen(mode);
		ds3231_ReadTime();
	}
}
void checkAlarm() {
	if ((ds3231_hours == alarmArray[ALARM_HOUR]) && (ds3231_min == alarmArray[ALARM_MIN]) && (ds3231_sec == alarmArray[ALARM_SEC])) {
		lcd_ShowStr(30, 200, " TIT TIT TIT", RED, BLACK, 24, 0);
		//HAL_Delay(1000);
	}
	else {
		lcd_ShowStr(30, 200, " bbbbbbbbbbbbbb", BLACK, BLACK, 24, 0);
	}
}
void processIncrease() {
	switch (counter_time) {
	case day:
		ds3231_day += 1;
		if (ds3231_day > 8)
			ds3231_day = 2;
		//buttonLongPressProcessing();
		//saveAndChange(ADDRESS_DAY,ds3231_day);
		break;
	case date:
		ds3231_date += 1;
		if (ds3231_month == 4 || ds3231_month == 6 || ds3231_month == 9 || ds3231_month == 11) {
			if (ds3231_date > 30)
				ds3231_date = 1;
		}
		else if (ds3231_month == 2) {
			if (((ds3231_year % 4 == 0) && (ds3231_year % 100 != 0)) || (ds3231_year % 400 == 0)) {
				//this is a leap year
				if (ds3231_date > 29)
					ds3231_date = 1;
			}
			else {
				if (ds3231_date > 28)
					ds3231_date = 1;
			}
		}
		else {
			if (ds3231_date > 31)
				ds3231_date = 1;
		}
		//buttonLongPressProcessing();
		//saveAndChange(ADDRESS_DATE,ds3231_date);
		break;
	case month:
		ds3231_month += 1;
		if (ds3231_month > 12)
			ds3231_month = 1;
		//buttonLongPressProcessing();
		//saveAndChange(ADDRESS_MONTH,ds3231_month);
		break;
	case year:
		ds3231_year += 1;
		//saveAndChange(ADDRESS_MONTH,ds3231_month);
		//buttonLongPressProcessing();
		break;
	case hour:
		if (mode == THIRDMODE) {
			alarmArray[ALARM_HOUR] += 1;
			if (alarmArray[ALARM_HOUR] > 23) {
				alarmArray[ALARM_HOUR] = 0;
				//buttonLongPressProcessing();
			}
		}
		else if (mode == SECONDMODE) {
			ds3231_hours += 1;
			if (ds3231_hours > 23)
				ds3231_hours = 0;
			//buttonLongPressProcessing();
		}
		//saveAndChange(ADDRESS_HOUR, ds3231_hours);
		break;
	case minute:
		if (mode == THIRDMODE) {
			alarmArray[ALARM_MIN] += 1;
			if (alarmArray[ALARM_MIN] > 59) {
				alarmArray[ALARM_MIN] = 0;
				//buttonLongPressProcessing();
			}
		}
		else if (mode == SECONDMODE) {
			ds3231_min += 1;
			if (ds3231_min > 59)
				ds3231_min = 0;
			//buttonLongPressProcessing();
		}

		//saveAndChange(ADDRESS_MIN, ds3231_min);
		break;
	case second:
		if (mode == THIRDMODE) {
			alarmArray[ALARM_SEC] += 1;
			if (alarmArray[ALARM_SEC] > 59) {
				alarmArray[ALARM_SEC] = 0;
				//buttonLongPressProcessing();
			}
		}
		else if (mode == SECONDMODE) {
			ds3231_sec += 1;
			if (ds3231_sec > 59)
				ds3231_sec = 0;
			//buttonLongPressProcessing();
		}

		//saveAndChange(ADDRESS_SEC, ds3231_sec);
		break;
	default:
		break;
	}
	prestatus = status;
	displayOnScreen(mode);
	//on
}
void displayToggle() {
	if (mode == THIRDMODE) {
		switch (counter_time) {
		case hour:
			//lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, alarmArray[ALARM_MIN], 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, alarmArray[ALARM_SEC], 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			//lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 0);
			break;
		case minute:
			lcd_ShowIntNum(70, 100, alarmArray[ALARM_HOUR], 2, GREEN, BLACK, 24);
			//lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, alarmArray[ALARM_SEC], 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			//lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 0);
			break;
		case second:
			lcd_ShowIntNum(70, 100, alarmArray[ALARM_HOUR], 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, alarmArray[ALARM_MIN], 2, GREEN, BLACK, 24);
			//lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			//lcd_ShowStr(30, 50, str, GREEN, BLACK, 12, 0);
			break;
		default:
			break;
		}
	}
	else {
		switch (counter_time) {
		case day:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			//lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case date:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			//lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case month:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			//lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case year:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			//lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case hour:
			//lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case minute:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			//lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		case second:
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			//lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			//Display mode
			lcd_ShowIntNum(100, 50, mode, 2, GREEN, BLACK, 24);
			break;
		default:
			break;
		}
	}

}
void fsmSystemRun() {
	switch (status) {
	case FIRSTMODE:
		//ds3231_ReadTime();
		//updateBufferTimeValue();
		processRealTime();
		//ds3231_ReadAlarm();
		//updateAlarm();
		checkAlarm();
		//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
		if (isButtonChangeMode()) {
			prestatus = status;
			status = SECONDMODE;
			mode = SECONDMODE;
			lcd_Clear(BLACK);
		}
		break;
	case SECONDMODE:
		//ds3231_ReadTime();
		//displayTime();
		//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
		toggleSpecificTime();
		if (isButtonUp()) {
			prestatus = status;
			status = MODIFYDAY;
		}
		if (isButtonChangeMode()) {
			//do not need update status
			status = THIRDMODE;
			mode = THIRDMODE;
			counter_time = hour;
			lcd_Clear(BLACK);
		}
		if (isButtonSave()) {
			//todo function
			prestatus = status;
			lcd_Clear(BLACK);
			status = SAVEANDCHANGE;
		}
		break;
	case THIRDMODE:
		//ds3231_ReadTime();
		//displayTime(); //lam ham ma gia tri la 00:00:00
		//displayAlarmModify();
		toggleSpecificTime();
		if (isButtonUp()) {
			prestatus = status;
			status = MODIFYTIME;
		}
		if (isButtonChangeMode()) {
			//do not need update status
			status = FIRSTMODE;
			mode = FIRSTMODE;
			counter_time = day;
			lcd_Clear(BLACK);
		}
		if (isButtonSave()) {
			//todo function
			prestatus = status;
			lcd_Clear(BLACK);
			status = SAVEANDCHANGE;
		}
		break;
	case MODIFYDAY:
		//lcd_ShowIntNum(120, 300, status, 2, YELLOW, BLACK, 24);
		if (prestatus == SECONDMODE && status == MODIFYDAY)
			processIncrease();
		//ds3231_ReadTime();
		//lcd_ShowIntNum(120, 200, status, 2, YELLOW, BLACK, 24);
		//displayTime();
		toggleSpecificTime();
		//processIncrease(); suy nghi sao cho nay chi chay dung 1 lan thoi
		if (isButtonUpMoreThan2S()) {
			//lcd_DrawCircle(100, 280, RED, 20,1);
			//processIncrease();
			buttonLongPressProcessing();
		}
		else if (isButtonUp()) {
			//lcd_DrawCircle(100, 280, YELLOW, 20,1);
			processIncrease();
		}
		/*			if(isButtonUp()){
						processIncrease();
					}*/
		if (isButtonChangeMode()) {
			if (mode == SECONDMODE) {
				prestatus = status;
				status = THIRDMODE;
				mode = THIRDMODE;
				counter_time = hour;
				lcd_Clear(BLACK);
			}
		}
		if (isButtonSave()) {
			//todo function
			prestatus = status;
			lcd_Clear(BLACK);
			status = SAVEANDCHANGE;
		}
		break;
	case MODIFYTIME:
		if (prestatus == THIRDMODE && status == MODIFYTIME) {
			processIncrease();
		}
		//displayAlarmModify();
		//lcd_ShowIntNum(120, 300, status, 2, YELLOW, BLACK, 24);
		toggleSpecificTime();
		//processIncrease(); suy nghi sao cho nay chi chay dung 1 lan thoi
		if (isButtonUpMoreThan2S()) {
			//lcd_DrawCircle(100, 280, RED, 20,1);
			//processIncrease();
			buttonLongPressProcessing();
		}
		else if (isButtonUp()) {
			//lcd_DrawCircle(100, 280, YELLOW, 20,1);
			processIncrease();
		}
		if (isButtonChangeMode()) {
			if (mode == SECONDMODE) {
				prestatus = status;
				status = THIRDMODE;
				mode = THIRDMODE;
				counter_time = hour;
			}
			else {
				prestatus = status;
				status = FIRSTMODE;
				mode = FIRSTMODE;
				counter_time = day;
			}
			lcd_Clear(BLACK);
		}
		if (isButtonSave()) {
			//todo function
			prestatus = status;
			lcd_Clear(BLACK);
			status = SAVEANDCHANGE;
		}
		break;
	case SAVEANDCHANGE:
		counter_2hz = 0;
		saveAndChange();
	default:
		break;
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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	   /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
