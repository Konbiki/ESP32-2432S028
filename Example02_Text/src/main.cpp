#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_spiffs.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "LCD_Manager.h"
#include "LCD_TextWriter.h"
#include "LCD_Console.h"

///////////////////////////////////////////////////////////////

extern const FONTDATA FontShinonome16; // フォントデータ

// 表示データ
const char* TextData[] = {
    "\033[39m枕草子\n",
    "\n",
    "\033[32m春はあけぼの。\n",
    "やうやうしろくなりゆく山ぎは、",
    "すこしあかりて、",
    "紫だちたる雲のほそくたなびきたる。\n",
    "\n",
    "\033[36m夏は夜。\n",
    "月の頃はさらなり、",
    "闇もなほ、",
    "蛍のおほく飛びちがひたる。\n",
    "また、",
    "ただ一つ二つなど、",
    "ほのかにうち光りて行くも、",
    "をかし。\n",
    "雨など降るも、", 
    "をかし。\n",
    "\n",
    "\033[33m秋は夕暮れ。\n",
    "夕日のさして、",
    "山の端いと近くなりたるに、",
    "烏(からす)の、",
    "寝(ねどころ)へ行くとて、",
    "三つ四つ、",
    "二つ三つなど、",
    "飛び急ぐさへ、",
    "あはれなり。\n",
    "まいて、",
    "雁(かり)などのつらねたるが、", 
    "いと小さく見ゆるは、",
    "いとをかし。\n",
    "日入りはてて、",
    "風の音、",
    "虫の音など、",
    "はた、",
    "言ふべきにあらず。\n",
    "\n",
    "\033[38m冬はつとめて。\n",
    "雪の降りたるは、",
    "言ふべきにもあらず。\n"
    "霜のいと白きも、",
    "またさらでも、",
    "いと寒きに、",
    "火など急ぎおこして、",
    "炭持てわたるも、",
    "いとつきづきし。\n", 
    "昼になりて、",
    "ぬるくゆるびもていけば、",
    "火桶(ひおけ)の火も、",
    "白き灰がちになりて、",
    "わろし。\n"
};
const int TextDataLineCount = sizeof(TextData) / sizeof(TextData[0]); // 行数


///////////////////////////////////////////////////////////////

// ------------------------------------------------------------
// 開始
extern "C" void app_main()
{
    // 開始
    printf( "---- Program Start ----\n" );

    // LCD初期化
    LCDManager lcd;
    lcd.Initialize();

    // 文字描画準備
    TextWriter wrt( lcd, &FontShinonome16 );

    // コンソール準備
    ConsoleManager con( wrt );
    con.Setup(); // 全体を使用

    // デモ表示
    for ( int rot = 0; ; rot++ ) {

        // LCDの向きを設定
        printf( "Rotation : %d\n", rot % 4 );
        lcd.SetRotation( rot % 4 );
        con.Setup();

        // １行づつ描画
        for ( int idx = 0; idx < TextDataLineCount; idx++ ) {

            // 描画
            con.Print( TextData[idx] );
            con.Refresh();

            // 待機
            vTaskDelay( pdMS_TO_TICKS(1000) );
        }

        // キャラアニメーション
        const char* waitchars[] = { "｜", "／", "ー", "＼", "｜", "／", "ー", "＼" };
        for ( int idx = 0; idx < 100; idx++ ) {
            int chridx = idx % (sizeof(waitchars) / sizeof(waitchars[0]));
            con.Printf( "%s\r", waitchars[chridx] );
            con.Refresh();
            vTaskDelay( pdMS_TO_TICKS(100) );
        }

    }

    // 終了
    printf( "---- Program End ----\n" );
    for(;;) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
}
