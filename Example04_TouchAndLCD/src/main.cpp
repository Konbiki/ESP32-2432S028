#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_spiffs.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "LCD_Manager.h"
#include "Touch_Manager.h"

///////////////////////////////////////////////////////////////

void TouchCalibration(LCDManager& Lcd, TouchManager& Tch);

///////////////////////////////////////////////////////////////

// ------------------------------------------------------------
// 開始
extern "C" void app_main()
{
    // 開始
    printf( "---- Program Start ----\n" );
    vTaskDelay( pdMS_TO_TICKS(3000) );

    // LCD初期化
    LCDManager lcd;
    lcd.Initialize();
    printf( "---- LCD initialized\n" );

    // タッチパネル初期化
    // vTaskDelay( 2000 ); // タッチパネルが認識されない場合、入れる
    TouchManager tch;
    tch.Initialize();

    // キャリブレーション
    TouchCalibration( lcd, tch );

    // 動作
    lcd.Clear( LCD_COLOR_BLACK );
    bool lasttouch = false; // 前回タッチ状況(true:タッチあり)
    int idx = 1; // 色番号
    const int MarkerSize = 4; // マーカーサイズ
    for(;;) {

        if ( tch.Scan() > 0 ) {
            // タッチがあった場合

            TOUCHDATA tt; // タッチ情報
            if ( tch.GetPoint( 0, tt ) ) {
                // 座標を取得できた場合

                // タッチ位置に矩形描画
                lcd.FillRectangle( 
                    tt.PosX - MarkerSize, tt.PosY - MarkerSize, 
                    MarkerSize * 2 + 1, MarkerSize * 2 + 1, 
                    LCDManager::GetColor(idx) );
            }
            lasttouch = true;

        } else {
            // タッチがなかった場合

            if ( lasttouch ) {
                // タッチありからタッチなしになった場合
                // 線の色変更
                idx++; 
                if ( idx == 0 ) idx = 1;
            }
            lasttouch = false;
        }

        vTaskDelay( pdMS_TO_TICKS(10) );
    }


    // 終了
    printf( "---- Program End ----\n" );
}


// ------------------------------------------------------------
// タッチパネルのキャリブレーション
//   Lcd        LCD制御
//   Tch        タッチパネル制御
void TouchCalibration(LCDManager& Lcd, TouchManager& Tch)
{
    int16_t markersize = 16;
    uint16_t x1, y1; // 左上タッチ座標
    uint16_t x2, y2; // 右上タッチ座標
    uint16_t x3, y3; // 右下タッチ座標
    uint16_t x4, y4; // 左下タッチ座標
    TOUCHDATA tt; // タッチ情報
    uint16_t sx, sy; // 画面サイズ
    uint16_t sl, sr, st, sb; // 補正値

    // 準備
    Lcd.GetSize( sx, sy );

    // 左上
    Lcd.Clear( LCD_COLOR_BLACK );
    Lcd.DrawRectangle( 0, 0, markersize, markersize, LCD_COLOR_GREEN );
    Tch.Wait();
    (void)Tch.GetPoint(0, tt);
    x1 = tt.RawX;
    y1 = tt.RawY;

    // 右上
    Lcd.Clear( LCD_COLOR_BLACK );
    Lcd.DrawRectangle( sx - markersize, 0, markersize, markersize, LCD_COLOR_GREEN );
    Tch.Wait();
    (void)Tch.GetPoint(0, tt);
    x2 = tt.RawX;
    y2 = tt.RawY;

    // 右下
    Lcd.Clear( LCD_COLOR_BLACK );
    Lcd.DrawRectangle( sx - markersize, sy - markersize, markersize, markersize, LCD_COLOR_GREEN );
    Tch.Wait();
    (void)Tch.GetPoint(0, tt);
    x3 = tt.RawX;
    y3 = tt.RawY;

    // 左下
    Lcd.Clear( LCD_COLOR_BLACK );
    Lcd.DrawRectangle( 0, sy - markersize, markersize, markersize, LCD_COLOR_GREEN );
    Tch.Wait();
    (void)Tch.GetPoint(0, tt);
    x4 = tt.RawX;
    y4 = tt.RawY;

    // 補正値設定
    sl = x1 < x4 ? x1:x4;
    sr = x2 < x3 ? x3:x2;
    st = y1 < y2 ? y1:y2;
    sb = y3 < y4 ? y4:y3;
    Tch.SetScreen( sl, sr, st, sb );
}
