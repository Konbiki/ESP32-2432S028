#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_spiffs.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include <math.h>

#include "LCD_Manager.h"

///////////////////////////////////////////////////////////////

extern const uint16_t gImgTest[];		// 画像データ

///////////////////////////////////////////////////////////////

// ------------------------------------------------------------
// 開始
extern "C" void app_main()
{
    // 開始
    printf( "---- Program Start ----\n" );

    // メモリ情報出力
    size_t sram_total = heap_caps_get_total_size( MALLOC_CAP_INTERNAL );
    size_t sram_free = heap_caps_get_free_size( MALLOC_CAP_INTERNAL );
    printf( "SRAM   : total = %8d, used = %8d, free = %8d\n", sram_total, sram_total - sram_free, sram_free );

    // LCD初期化
    LCDManager lcd;
    lcd.Initialize();

    // デモ描画
    uint16_t sx, sy; // 画面サイズ
    for ( int rot = 0; ; rot++ ) {

        // LCDの向きを設定
        printf( "Rotation : %d\n", rot % 4 );
        lcd.SetRotation( rot % 4 );

        // 描画
        lcd.Clear( LCD_COLOR_BLACK ); // 黒色で消去
        lcd.GetSize( sx, sy ); // 設定したLCD向きでの画面サイズ取得
        lcd.FillRectangle( 0, 0, 50, 30, LCD_COLOR_BLUE ); // 原点(0,0)(左上)に、50x30の青色矩形描画
        lcd.FillRectangle( sx - 30, 0, 30, 60, LCD_COLOR_RED ); // 右上に、30x60の赤色矩形描画
        lcd.DrawRectangle( 0, sy - 50, 50, 50, LCD_COLOR_GREEN ); // 左下に緑色の枠描画
        lcd.DrawImage( sx - 32, sy - 32, 32, 32, &gImgTest[0] ); // 右下に画像描画

        // 放射線描画
        float sz = static_cast<float>( (sx < sy) ? sx:sy ) / 2; // 画面サイズ左右の短い方
        for ( int rr = 0; rr < 360; rr += 5 ) { // 中心から放射線状の線描画
            float px = static_cast<int16_t>( static_cast<float>(sx / 2) + sz * cos( static_cast<float>(rr) * M_PI / 180.0f ));
            float py = static_cast<int16_t>( static_cast<float>(sy / 2) + sz * sin( static_cast<float>(rr) * M_PI / 180.0f ));
            lcd.DrawLine( sx / 2, sy / 2, px, py, LCD_COLOR_WHITE );
        }

        // タイミング調整
        vTaskDelay( pdMS_TO_TICKS(5000) ); // 5秒待機
    }

    // 終了
    printf( "---- Program End ----\n" );
    for(;;) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
}
