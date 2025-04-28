
#include "main.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stm32f1xx_hal.h"
#include <stdint.h>


ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

#define FFT_SIZE 256

#define ILI9488_CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define ILI9488_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

#define XPT2046_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define XPT2046_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

#define ILI9488_DC_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
#define ILI9488_DC_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)

#define ILI9488_RST_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define ILI9488_RST_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

#define DIR_PIN GPIO_PIN_9
#define DIR_PORT GPIOA
#define STEP_PIN GPIO_PIN_7
#define STEP_PORT GPIOC


void ILI9488_SendCommand(uint8_t cmd) {
    ILI9488_CS_LOW();
    ILI9488_DC_LOW();  // Command mode
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    ILI9488_CS_HIGH();
}

void ILI9488_SendData(uint8_t data) {
    ILI9488_CS_LOW();
    ILI9488_DC_HIGH();  // Data mode
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    ILI9488_CS_HIGH();
}


void ILI9488_Init() {
    // Reset the display
    ILI9488_RST_LOW();
    HAL_Delay(50);
    ILI9488_RST_HIGH();
    HAL_Delay(50);

    ILI9488_SendCommand(0x01);  // Software Reset
    HAL_Delay(100);

    ILI9488_SendCommand(0x3A);  // Set Pixel Format
    ILI9488_SendData(0x55);     // 16-bit color

    ILI9488_SendCommand(0x36);  // Memory Access Control
    ILI9488_SendData(0x48);     // Row and column order

    ILI9488_SendCommand(0x11);  // Sleep Out
    HAL_Delay(120);

    ILI9488_SendCommand(0x29);  // Display ON
    HAL_Delay(20);

}


void ILI9488_DrawColor(uint16_t color) {
    uint8_t colorHigh = (color >> 8);
    uint8_t colorLow = (color & 0xFF);

    ILI9488_SendCommand(0x2C);  // Memory Write

    for (uint32_t i = 0; i < 320 * 240; i++) {
        ILI9488_SendData(colorHigh);
        ILI9488_SendData(colorLow);
    }
}


uint16_t XPT2046_ReadData(uint8_t command) {
    uint8_t rxBuffer[2];
    XPT2046_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, rxBuffer, 2, HAL_MAX_DELAY);
    XPT2046_CS_HIGH();
    return ((rxBuffer[0] << 8) | rxBuffer[1]) >> 3;  // 12-bit conversion
}



const uint8_t Font8x8[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // SPACE (0x20)
    {0x00, 0x00, 0x5F, 0x5F, 0x00, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x00}, // "
    {0x14, 0x7F, 0x7F, 0x14, 0x14, 0x7F, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x2A, 0x7F, 0x7F, 0x2A, 0x2A, 0x12}, // $
    {0x23, 0x33, 0x18, 0x0C, 0x06, 0x33, 0x31, 0x00}, // %
    {0x36, 0x7F, 0x49, 0x7F, 0x3E, 0x08, 0x74, 0x00}, // &
    {0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x3E, 0x63, 0x41, 0x00, 0x00, 0x00}, // (
    {0x00, 0x00, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00}, // )
    {0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x3E, 0x08, 0x08, 0x00, 0x00}, // +
    {0x00, 0x80, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00}, // -
    {0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00}, // .
    {0x20, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // /
	{0x3E, 0x7F, 0x51, 0x49, 0x45, 0x7F, 0x3E, 0x00},  /* '0' (48, 0x30) */
	    {0x00, 0x42, 0x7F, 0x7F, 0x40, 0x00, 0x00, 0x00},  /* '1' (49, 0x31) */
	    {0x42, 0x63, 0x71, 0x59, 0x4F, 0x46, 0x00, 0x00},  /* '2' (50, 0x32) */
	    {0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00},  /* '3' (51, 0x33) */
	    {0x18, 0x1C, 0x16, 0x13, 0x7F, 0x7F, 0x10, 0x00},  /* '4' (52, 0x34) */
	    {0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x00},  /* '5' (53, 0x35) */
	    {0x3E, 0x7F, 0x49, 0x49, 0x79, 0x30, 0x00, 0x00},  /* '6' (54, 0x36) */
	    {0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x00},  /* '7' (55, 0x37) */
	    {0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00},  /* '8' (56, 0x38) */
	    {0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x00},  /* '9' (57, 0x39) */
	    {0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00},  /* ':' (58, 0x3A) */
	    {0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00},  /* ';' (59, 0x3B) */
	    {0x08, 0x1C, 0x36, 0x63, 0x41, 0x00, 0x00, 0x00},  /* '<' (60, 0x3C) */
	    {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00},  /* '=' (61, 0x3D) */
	    {0x41, 0x63, 0x36, 0x1C, 0x08, 0x00, 0x00, 0x00},  /* '>' (62, 0x3E) */
	    {0x02, 0x03, 0x51, 0x59, 0x0F, 0x06, 0x00, 0x00},   /* '?' (63, 0x3F) */
	    {0x3E, 0x7F, 0x41, 0x5D, 0x5F, 0x4E, 0x00, 0x00},  /* '@' (64, 0x40) */
	    {0x7E, 0x7F, 0x09, 0x09, 0x7F, 0x7E, 0x00, 0x00},  /* 'A' (65, 0x41) */
	    {0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00},  /* 'B' (66, 0x42) */
	    {0x3E, 0x7F, 0x41, 0x41, 0x63, 0x22, 0x00, 0x00},  /* 'C' (67, 0x43) */
	    {0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00},  /* 'D' (68, 0x44) */
	    {0x7F, 0x7F, 0x49, 0x49, 0x41, 0x63, 0x00, 0x00},  /* 'E' (69, 0x45) */
	    {0x7F, 0x7F, 0x09, 0x09, 0x01, 0x03, 0x00, 0x00},  /* 'F' (70, 0x46) */
	    {0x3E, 0x7F, 0x41, 0x49, 0x79, 0x3A, 0x00, 0x00},  /* 'G' (71, 0x47) */
	    {0x7F, 0x7F, 0x08, 0x08, 0x7F, 0x7F, 0x00, 0x00},  /* 'H' (72, 0x48) */
	    {0x00, 0x41, 0x7F, 0x7F, 0x41, 0x00, 0x00, 0x00},  /* 'I' (73, 0x49) */
	    {0x20, 0x60, 0x41, 0x7F, 0x3F, 0x01, 0x00, 0x00},  /* 'J' (74, 0x4A) */
	    {0x7F, 0x7F, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x00},  /* 'K' (75, 0x4B) */
	    {0x7F, 0x7F, 0x40, 0x40, 0x40, 0x60, 0x00, 0x00},  /* 'L' (76, 0x4C) */
	    {0x7F, 0x7F, 0x06, 0x0C, 0x06, 0x7F, 0x7F, 0x00},  /* 'M' (77, 0x4D) */
	    {0x7F, 0x7F, 0x06, 0x0C, 0x18, 0x7F, 0x7F, 0x00},  /* 'N' (78, 0x4E) */
	    {0x3E, 0x7F, 0x41, 0x41, 0x7F, 0x3E, 0x00, 0x00},  /* 'O' (79, 0x4F) */
	    {0x7F, 0x7F, 0x09, 0x09, 0x0F, 0x06, 0x00, 0x00},  /* 'P' (80, 0x50) */
	    {0x3E, 0x7F, 0x41, 0x61, 0x7F, 0x5E, 0x00, 0x00},  /* 'Q' (81, 0x51) */
	    {0x7F, 0x7F, 0x09, 0x19, 0x7F, 0x66, 0x00, 0x00},  /* 'R' (82, 0x52) */
	    {0x26, 0x6F, 0x49, 0x49, 0x7B, 0x32, 0x00, 0x00},  /* 'S' (83, 0x53) */
	    {0x03, 0x01, 0x7F, 0x7F, 0x01, 0x03, 0x00, 0x00},  /* 'T' (84, 0x54) */
	    {0x3F, 0x7F, 0x40, 0x40, 0x7F, 0x3F, 0x00, 0x00},  /* 'U' (85, 0x55) */
	    {0x1F, 0x3F, 0x60, 0x60, 0x3F, 0x1F, 0x00, 0x00},  /* 'V' (86, 0x56) */
	    {0x7F, 0x7F, 0x30, 0x18, 0x30, 0x7F, 0x7F, 0x00},  /* 'W' (87, 0x57) */
	    {0x63, 0x77, 0x1C, 0x08, 0x1C, 0x77, 0x63, 0x00},  /* 'X' (88, 0x58) */
	    {0x07, 0x0F, 0x78, 0x78, 0x0F, 0x07, 0x00, 0x00},  /* 'Y' (89, 0x59) */
	    {0x61, 0x71, 0x59, 0x4D, 0x47, 0x43, 0x00, 0x00},  /* 'Z' (90, 0x5A) */
		 {0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00}, // 91: '['
		    {0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00}, // 92: '\'
		    {0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 0x00}, // 93: ']'
		    {0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00}, // 94: '^'
		    {0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // 95: '_'
		    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00}, // 96: '`'
			{0x20, 0x54, 0x54, 0x54, 0x78, 0x7C, 0x00, 0x00},  /* 'a' (97, 0x61) */
			{0x7F, 0x7F, 0x48, 0x48, 0x78, 0x30, 0x00, 0x00},  /* 'b' (98, 0x62) */
			{0x38, 0x7C, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00},  /* 'c' (99, 0x63) */
			{0x30, 0x78, 0x48, 0x48, 0x7F, 0x7F, 0x00, 0x00},  /* 'd' (100, 0x64) */
			{0x38, 0x7C, 0x54, 0x54, 0x5C, 0x18, 0x00, 0x00},  /* 'e' (101, 0x65) */
			{0x08, 0x7E, 0x7F, 0x09, 0x03, 0x02, 0x00, 0x00},  /* 'f' (102, 0x66) */
			{0x98, 0xBC, 0xA4, 0xA4, 0xFC, 0x7C, 0x00, 0x00},  /* 'g' (103, 0x67) */
			{0x7F, 0x7F, 0x08, 0x08, 0x78, 0x70, 0x00, 0x00},  /* 'h' (104, 0x68) */
			{0x00, 0x48, 0x7D, 0x7D, 0x40, 0x00, 0x00, 0x00},  /* 'i' (105, 0x69) */
			{0x40, 0xC8, 0xBD, 0x7D, 0x00, 0x00, 0x00, 0x00},  /* 'j' (106, 0x6A) */
			{0x7F, 0x7F, 0x10, 0x38, 0x6C, 0x44, 0x00, 0x00},  /* 'k' (107, 0x6B) */
			{0x00, 0x3F, 0x7F, 0x40, 0x40, 0x00, 0x00, 0x00},  /* 'l' (108, 0x6C) */
			{0x7C, 0x7C, 0x18, 0x38, 0x1C, 0x7C, 0x78, 0x00},  /* 'm' (109, 0x6D) */
			{0x7C, 0x7C, 0x08, 0x08, 0x78, 0x70, 0x00, 0x00},  /* 'n' (110, 0x6E) */
			{0x38, 0x7C, 0x44, 0x44, 0x7C, 0x38, 0x00, 0x00},  /* 'o' (111, 0x6F) */
			{0xFC, 0xFC, 0x24, 0x24, 0x3C, 0x18, 0x00, 0x00},  /* 'p' (112, 0x70) */
			{0x18, 0x3C, 0x24, 0x24, 0xFC, 0xFC, 0x00, 0x00},  /* 'q' (113, 0x71) */
			{0x7C, 0x7C, 0x08, 0x04, 0x0C, 0x08, 0x00, 0x00},  /* 'r' (114, 0x72) */
			{0x48, 0x5C, 0x54, 0x74, 0x24, 0x00, 0x00, 0x00},  /* 's' (115, 0x73) */
			{0x04, 0x3E, 0x7F, 0x44, 0x24, 0x00, 0x00, 0x00},  /* 't' (116, 0x74) */
			{0x3C, 0x7C, 0x40, 0x40, 0x7C, 0x3C, 0x00, 0x00},  /* 'u' (117, 0x75) */
			{0x1C, 0x3C, 0x60, 0x60, 0x3C, 0x1C, 0x00, 0x00},  /* 'v' (118, 0x76) */
			{0x3C, 0x7C, 0x30, 0x18, 0x30, 0x7C, 0x3C, 0x00},  /* 'w' (119, 0x77) */
			{0x44, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0x44, 0x00},  /* 'x' (120, 0x78) */
			{0x9C, 0xBC, 0xA0, 0xA0, 0xFC, 0x7C, 0x00, 0x00},  /* 'y' (121, 0x79) */
			{0x4C, 0x6C, 0x74, 0x5C, 0x4C, 0x00, 0x00, 0x00},  /* 'z' (122, 0x7A) */



};

void ILI9488_SetCursor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    ILI9488_SendCommand(0x2A);  // Column Address Set
    ILI9488_SendData(x >> 8);
    ILI9488_SendData(x & 0xFF);
    ILI9488_SendData((x + w - 1) >> 8);  // Set width dynamically
    ILI9488_SendData((x + w - 1) & 0xFF);

    ILI9488_SendCommand(0x2B);  // Row Address Set
    ILI9488_SendData(y >> 8);
    ILI9488_SendData(y & 0xFF);
    ILI9488_SendData((y + h - 1) >> 8);  // Set height dynamically
    ILI9488_SendData((y + h - 1) & 0xFF);

    ILI9488_SendCommand(0x2C);  // Memory Write
}


void ILI9488_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg) {
    if (ch < 0x20 || ch > 0x7F) return;  // Ignore unsupported characters

    ILI9488_SetCursor(x, y,8,8);
    uint8_t *charData = (uint8_t *)Font8x8[ch - 0x20];

    for (uint8_t row = 0; row < 8; row++) {  // 8 rows (top to bottom)
        for (uint8_t col = 0; col < 8; col++) {  // 8 columns (left to right)
            if (charData[col] & (1 << row)) {  // Now row is controlled by bit shifts
                ILI9488_SendData(color >> 8);
                ILI9488_SendData(color & 0xFF);
            } else {
                ILI9488_SendData(bg >> 8);
                ILI9488_SendData(bg & 0xFF);
            }
        }
    }
}


void ILI9488_PrintText(uint16_t x, uint16_t y, char *text, uint16_t color, uint16_t bg) {
    while (*text) {
        ILI9488_DrawChar(x, y, *text, color, bg);
        x += 9;  // Move to next character (8 pixels wide + 1 space)
        text++;
    }
}



void ILI9488_SetRotation(uint8_t rotation) {
    uint8_t madctl = 0;

    switch (rotation) {
        case 0:  // 0° (Default)
            madctl = 0x48;  // Row/Column Exchange | BGR
            break;
        case 1:  // 90° Clockwise
            madctl = 0x28;  // Column Address Order | BGR
            break;
        case 2:  // 180° Rotation
            madctl = 0x88;  // Row/Column Exchange | Row Address Order | BGR
            break;
        case 3:  // 270° Clockwise
            madctl = 0xE8;  // Column Address Order | Row Address Order | BGR
            break;
        default:
            return; // Invalid input, do nothing
    }

    ILI9488_SendCommand(0x36); // MADCTL (Memory Access Control)
    ILI9488_SendData(madctl);
}




// Function to display a float on screen at given coordinates
void DisplayFloat(float number, uint16_t x, uint16_t y, uint16_t color, uint16_t bgcolor) {
    char buffer[30];
    sprintf(buffer, " %.1f ", number);
    ILI9488_PrintText(x, y, buffer, color, bgcolor);
}


void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    ILI9488_SetCursor(x, y, 1, 1);
    ILI9488_SendData(color >> 8);
    ILI9488_SendData(color & 0xFF);
}

void ILI9488_DrawLine(int x0, int y0, int x1, int y1, uint16_t color) {
                int dx = abs(x1 - x0);
                int dy = -abs(y1 - y0);
                int sx = (x0 < x1) ? 1 : -1;
                int sy = (y0 < y1) ? 1 : -1;
                int err = dx + dy;  // error value e_xy

                while (1) {
                    ILI9488_DrawPixel(x0, y0, color);

                    if (x0 == x1 && y0 == y1) break;
                    int e2 = 2 * err;

                    if (e2 >= dy) {
                        err += dy;
                        x0 += sx;
                    }
                    if (e2 <= dx) {
                        err += dx;
                        y0 += sy;
                    }
                }
            }




void DrawGraphAxes(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                               char* title, char* xlabel, char* ylabel,
                               uint16_t color) {
                // Draw X and Y axes
                ILI9488_DrawLine(x, y, x, y + height , color);          // Y axis
                ILI9488_DrawLine(x , y , x + width, y , color);  // X axis

                // Labels (simple positioning, adjust as needed)
                ILI9488_PrintText(x + width / 2 + 15 ,y - 25 , xlabel, color, 0x0000);
                ILI9488_PrintText(x + width / 2 - 60, y - 25 , title, color, 0x0000);
                ILI9488_SetRotation(2);
                ILI9488_PrintText(90, y - 25 , ylabel, color, 0x0000);
                ILI9488_SetRotation(3);

               // ILI9488_PrintText(0, 0, ylabel, color, 0x0000);


            }


//DrawGraphAxes(32, 32, 288, 208, "GPR", "Distance [m]", "Depth [m]", 0xFFFF);  // White axes

#define MCP4725_ADDR 0x60 << 1  // Shifted left because HAL expects 8-bit address

void MCP4725_WriteDAC(uint16_t dac_value) {
    uint8_t data[3];

    // 0x40 = Command byte to write DAC register
    data[0] = 0x40;
    data[1] = (dac_value >> 4) & 0xFF;        // Upper 8 bits
    data[2] = (dac_value & 0x0F) << 4;        // Lower 4 bits, shifted

    HAL_I2C_Master_Transmit(&hi2c1, MCP4725_ADDR, data, 3, HAL_MAX_DELAY);
}


float Read_ADC_Voltage(void)
{
    uint32_t adc_raw = 0;
    float adc_voltage = 0.0f;

    HAL_ADC_Start(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        adc_raw = HAL_ADC_GetValue(&hadc1);
        adc_voltage = ((float)adc_raw * 3.3f) / 4095.0f;  // 12-bit resolution
    }

    HAL_ADC_Stop(&hadc1);

    return adc_voltage;
}





/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

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
        MX_ADC1_Init();
        HAL_Init();
	    SystemClock_Config();
	    MX_GPIO_Init();
	    MX_SPI1_Init();
	    MX_I2C1_Init();
	    ILI9488_Init();

	    ILI9488_SetCursor(0,0,240,320);
	  	HAL_Delay(50);
    ILI9488_DrawColor(0x0000);//Full black screen
    HAL_Delay(50);
    ILI9488_SetRotation(3);

    ILI9488_PrintText(50,100,"GROUND PENETRATING RADAR ",0xFFE0, 0x0000);

    HAL_Delay(50);

    ILI9488_PrintText(120,120,"MON-12 ",0xFFE0, 0x0000);

    HAL_Delay(1000);

    ILI9488_SetCursor(0,0,320,240);
    ILI9488_DrawColor(0x0000);//Full black screen
    HAL_Delay(500);

    float max_amplitude = 12000; //maximum FFT magnitude value (for scaling)
    float bandwidth = 300;
    float resolution;
    float min_depth;
    float cable_offset = 10; //resolution multiples
    //set propagation media
    float propagation_velocity = 3;

    ILI9488_PrintText(50,100,"PROPAGATION VELOCITY : ",0xFFE0, 0x0000);
    HAL_Delay(50);

    DisplayFloat(propagation_velocity, 100, 120, 0x07E0, 0x0000);  // Green text on black background
    HAL_Delay(50);

    ILI9488_PrintText(140,120," m/s ",0x07E0, 0x0000);

    resolution = propagation_velocity / (2.0 * bandwidth); //depth resolution [m]
    min_depth = -1.0 * cable_offset * resolution; //offset cables + antennas

    // set depths=resolution multiples
    HAL_Delay(3000);

    ILI9488_SetCursor(0,0,320,240);
    ILI9488_DrawColor(0x0000);//Full black screen
    HAL_Delay(500);


    float depths = 30;

    ILI9488_PrintText(120,100,"DEPTHS : ",0xFFE0, 0x0000);
    HAL_Delay(50);

    DisplayFloat(depths, 115, 120, 0x07E0, 0x0000);  // Green text on black background
    HAL_Delay(50);

    float no_res_cells_vert = depths;
   // float max_depth = no_res_cells_vert * resolution + min_depth;//actual depth from the surface
   // float depth_step_temp = (max_depth - min_depth) / 4;

   //set distance

          HAL_Delay(3000);

          ILI9488_SetCursor(0,0,320,240);
          ILI9488_DrawColor(0x0000);//Full black screen
          HAL_Delay(500);


          float distance = 30;

          ILI9488_PrintText(120,100,"DISTANCE : ",0xFFE0, 0x0000);
          HAL_Delay(50);

          DisplayFloat(distance, 115, 120, 0x07E0, 0x0000);  // Green text on black background
          HAL_Delay(50);

          ILI9488_PrintText(150,120," m ",0x07E0, 0x0000);

          //graph parameters
          float graph_width = 288, graph_height = 208;
          float GPR_horizontal_step = 0.12;  //GPR horizontal step [m]
          float res_cell_height = graph_height / no_res_cells_vert;//[pixels]available for each resolution step for depth
          float no_res_cells_horiz = distance / GPR_horizontal_step;
          float res_cell_width = graph_width / no_res_cells_horiz;  //[pixels]available for each resolution step for distance

          ILI9488_SetCursor(0,0,320,240);
          ILI9488_DrawColor(0x0000);  // Full black screen
          HAL_Delay(50);
          //ILI9488_DrawLine(0, 310, 320, 310, 0xF800);  // Red line
          //ILI9488_DrawLine(10, 0, 10, 2400, 0x07E0); // Green line
          DrawGraphAxes(32, 32, 288, 208, "GPR", "Distance [m]", "Depth [m]", 0xFFFF);  // White axes
          HAL_Delay(5000);

          float real[256], imag[256];
          float samples[256] = {0};
          float fft_input[2 * FFT_SIZE];  // Interleaved complex input
          float magnitude[FFT_SIZE];      // FFT magnitude output

          uint16_t scan_index = 0;

          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

	    while (scan_index < no_res_cells_horiz) {


	    		uint16_t x;
	    		float freq[256];
	    		float L = 3.9e-9;
	    		freq[0]=430.9e6;
	    		float pi=3.14;
	    		float delta_f = 1.3199e6;
	    		float C;
	    		int j = 0;


	    		uint16_t i = 270;
	    	      while ( i <= 4095) {
	    	            MCP4725_WriteDAC(i);
	    	            HAL_Delay(10);
	    	            float fr = freq[j+1]=freq[j]+delta_f;

	    	            C = 1 / (2 * L * pow(pi * fr, 2));
	    	           float C_new = C/(1e-12);
	    	           float V_vco=-(C_new-69.685)/8.7933;
	    	           float V_dac = V_vco/1.2;
	    	           x = (uint16_t)((V_dac / 5) * 4095.0f);
	    	           i = x;

	    	           samples[j] = Read_ADC_Voltage();
	    	           HAL_Delay(500);

	    	           j = j + 1 ;

	    	        }

	    	      for (i = 0; i < 256 ; i++)
	    	      {
	    	        real[i] = (double)(samples[i]) - 512.0; // Load samples and eliminate d.c.
	    	        imag[i] = 0.0;                          // Delete imaginary part
	    	      }


	    	      for (int i = 0; i < FFT_SIZE; i++) {
	    	             fft_input[2 * i] = real[i];        // Real part
	    	             fft_input[2 * i + 1] = imag[i];    // Imaginary part
	    	         }

	    	      arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_input, 0, 1);

	    	     // Compute magnitude
	    	      arm_cmplx_mag_f32(fft_input, magnitude, FFT_SIZE);




	    	    uint16_t color;
	    	    float depth_corrected_amplitude;
	    	    uint8_t scaled_amplitude;
	    	    uint16_t orig_x = 32;
	    	    uint16_t orig_y = 32;


	    	 for (uint16_t i = 0; i < no_res_cells_vert; i++) {
	    	        // Depth correction
	    	        depth_corrected_amplitude = magnitude[i] * expf(i * 0.025f);

	    	        // Scale to 0-255
	    	        scaled_amplitude = (uint8_t)(depth_corrected_amplitude * 255.0f / max_amplitude);
	    	        if (scaled_amplitude > 255) scaled_amplitude = 255;

	    	        // RGB565 color generation
	    	        color = (((scaled_amplitude & 0b11111000) << 8) |  // Red
	    	                 ((scaled_amplitude & 0b11111100) << 3) |  // Green
	    	                 (scaled_amplitude >> 3));                 // Blue

	    	        // Fill the region: width × height block
	    	        for (uint16_t dx = 0; dx < res_cell_width; dx++) {
	    	            for (uint16_t dy = 0; dy < res_cell_height; dy++) {
	    	                ILI9488_DrawPixel(orig_x + dx, orig_y + dy, color);
	    	            }
	    	        }

	    	        // Move origin vertically down for next cell
	    	        orig_y += res_cell_height;
	    	    }

	    	 HAL_Delay(1000);

	    	for (uint16_t j = 0; j < 7; j++)
	    	            	     {
	    	            		     uint16_t delay_ms = 125 ; // 14.4 deg in 2 sec

	    	            	         HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);   // STEP high
	    	            	         HAL_Delay(delay_ms);
	    	            	         HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // STEP low
	    	            	         HAL_Delay(delay_ms);

	    	        }

	    	scan_index++;

	    }

	    while(1){}

}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
