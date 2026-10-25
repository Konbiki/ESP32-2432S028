#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_spiffs.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////

// SPIFFS領域の設定
esp_vfs_spiffs_conf_t gSPIFFSConf = {
    .base_path = "/spiffs",             // アクセス用パス
    .partition_label = "spiffs",        // spiffsパーティション名
    .max_files = 5,                     // 最大ファイル数
    .format_if_mount_failed = false     // true:マウント失敗時に、自動フォーマット
};

///////////////////////////////////////////////////////////////

// ------------------------------------------------------------
// 開始
extern "C" void app_main()
{
    // 開始
    printf( "---- Program Start ----\n" );

    // CPU情報出力
    uint32_t cpuclock = esp_rom_get_cpu_ticks_per_us();
    printf( "CPU Clock : %lu [MHz]\n", cpuclock );

    // メモリ情報出力
    size_t sram_total = heap_caps_get_total_size( MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT );
    size_t sram_free = heap_caps_get_free_size( MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT );
    printf( "SRAM   : total = %8d, used = %8d, free = %8d\n", sram_total, sram_total - sram_free, sram_free );
    size_t psram_total = heap_caps_get_total_size( MALLOC_CAP_SPIRAM );
    size_t psram_free = heap_caps_get_free_size( MALLOC_CAP_SPIRAM );
    printf( "PSRAM  : total = %8d, used = %8d, free = %8d\n", psram_total, psram_total - psram_free, psram_free );

    // SPIFFS情報出力
    esp_err_t rc = esp_vfs_spiffs_register( &gSPIFFSConf );
    if ( rc == ESP_ERR_NOT_FOUND ) {
        // SPIFFS領域がない場合
        printf("SPIFFS : not found '%s'\n", gSPIFFSConf.partition_label );

    } if ( rc != ESP_OK ) {
        // マウントできない場合
        printf("SPIFFS : mount '%s' failed rc=(%d)(0x%08X)\n", gSPIFFSConf.partition_label, rc, rc );

    } else {
        // 成功した場合
        size_t spiffs_total = 0;
        size_t spiffs_used = 0;
        if ( esp_spiffs_info(gSPIFFSConf.partition_label, &spiffs_total, &spiffs_used) == ESP_OK )  {
            // 情報取得に成功した場合
            printf( "SPIFFS : total = %8d, used = %8d, free = %8d\n", spiffs_total, spiffs_used, spiffs_total - spiffs_used );
        } else {
            // 情報取得に失敗した場合
            printf( "SPIFFS : Unkown\n" );
        }
    }

    // 終了
    printf( "---- Program End ----\n" );
    for(;;) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
}
