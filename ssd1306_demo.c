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

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#include "ssd1306.h"
#include "ssd1306_tests.h"

#include "hi_io.h"
#include "hi_adc.h"
#include "number.c"

#define OLED_I2C_BAUDRATE (400 * 1000)

int pressed = 0;
int nowSelect = 0;
int selected[7] = {0};
int goFloor[7] = {0};
int moving = 0;
int nowFloor = 0;
int now = 0;
int lasttime = 0;
int start = 0;

/**
 * 中断函数里无法直接读取全局变量，同时调用外部函数只能传参常量，于是有了这几个函数
*/
void setLast()
{
    lasttime = now;
}
int getLast()
{
    return lasttime;
}
int getNow()
{
    return now;
}
int getStart(){
    return start;
}

void directTip(int direct)
{
    if(direct){
        printf("Up!\n");
    }else{
        printf("Down!\n");
    }
}
void floorTip(int floor)
{
    printf("Floor:%d\n",floor);
}

void moveNumber(int from, int to)
{
    if (from < to)
    {
        directTip(1);
        drawArea(44, 15, arrays[0], 20, 20, 0); // 绘制上行箭头
        ssd1306_UpdateScreen();
        usleep(3000000);
        // 等待2s
        for (; from < to; from++)
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j <= 20; j++)
                {
                    drawArea(44, 15, arrays[0] + j * 20, 20, 20, 0); // 箭头向上滚动
                    ssd1306_UpdateScreen();
                    usleep(30000);
                }
            }
            drawArea(64, 15, bignum[from + 1], 20, 20, 0); // 绘制数字from+1
            ssd1306_UpdateScreen();
            nowFloor++;
        }
    }
    else
    {
        directTip(0);
        drawArea(44, 15, arrays[1], 20, 20, 0); // 绘制下行箭头
        ssd1306_UpdateScreen();
        usleep(3000000);
        // 等待2s
        for (; from > to; from--)
        {
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j <= 20; j++)
                {
                    drawArea(44, 15, arrays[1] + 400 - j * 20, 20, 20, 0); // 箭头向上滚动
                    ssd1306_UpdateScreen();
                    usleep(30000);
                }
            }
            drawArea(64, 15, bignum[from - 1], 20, 20, 0); // 绘制数字from-1
            ssd1306_UpdateScreen();
            nowFloor--;
        }
    }
}
void startMoving(void)
{
    int l = 1;
    goFloor[0] = nowFloor;
    moving=1;
    for (int i = nowFloor + 1; i < 7; i++)
    {
        if (selected[i])
        {
            goFloor[l++] = i;
        }
    }
    for (int i = nowFloor - 1; i >= 0; i--)
    {
        if (selected[i])
        {
            goFloor[l++] = i;
        }
    }
    for (int i = 0; i < l - 1; i++)
    {
        moveNumber(goFloor[i], goFloor[i + 1]);
        selected[goFloor[i+1]]=0;
        // 取消填充
        drawRectangle(6 + 17 * goFloor[i+1], 47, 5 + 17 * goFloor[i+1] + 10, 56, 0);
        // 绘制原来的样式
        drawArea(7 + 17 * goFloor[i+1], 48, numbers[goFloor[i+1]], 8, 8, 0);
        floorTip(nowFloor);
    }
    start = 0;
    moving=0;
    usleep(1000000);
    fillArea(44,15,20,20,0);
}

static float GetVoltage(void)
{
    unsigned int ret;
    unsigned short data;
    // 该函数通过使用hi_adc_read()函数来读取 HI_ADC_CHANNEL_2 的数值存储在data中，
    // HI_ADC_EQU_MODEL_8 表示8次平均算法模式，
    // HI_ADC_CUR_BAIS_DEFAULT 表示默认的自动识别模式，
    ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_2, &data,
                      HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("ADC Read Fail\n");
        return;
    }
    // 最后通过 data * 1.8 * 4 / 4096.0 计算出实际的电压值。
    return (float)data * 1.8 * 4 / 4096.0;
}

/**
 * 在指定坐标处绘制bitmap中的数据
 * @param xoffset x坐标偏移量(0-127)
 * @param yoffset y坐标偏移量(0-63)
 * @param bitmap 二维点阵数组(0:亮,1:灭)
 * @param height 绘制的高度
 * @param width 绘制的宽度
 * @param color 颜色反转(0:不反转,1:反转)
 */
void drawArea(int xoffset, int yoffset, const uint8_t *bitmap, int height, int width, int color)
{
    for (uint8_t y = 0; y < height; y++)
    {
        for (uint8_t x = 0; x < width; x++)
        {
            uint8_t byte = bitmap[(y * width) + (x)];
            ssd1306_DrawPixel(x + xoffset, y + yoffset, byte ^ color ? White : Black);
        }
    }
}

/**
 * 在指定坐标处绘制矩形边框
 * @param x 矩形左上角横坐标(0-127)
 * @param y 矩形左上角纵坐标(0-63)
 * @param tx 矩形右下角横坐标(0-127)
 * @param ty 矩形右下角纵坐标(0-63)
 * @param color 颜色(0:灭,1:亮)
 */
void drawRectangle(int x, int y, int tx, int ty, int color)
{
    for (uint8_t i = x; i <= tx; i++)
    {
        for (uint8_t j = y; j <= ty; j++)
        {
            if (i == x || i == tx || j == y || j == ty)
            {
                ssd1306_DrawPixel(i, j, color ? White : Black);
            }
        }
    }
}

/**
 * 移动矩形边框切换待选楼层
 */
void moveSelector(void)
{
    if(moving){
        return;
    }
    drawRectangle(5 + 17 * nowSelect, 46, 5 + 17 * nowSelect + 11, 57, 0);
    nowSelect++;
    nowSelect %= 7;
    drawRectangle(5 + 17 * nowSelect, 46, 5 + 17 * nowSelect + 11, 57, 1);
}

/**
 * 选择/取消选择当前所在的楼层
 */
void selectNumber(void)
{
    printf("start:%d\n",start);
    if(moving){
        return;
    }
    if (nowSelect == nowFloor)
    {
        selected[nowSelect] = 1;    //好像可以直接return
    }
    // 其实可以不写if(
    if (selected[nowSelect] == 0)
    {
        // 填充与选择光标之间的间隙
        drawRectangle(6 + 17 * nowSelect, 47, 5 + 17 * nowSelect + 10, 56, 1);
        selected[nowSelect] = 1;
        // 绘制反色后的数字
        drawArea(7 + 17 * nowSelect, 48, numbers[nowSelect], 8, 8, 1);
    }
    else
    {
        // 取消填充
        drawRectangle(6 + 17 * nowSelect, 47, 5 + 17 * nowSelect + 10, 56, 0);
        // 绘制原来的样式
        drawArea(7 + 17 * nowSelect, 48, numbers[nowSelect], 8, 8, 0);
        selected[nowSelect] = 0;
    }
}

/**
 * 按下按钮时触发的中断函数
 */
static void OnButtonPressed(char *arg)
{
    int n = getNow();
    int l = getLast();
    if (n - l <= 100)
    {
        return;
    }
    else
    {
        setLast();
    }
    (void *)arg;
    float v = GetVoltage();
    printf("%f\n", v);
    if (v >= 0.2)
    {
        // s1 && s2
        moveSelector();
    }
    else
    {
        // user
        selectNumber();
    }
    start = 1;
}

/**
 * 填充矩形区域
 * @param xoffset x坐标偏移量(0-127)
 * @param yoffset y坐标偏移量(0-127)
 * @param height 绘制的高度
 * @param width 绘制的宽度
 * @param color 颜色(0:灭,1:亮)
 */
void fillArea(int xoffset, int yoffset, int height, int width, int color)
{
    for (uint8_t y = 0; y < height; y++)
    {
        for (uint8_t x = 0; x < width; x++)
        {
            ssd1306_DrawPixel(x + xoffset, y + yoffset, color ? White : Black);
        }
    }
}

/**
 * 绘制电梯楼层数字
 */
void showNumbers(void)
{
    int i;
    int pos = 7;
    for (i = 0; i < 7; i++)
    {
        drawArea(pos, 48, numbers[i], 8, 8, 0);
        pos += 17;
    }
}

void Ssd1306TestTask(void *arg)
{
    (void)arg;
    IoTGpioInit(HI_IO_NAME_GPIO_13);
    IoTGpioInit(HI_IO_NAME_GPIO_14);

    hi_io_set_func(HI_IO_NAME_GPIO_13, HI_IO_FUNC_GPIO_13_I2C0_SDA);
    hi_io_set_func(HI_IO_NAME_GPIO_14, HI_IO_FUNC_GPIO_14_I2C0_SCL);

    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    IoTGpioInit(5);
    hi_io_set_func(5, HI_IO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(5, IOT_GPIO_DIR_IN);
    hi_io_set_pull(5, HI_IO_PULL_UP);

    IoTGpioRegisterIsrFunc(5, IOT_INT_TYPE_EDGE,
                           IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                           OnButtonPressed, NULL);
    // WatchDogDisable();
    usleep(20 * 1000);
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    fillArea(0, 24, 80, 128, 0);
    showNumbers();
    drawRectangle(5 + 17 * nowSelect, 46, 5 + 17 * nowSelect + 11, 57, 1);
    drawArea(64, 15, bignum[0], 20, 20, 0); // 绘制数字1
    // moveNumber(0, 6);
    while (1)
    {
        //printf("%d %d\n", now, lasttime);
        now = HAL_GetTick();
        if (now - lasttime > 2000 && start)
        {
            startMoving();
        }
        ssd1306_UpdateScreen();
        usleep(10000);
    }
}

void Ssd1306TestDemo(void)
{
    osThreadAttr_t attr;
    attr.name = "Ssd1306Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;
    if (osThreadNew(Ssd1306TestTask, NULL, &attr) == NULL)
    {
        printf("[Ssd1306TestDemo] Falied to create Ssd1306TestTask!\n");
    }
}
APP_FEATURE_INIT(Ssd1306TestDemo);