#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_spiffs.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "Touch_Manager.h"

///////////////////////////////////////////////////////////////

// ------------------------------------------------------------
// 開始
extern "C" void app_main()
{
    // 開始
    printf( "---- Program Start ----\n" );

    // タッチパネル初期化
    TouchManager tch;
    tch.Initialize();

    // 入力デモ
    for (;;) {

        // 確認
        if ( tch.Scan() ) {
            // タッチがあった場合
            TOUCHDATA tt;
            if ( tch.GetPoint( 0, tt ) ) {
                // 座標を取得できた場合

                // 出力
                printf( "(%d, %d)\n", tt.PosX, tt.PosY );
            }
        }

        vTaskDelay( pdMS_TO_TICKS(10) );
    }

    // 終了
    printf( "---- Program End ----\n" );
    for(;;) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
}
