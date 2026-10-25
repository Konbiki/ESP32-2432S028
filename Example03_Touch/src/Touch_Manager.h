///////////////////////////////////////////////////////////////
// タッチコントローラ(XPT2046)　制御モジュール
///////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <stdint.h>

///////////////////////////////////////////////////////////////

#if !defined(TOUCH_ERRORPRINT)
#define TOUCH_ERRORPRINT(...)	    printf(__VA_ARGS__)
#endif

///////////////////////////////////////////////////////////////

#define TOUCH_MAX_COUNT                 (1)             // 最大タッチ数

#define TOUCH_WIDTH                     (240)           // 横方向(短い方)ピクセル数
#define TOUCH_HEIGHT                    (320)           // 縦方向(長い方)ピクセル数

#define TOUCH_PIN_MOSI                  (GPIO_NUM_32)   // SPI MOSI ピン番号
#define TOUCH_PIN_MISO                  (GPIO_NUM_39)   // SPI MISO ピン番号
#define TOUCH_PIN_SCLK                  (GPIO_NUM_25)   // SPI SCLK ピン番号
#define TOUCH_PIN_CS                    (GPIO_NUM_33)   // SPI CS ピン番号

#define TOUCH_PIN_IRQ                   (GPIO_NUM_36)   // IRQ ピン番号

#define TOUCH_SPI_FREQUENCY             (2000000)       // SPI速度 2MHz
#define TOUCH_SPI_PORT                  (SPI2_HOST)     // SPIポート
#define TOUCH_SPI_MAX_TRANSBYTE         (2048)          // 転送最大バイト数
#define TOUCH_SPI_MAX_TRANSWORD         ((TOUCH_SPI_MAX_TRANSBYTE)/2) // 最大転送ワード数

// XPT2046コマンド
#define TOUCH_XPT2046_START             (0x80)
#define TOUCH_XPT2046_CHANNEL_X         (0x50)  // Y軸読み取り
#define TOUCH_XPT2046_CHANNEL_Y         (0x10)  // X軸読み取り
#define TOUCH_XPT2046_8BIT              (0x08)  // 0=12bit, 1=8bit
#define TOUCH_XPT2046_SER               (0x04)  // 0=Differential, 1=Single-Ended
#define TOUCH_XPT2046_PD0               (0x00)  // パワーダウンモード設定
#define TOUCH_XPT2046_PD1               (0x01)

///////////////////////////////////////////////////////////////

// タッチ座標情報
typedef struct 
{
    uint8_t     ID;     // 識別ID
    uint16_t    PosX;   // スクリーンX座標
    uint16_t    PosY;   // スクリーンY座標
    uint16_t    RawX;   // タッチセンサ値X
    uint16_t    RawY;   // タッチセンサ値Y
    uint16_t    Size;   // サイズ
} TOUCHDATA;

///////////////////////////////////////////////////////////////

// ============================================================
// タッチパネル制御クラス
class TouchManager
{
protected:
    uint16_t    mScreenL;   // タッチ可能領域 左端
    uint16_t    mScreenT;   // タッチ可能領域 上端
    uint16_t    mScreenR;   // タッチ可能領域 右端
    uint16_t    mScreenB;   // タッチ可能領域 下端

    uint8_t     mTouchCount;    // タッチ数
    TOUCHDATA   mTouch[TOUCH_MAX_COUNT];   // タッチ座標

    spi_device_handle_t     mSPI;   // SPIハンドラ

public:

    // --------------------------------------------------------
    // コンストラクタ
    TouchManager()
    {
        // 初期化
        mScreenL = 3700;
        mScreenR = 450;
        mScreenT = 300;
        mScreenB = 3800;
        mTouchCount = 0;
        mSPI = nullptr;
    }


    // --------------------------------------------------------
    // 初期化
    void Initialize()
    { 
        // SPI初期化
        InitializeSPI();
    }


    // --------------------------------------------------------
    // タッチ検査
    //   戻り値 : タッチがあった数をかえす。(タッチがなかった場合、0を返す)
    // 補足
    // ・短期間(10ms未満)での連続呼出しを行った場合、正しく取得されない場合がある。
    uint8_t Scan()
    {
        // 状態確認
        if ( mSPI == nullptr ) return 0; // 初期化が完了していない場合、タッチなし

        // 取得
        uint16_t px = XPT2046_Read( TOUCH_XPT2046_START | TOUCH_XPT2046_CHANNEL_X );
        uint16_t py = XPT2046_Read( TOUCH_XPT2046_START | TOUCH_XPT2046_CHANNEL_Y );
        if ( (px == 0) || (py == 0) || (px >= 4095) || (py >= 4095) ) {
            // 範囲外の場合

            // タッチなし
            mTouchCount = 0;

        } else {
            // 範囲内の場合

            // タッチあり
            mTouchCount = 1;

            // 格納
            mTouch[0].ID = 0; // 固定
            mTouch[0].PosX = Normalize( px, mScreenL, mScreenR, 0, TOUCH_WIDTH - 1 );
            mTouch[0].PosY = Normalize( py, mScreenT, mScreenB, 0, TOUCH_HEIGHT - 1 );
            mTouch[0].RawX = px;
            mTouch[0].RawY = py;
            mTouch[0].Size = 1; // 固定
        }

        return mTouchCount;
    }


    // --------------------------------------------------------
    // タッチ情報取得
    //   Index      インデックス(0～TOUCH_MAX_COUNT-1)
    //   Touch      タッチ情報格納先
    //   戻り値 : 格納された場合、trueを返す。
    bool GetPoint( int Index, TOUCHDATA& Touch )
    {
        // 引数チェック
        if ( (Index < 0) || (mTouchCount <= Index) ) return false; // インデックスが不正な場合、失敗

        // 格納
        Touch = mTouch[Index];

        // 成功
        return true;
    }


    // --------------------------------------------------------
    // タッチ待ち
    //   Timeout    タイムアウト時間[ミリ秒](負数の場合、タイムアウトなし)
    //   戻り値 : タッチがあった場合、trueを返す。タイムアウトとなった場合、falseを返す。
    bool Wait( int Timeout )
    {
        int64_t st = esp_timer_get_time(); // 開始時刻

        // 押下待ち
        for(;;) {
            if ( Scan() != 0 ) break; // 入力があった場合、次へ
            if ( Timeout >= 0 ) {
                // タイムアウトが有効な場合
                int64_t df = (esp_timer_get_time() - st) / 1000;
                if ( df >= (int64_t)Timeout ) return false; // タイムアウトになった場合、タッチなし(終了)
            }
            vTaskDelay( pdMS_TO_TICKS(10) );
        }

        // 開放待ち
        TOUCHDATA tt = mTouch[0]; // 退避
        for(;;) {
            if ( Scan() == 0 ) break; // 入力なしの場合、次へ
            vTaskDelay( pdMS_TO_TICKS(10) );
        }

        // タッチ座標を復旧
        mTouch[0] = tt;
        mTouchCount = 1;

        // タッチあり
        return true;
    }


    // --------------------------------------------------------
    // タッチ待ち(タイムアウトなし)
    void Wait()
    {
        // タイムアウトなしでタッチ待ち
        (void)Wait( -1 );
    }


    // --------------------------------------------------------
    // 補正用パラメタの設定
    //   ScreenL        左端値
    //   ScreenR        右端値
    //   ScreenT        上端値
    //   ScreenB        下端値
    void SetScreen( uint16_t ScreenL, uint16_t ScreenR, uint16_t ScreenT, uint16_t ScreenB )
    {
        // 更新
        mScreenL = ScreenL;
        mScreenR = ScreenR;
        mScreenT = ScreenT;
        mScreenB = ScreenB;
    }


protected:


    // --------------------------------------------------------
    // SPI初期化
    void InitializeSPI()
    {
        // バス初期化
        mSPI = nullptr;
        spi_bus_config_t buscfg = {
            .mosi_io_num = TOUCH_PIN_MOSI,
            .miso_io_num = TOUCH_PIN_MISO,
            .sclk_io_num = TOUCH_PIN_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
#if 1 // platformio.init内の platform が 6.12.0 の場合            
            .data_io_default_level = false,
            .max_transfer_sz = 8,
            .flags = 0,
            .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#else // platformio.init内の platform が 6.5.0 の場合            
            .max_transfer_sz = 8,
            .flags = 0,
            .isr_cpu_id = INTR_CPU_ID_AUTO,
#endif
            .intr_flags = 0
        };
        esp_err_t ret = spi_bus_initialize( TOUCH_SPI_PORT, &buscfg, SPI_DMA_CH_AUTO );
        if ( ret != ESP_OK ) {
            // 初期化できない場合
            TOUCH_ERRORPRINT( "spi_bus_initialize() failed  rc=0x%x\n", ret );
            return; // 失敗
        }

        // バス割り当て
        spi_device_interface_config_t devcfg;
        memset(&devcfg, 0, sizeof(devcfg));
        devcfg.clock_speed_hz = TOUCH_SPI_FREQUENCY;
        devcfg.queue_size = 3;
        devcfg.mode = 0;
        devcfg.spics_io_num = TOUCH_PIN_CS;
        ret = spi_bus_add_device( TOUCH_SPI_PORT, &devcfg, &mSPI );
        if ( ret != ESP_OK ) {
            // 追加できない場合
            TOUCH_ERRORPRINT( "spi_bus_add_device() failed  rc=0x%x\n", ret );
            return; // 失敗
        }
    }


    // --------------------------------------------------------
    // レジスタ値読み込み
    //   Cmd        制御バイト値
    uint16_t XPT2046_Read( uint8_t Cmd )
    {
        uint8_t tx_buf[3] = { Cmd, 0x00, 0x00 }; // 送信データ
        uint8_t rx_buf[3] = {0}; // 受信データ格納先

        // SPI送受信
        spi_transaction_t t = {
            .flags = 0,
            .cmd = 0,
            .addr = 0,
            .length = 24, 
            .rxlength = 0,
            .override_freq_hz = 0,
            .user = NULL,
            .tx_buffer = tx_buf,
            .rx_buffer = rx_buf,
        };
        esp_err_t ret = spi_device_transmit(mSPI, &t);
        if (ret != ESP_OK) {
            TOUCH_ERRORPRINT( "spi_device_transmit() failed  rc=0x%x\n", ret );
            return 0;
        }

        
        uint16_t raw_val = ((rx_buf[1] << 8) | rx_buf[2]) >> 3;
        return raw_val & 0x0FFF;
    }


    // --------------------------------------------------------
    // 正規化
    //   Src                入力値
    //   SrcMin, SrcMax     入力値の最小/最大
    //   DstMin, DstMax     出力値の最小/最大
    //   戻り値 : 入力値を出力値範囲に正規化した値を返す。
    uint16_t Normalize( uint16_t Src, uint16_t SrcMin, uint16_t SrcMax, uint16_t DstMin, uint16_t DstMax )
    {
        uint16_t wk = 0;

        if ( SrcMax < SrcMin ) {
            // 入力値範囲が反転している場合

            // 最小/最大入れ替え
            Src = SrcMin - Src + SrcMax;
            wk = SrcMin;
            SrcMin = SrcMax;
            SrcMax = wk;
        }

        if ( DstMax < DstMin ) {
            // 出力値範囲が反転している場合

            // 最小/最大入れ替え
            wk = DstMin;
            DstMin = DstMax;
            DstMax = wk;
        }

        // 範囲外判定
        if ( Src <= SrcMin ) return DstMin; // 入力最小値以下の場合、出力最小値
        if ( SrcMax <= Src ) return DstMax; // 入力最大値以上の場合、出力最大値

        // 変換
        return (DstMax - DstMin) * (Src - SrcMin) / (SrcMax - SrcMin) + DstMin;
    }

};
