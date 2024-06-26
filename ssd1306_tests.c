/*
 * Copyright (C) 2021 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
 
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"

const unsigned char q[] = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF,
0xFE, 0x7F, 0xFF, 0x03, 0xFF, 0xFF, 0xBF, 0xFF, 0xEF, 0xFF, 0xCF, 0xFF, 0xF9, 0xFF, 0xFF, 0xBF,
0xFE, 0x7F, 0xFC, 0x30, 0xFF, 0xBF, 0xBF, 0xFF, 0xF3, 0xFF, 0xCF, 0xFF, 0xF6, 0xFF, 0xFD, 0xDF,
0xFC, 0xFF, 0xF8, 0xFC, 0xFF, 0xBF, 0x9F, 0xD8, 0xF1, 0xF6, 0x39, 0x9E, 0x1B, 0x87, 0xF9, 0xE7,
0xFD, 0x5F, 0xFB, 0x8E, 0x7E, 0x9F, 0xAF, 0xC1, 0xF3, 0xFD, 0xDF, 0x7F, 0x7F, 0xAF, 0xF2, 0x33,
0xF2, 0x0F, 0xFF, 0x83, 0x7E, 0x5F, 0x4F, 0x8F, 0xF3, 0xFB, 0x39, 0xFF, 0x6F, 0xEF, 0xF1, 0xDB,
0xE1, 0xA7, 0xFF, 0xF9, 0x3D, 0x3E, 0x97, 0x9F, 0xE3, 0xFE, 0xFD, 0xBF, 0xF9, 0xFF, 0xEB, 0x59,
0xD7, 0x6B, 0xF3, 0xFC, 0xBC, 0xEE, 0xE7, 0xBF, 0x73, 0xFA, 0xFD, 0xBF, 0xFF, 0xFF, 0xEE, 0xD5,
0xE7, 0xB7, 0xFF, 0xCE, 0xBE, 0x1F, 0x8F, 0xBF, 0xFF, 0xFB, 0xFF, 0xFF, 0x9B, 0x9F, 0xE6, 0xE7,
0xF5, 0xB7, 0xFF, 0xA6, 0x7F, 0xFF, 0xFF, 0xFA, 0xFF, 0xFD, 0xFF, 0x7F, 0x7F, 0xEF, 0xF6, 0xD7,
0xDD, 0xEB, 0xFF, 0xF7, 0x7F, 0xFB, 0xFF, 0xFC, 0xFF, 0xFE, 0xFC, 0xFE, 0x19, 0xC7, 0xFB, 0x27,
0xF7, 0xDF, 0xFF, 0xFD, 0xFF, 0xF4, 0xFF, 0xFC, 0x7F, 0xFF, 0x33, 0xFF, 0xFF, 0xFF, 0xFD, 0xEF,
0xFD, 0xDF, 0xFF, 0xF9, 0xFF, 0xFD, 0xFF, 0xFF, 0x07, 0xFF, 0xCF, 0xFF, 0xF9, 0xFF, 0xFE, 0xDF,
0xFE, 0x7F, 0xFF, 0x87, 0xFF, 0xFB, 0xFF, 0xFF, 0x9F, 0xFF, 0xEF, 0xFF, 0xF9, 0xFF, 0xFF, 0x7F,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// void ssd1306_TestBitmap(void) {
//   ssd1306_Fill(Black);
//   ssd1306_DrawBitmap(Untitled, sizeof(Untitled));
//   ssd1306_UpdateScreen();
// }
// void ssd1306_TestBitmap2(void) {
//   ssd1306_Fill(Black);
//   ssd1306_DrawBitmap(Untitled2, sizeof(Untitled2));
//   ssd1306_UpdateScreen();
// }
void ssd1306_DrawBitmap2(const uint8_t* bitmap, uint32_t size)
{
    unsigned int c = 8;
    uint8_t rows = size * c / SSD1306_WIDTH;
    if (rows > SSD1306_HEIGHT) {
        rows = SSD1306_HEIGHT;
    }
    for (uint8_t x = 0; x < SSD1306_WIDTH; x++) {
        for (uint8_t y = 0; y < rows; y++) {
            uint8_t byte = bitmap[(y * SSD1306_WIDTH / c) + (x / c)];
            uint8_t bit = byte & (0x80 >> (x % c));
            ssd1306_DrawPixel(x, y+24, bit ?   Black:White);
            
        }
        if(x%3==0)
        ssd1306_UpdateScreen();
        if(x%8==0){
            HAL_Delay(200);
        }else{
            HAL_Delay(20);
        }
        if(x==121){
            HAL_Delay(2000);
        }
    }
}
void ssd1306_TestBitmap3(void) {
  //ssd1306_Fill(Black);
  ssd1306_DrawBitmap2(q, 256);
}
void ssd1306_TestAll(void) {
    ssd1306_Init();
    ssd1306_TestBitmap3();
}

