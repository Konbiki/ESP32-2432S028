///////////////////////////////////////////////////////////////
// LCD(ILI9341)　制御モジュール
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

#if !defined(LCD_ERRORPRINT)
#define LCD_ERRORPRINT(...)		printf(__VA_ARGS__)
#endif

///////////////////////////////////////////////////////////////

// ハードウェア情報定義
#define LCD_WIDTH               (240)           // 横方向(短い方)ピクセル数
#define LCD_HEIGHT              (320)           // 縦方向(長い方)ピクセル数

#define LCD_PIN_MOSI            (GPIO_NUM_13)   // SPI MOSI ピン番号
#define LCD_PIN_MISO            (GPIO_NUM_12)   // SPI MISO ピン番号(-1: MISO ピンは使用しない)
#define LCD_PIN_SCLK            (GPIO_NUM_14)   // SPI SCLK ピン番号
#define LCD_PIN_CS              (GPIO_NUM_15)   // SPI CS ピン番号
#define LCD_PIN_DC              (GPIO_NUM_2)    // データ/コマンド選択ピン番号
#define LCD_PIN_RESET           (-1)            // リセットピン番号
#define LCD_PIN_BACKLIGHT       (GPIO_NUM_21)   // バックライト制御ピン番号

#define LCD_SPI_FREQUENCY       (40000000)      // SPI速度 40MHz
#define LCD_SPI_PORT            (SPI3_HOST)     // SPIポート
#define LCD_SPI_MAX_TRANSBYTE   (2048)          // 転送最大バイト数
#define LCD_SPI_MAX_TRANSWORD   ((LCD_SPI_MAX_TRANSBYTE)/2) // 最大転送ワード数

// ILI9341コマンド
#define LCD_ILI9341_NOP			(0x00)	// No-op register
#define LCD_ILI9341_SWRESET		(0x01)	// Software reset register
#define LCD_ILI9341_RDDID		(0x04)	// Read display identification information
#define LCD_ILI9341_RDDST		(0x09)	// Read Display Status
#define LCD_ILI9341_SLPIN		(0x10)	// Enter Sleep Mode
#define LCD_ILI9341_SLPOUT		(0x11)	// Sleep Out
#define LCD_ILI9341_PTLON		(0x12)	// Partial Mode ON
#define LCD_ILI9341_NORON		(0x13)	// Normal Display Mode ON
#define LCD_ILI9341_RDMODE		(0x0A)	// Read Display Power Mode
#define LCD_ILI9341_RDMADCTL	(0x0B)	// Read Display MADCTL
#define LCD_ILI9341_RDPIXFMT	(0x0C)	// Read Display Pixel Format
#define LCD_ILI9341_RDIMGFMT	(0x0D)	// Read Display Image Format
#define LCD_ILI9341_RDSELFDIAG	(0x0F)	// Read Display Self-Diagnostic Result
#define LCD_ILI9341_INVOFF		(0x20)	// Display Inversion OFF
#define LCD_ILI9341_INVON		(0x21)	// Display Inversion ON
#define LCD_ILI9341_GAMMASET	(0x26)	// Gamma Set
#define LCD_ILI9341_DISPOFF		(0x28)	// Display OFF
#define LCD_ILI9341_DISPON		(0x29)	// Display ON
#define LCD_ILI9341_CASET		(0x2A)	// Column Address Set
#define LCD_ILI9341_PASET		(0x2B)	// Page Address Set
#define LCD_ILI9341_RAMWR		(0x2C)	// Memory Write
#define LCD_ILI9341_RAMRD		(0x2E)	// Memory Read
#define LCD_ILI9341_PTLAR		(0x30)	// Partial Area
#define LCD_ILI9341_VSCRDEF		(0x33)	// Vertical Scrolling Definition
#define LCD_ILI9341_MADCTL		(0x36)	// Memory Access Control
#define LCD_ILI9341_VSCRSADD	(0x37)	// Vertical Scrolling Start Address
#define LCD_ILI9341_PIXFMT		(0x3A)	// COLMOD: Pixel Format Set
#define LCD_ILI9341_FRMCTR1		(0xB1)	// Frame Rate Control (In Normal Mode/Full Colors)
#define LCD_ILI9341_FRMCTR2		(0xB2)	// Frame Rate Control (In Idle Mode/8 colors)
#define LCD_ILI9341_FRMCTR3		(0xB3)	// Frame Rate control (In Partial Mode/Full Colors)
#define LCD_ILI9341_INVCTR		(0xB4)	// Display Inversion Control
#define LCD_ILI9341_DFUNCTR		(0xB6)	// Display Function Control
#define LCD_ILI9341_PWCTR1		(0xC0)	// Power Control 1
#define LCD_ILI9341_PWCTR2		(0xC1)	// Power Control 2
#define LCD_ILI9341_PWCTR3		(0xC2)	// Power Control 3
#define LCD_ILI9341_PWCTR4		(0xC3)	// Power Control 4
#define LCD_ILI9341_PWCTR5		(0xC4)	// Power Control 5
#define LCD_ILI9341_VMCTR1		(0xC5)	// VCOM Control 1
#define LCD_ILI9341_VMCTR2		(0xC7)	// VCOM Control 2
#define LCD_ILI9341_RDID1		(0xDA)	// Read ID 1
#define LCD_ILI9341_RDID2		(0xDB)	// Read ID 2
#define LCD_ILI9341_RDID3		(0xDC)	// Read ID 3
#define LCD_ILI9341_RDID4		(0xDD)	// Read ID 4
#define LCD_ILI9341_GMCTRP1		(0xE0)	// Positive Gamma Correction
#define LCD_ILI9341_GMCTRN1		(0xE1)	// Negative Gamma Correction
#define LCD_ILI9341_IFCTL       (0xF6)  // Interface Control

// メモリアクセス設定
#define LCD_MADCTL_DEFAULT      (0x00)  // デフォルト
#define LCD_MADCTL_MY           (0x80)  // Y軸反転
#define LCD_MADCTL_MX           (0x40)  // X軸反転
#define LCD_MADCTL_MV           (0x20)  // XY軸交換
#define LCD_MADCTL_BGR          (0x10)  // BGR設定
#define LCD_MADCTL_MH           (0x08)  // 水平同期指示

// 色定義(RGB565:BigEndian)
#define LCD_COLOR(rr,gg,bb) static_cast<uint16_t>( \
            (((rr) & 0x1F) << 3) | \
            (((gg) & 0x07) << 13) | (((gg) >> 3) & 0x07) | \
            (((bb) & 0x1f) << 8) )     // 色コード作成(rr/bbは 0～31, ggは 0～63)
#define LCD_COLOR_BLACK	        LCD_COLOR(  0,  0,  0)
#define LCD_COLOR_BLUE			LCD_COLOR(  0,  0, 31)
#define LCD_COLOR_RED			LCD_COLOR( 31,  0,  0)
#define LCD_COLOR_MAGENTA		LCD_COLOR( 31,  0, 31)
#define LCD_COLOR_GREEN			LCD_COLOR(  0, 63,  0)
#define LCD_COLOR_CYAN			LCD_COLOR(  0, 63, 31)
#define LCD_COLOR_YELLOW		LCD_COLOR( 31, 63,  0)
#define LCD_COLOR_WHITE			LCD_COLOR( 31, 63, 31)
#define LCD_COLOR_DARKGRAY		LCD_COLOR(  8, 16,  8)
#define LCD_COLOR_DARKBLUE		LCD_COLOR(  0,  0, 16)
#define LCD_COLOR_DARKRED		LCD_COLOR( 16,  0,  0)
#define LCD_COLOR_DARKMAGENTA	LCD_COLOR( 16,  0, 16)
#define LCD_COLOR_DARKGREEN		LCD_COLOR(  0, 32,  0)
#define LCD_COLOR_DARKCYAN		LCD_COLOR(  0, 32, 16)
#define LCD_COLOR_DARKYELLOW	LCD_COLOR( 16, 32,  0)
#define LCD_COLOR_GRAY			LCD_COLOR( 16, 32, 16)

// 回転方向
#define LCD_ROTATE_NONE         (0)         // 回転なし
#define LCD_ROTATE_90           (1)         // 左90度(反時計方向)
#define LCD_ROTATE_180          (2)         // 180度
#define LCD_ROTATE_270          (3)         // 右90度(時計方向)


///////////////////////////////////////////////////////////////

// ============================================================
// LCDパネル制御クラス
class LCDManager
{
private:
    int8_t      mRot;               // 回転設定(0:回転なし、1:左90度回転、2:180度回転、右90度回転)
    int16_t     mLCDSizeX;          // 現在のLCDの横幅[pixel](回転により変化)
    int16_t     mLCDSizeY;          // 現在のLCDの縦幅[pixel](回転により変化)

    spi_device_handle_t     mSPI;   // SPIハンドラ


public:

    // カラーテーブル
    static constexpr uint16_t ColorTable[] = { 
        LCD_COLOR_BLACK,		// [0]  黒
        LCD_COLOR_BLUE,			// [1]  青
        LCD_COLOR_RED,			// [2]  赤
        LCD_COLOR_MAGENTA,		// [3]  紫
        LCD_COLOR_GREEN,		// [4]  緑
        LCD_COLOR_CYAN,			// [5]  シアン
        LCD_COLOR_YELLOW,		// [6]  黄
        LCD_COLOR_WHITE,		// [7]  白
        LCD_COLOR_DARKGRAY,		// [8]  暗い灰
        LCD_COLOR_DARKBLUE,		// [9]  暗い青
        LCD_COLOR_DARKRED,		// [10] 暗い赤
        LCD_COLOR_DARKMAGENTA,	// [11] 暗い紫
        LCD_COLOR_DARKGREEN,	// [12] 暗い緑
        LCD_COLOR_DARKCYAN,		// [13] 暗いシアン
        LCD_COLOR_DARKYELLOW,	// [14] 暗い黄
        LCD_COLOR_GRAY,			// [15] 明るい灰
    };

    // カラーテーブル個数
    static constexpr int ColorTableCount = sizeof(ColorTable) / sizeof(ColorTable[0]);


public:

    // --------------------------------------------------------
    // コンストラクタ
    LCDManager()
    {
        // 初期化
        mRot = LCD_ROTATE_NONE;
        mLCDSizeX = LCD_WIDTH;
        mLCDSizeY = LCD_HEIGHT;
    }


    // --------------------------------------------------------
    // 初期化
    void Initialize()
    {
        // GPIO初期化
        InitializeGPIO();

        // SPI初期化
        InitializeSPI();

        // LCD初期化
        InitializeLCD();

        // 回転設定
        SetRotation( 0 );

        // 消去
        Clear( LCD_COLOR_BLACK );

        // バックライトON
        gpio_set_level( LCD_PIN_BACKLIGHT, 1 );
    }


    // --------------------------------------------------------
    // 横サイズ取得
    //   戻り値 : 横サイズを返します。
    int16_t GetSizeX() const
    {
        // 取得
        return mLCDSizeX;
    }


    // --------------------------------------------------------
    // 縦サイズ取得
    //   戻り値 : 縦サイズを返します。
    int16_t GetSizeY() const
    {
        // 取得
        return mLCDSizeY;
    }

    
    // --------------------------------------------------------
    // サイズ取得
    //   SizeX          横サイズ格納先
    //   SizeY          縦サイズ格納先
    //   戻り値 : 縦サイズを返します。
    void GetSize( uint16_t& SizeX, uint16_t& SizeY ) const
    {
        // 取得
        SizeX = mLCDSizeX;
        SizeY = mLCDSizeY;
    }


    // --------------------------------------------------------
    // 回転設定
    //   Rot        回転方向(0～3)
    // 補足
    // ・回転方向は下記の何れか。
    //     0(LCD_ROTATE_NONE) : USBコネクタを下にした状態で左上が(0,0)。USBコネクタが下で縦長。回転なし。
    //     1(LCD_ROTATE_90)   : USBコネクタを下にした状態で右上が(0,0)。USBコネクタが右で横長。LCDを左に90度回転。
    //     2(LCD_ROTATE_180)  : USBコネクタを下にした状態で右下が(0,0)。USBコネクタが上で横長。LCDを180度回転。
    //     3(LCD_ROTATE_270)  : USBコネクタを下にした状態で左下が(0,0)。USBコネクタが左で横長。LCDを右に90度回転。
    void SetRotation( uint8_t Rot )
    {
        // 入力補正
        Rot %= 4;

        // 設定決定
        uint8_t madctl = LCD_MADCTL_DEFAULT; // メモリアクセス設定：初期値（色順RGB：回転なし)
        switch (Rot) {
            case LCD_ROTATE_NONE: // 0度
                mLCDSizeX = LCD_WIDTH;
                mLCDSizeY = LCD_HEIGHT;
                break;

            case LCD_ROTATE_90: // 90度
                mLCDSizeX = LCD_HEIGHT;
                mLCDSizeY = LCD_WIDTH;
                madctl |= LCD_MADCTL_MV | LCD_MADCTL_MX; // XY軸交換、X軸反転
                break;

            case LCD_ROTATE_180: // 180度
                mLCDSizeX = LCD_WIDTH;
                mLCDSizeY = LCD_HEIGHT;
                madctl |= LCD_MADCTL_MY | LCD_MADCTL_MX; // X軸反転、Y軸反転
                break;

            case LCD_ROTATE_270: // 270度
                mLCDSizeX = LCD_HEIGHT;
                mLCDSizeY = LCD_WIDTH;
                madctl |= LCD_MADCTL_MV |  LCD_MADCTL_MY; // XY軸交換、Y軸反転
                break;

            default: // 上記以外
                // 変更しない
                return;
        }

        // メモリアクセス設定の書き込み
        SPISendCommand(LCD_ILI9341_MADCTL);
        SPISendData(madctl);

        // 回転設定保存
        mRot = Rot;
    }


    // --------------------------------------------------------
    // 回転設定取得
    //   戻り値 : 現在の回転設定(0:回転なし、1:左90度回転、2:180度回転、右90度回転)を返します。
    uint8_t GetRotation() const
    {
        return mRot;
    }


    // --------------------------------------------------------
    // クリア
    //   Color          色
    void Clear( uint16_t Color )
    {
        FillRectangle( 0, 0, mLCDSizeX, mLCDSizeY, Color );
    }


    // --------------------------------------------------------
    // 塗り潰し矩形描画
    //   PosX, PosY     位置(左上座標)
    //   SizeX, SizeY   サイズ
    //   Color      色
    void FillRectangle( int16_t PosX, int16_t PosY, int16_t SizeX, int16_t SizeY, uint16_t Color )
    {
        // 範囲のチェックと補正
        if ( !GetDrawableArea(PosX, PosY, SizeX, SizeY) ) return; // サイズ不正の場合、何もしない

        // 対象領域設定
        SetWindow( PosX, PosY, PosX + SizeX - 1, PosY + SizeY - 1 );

        // 色転送
        SPISendCommand( LCD_ILI9341_RAMWR );
        SPISendColor( Color, static_cast<uint32_t>(SizeX) * static_cast<uint32_t>(SizeY) );
    }

    
    // --------------------------------------------------------
    // 水平線描画
    //   PosX, PosY     開始位置
    //   Size           サイズ
    //   Colors         色
    void DrawLineH( int16_t PosX, int16_t PosY, int16_t Size, uint16_t Color )
    {
        // 横長矩形を描画
        if ( Size < 0 ) {
            // サイズが負数の場合、サイズが正数になるように描画
            FillRectangle( PosX + Size + 1, PosY, -Size, 1, Color );
        } else {
            FillRectangle( PosX, PosY, Size, 1, Color );
        }
    }


    // --------------------------------------------------------
    // 垂直線描画
    //   PosX, PosY     開始位置
    //   Size           サイズ
    //   Colors         色
    void DrawLineV( int16_t PosX, int16_t PosY, int16_t Size, uint16_t Color )
    {
        // 縦長矩形を描画
        if ( Size < 0 ) {
            // サイズが負数の場合、サイズが正数になるように描画
            FillRectangle( PosX, PosY + Size + 1, 1, -Size, Color );
        } else {
            FillRectangle( PosX, PosY, 1, Size, Color );
        }
    }    


    // --------------------------------------------------------
    // 塗り潰し矩形描画
    //   Pos1X, Pos1Y   始点
    //   Pos2X, Pos2Y   終点
    //   Color      色
    void DrawLine( int16_t Pos1X, int16_t Pos1Y, int16_t Pos2X, int16_t Pos2Y, uint16_t Color )
    {
        // 描画範囲のチェックと補正
        if ( !CorrectToBoundary(Pos1X, Pos1Y, Pos2X, Pos2Y) ) return; // 描画不可の場合、何もしない

		// 描画準備
		int16_t px, py; // 描画位置(初期値：開始位置)
		int16_t ex, ey; // 終了位置
        px = Pos1X;
        py = Pos1Y;
        ex = Pos2X;
        ey = Pos2Y;
		int16_t fx = ex - px; // 横幅
		int16_t fy = ey - py; // 縦幅
		int16_t dx = (fx < 0) ? -fx : fx; // 横幅絶対値
		int16_t dy = (fy < 0) ? -fy : fy; // 縦幅絶対値
		int16_t sx = (fx < 0) ? -1 : 1; // 横移動量
		int16_t sy = (fy < 0) ? -1 : 1; // 縦移動量

		// 描画(ブレゼンハム方式)
		int16_t ee; // 誤差
#if 0        
        DrawPixel( px, py, Color ); // 始点描画
		if (dx >= dy) {
			// X軸基準で描画すべき場合

			// 終端まで描画
			ee = 2 * dy - dx;
			while (px != ex) {
				px += sx;
				if (ee > 0) {
					// 縦位置が変化するタイミングの場合
					py += sy;
					ee -= 2 * dx;
				}
				ee += 2 * dy;
                DrawPixel( px, py, Color ); // 点描画
			}

		} else {
			// Y軸基準で描画すべき場合

			// 終端まで描画
			ee = 2 * dx - dy;
			while (py != ey) {
				py += sy;
				if (ee > 0) {
					// 横位置が変化するタイミングの場合
					px += sx;
					ee -= 2 * dy;
				}
				ee += 2 * dx;
                DrawPixel( px, py, Color ); // 点描画
			}
		}
#else 
        // ちょこっと高速版

        int16_t sp;
		int16_t ss; // 連続数
        if (dx >= dy) {
			// X軸基準で描画すべき場合

            // 終端まで描画
			ee = 2 * dy - dx;
            sp = px;
            ss = sx;
			while (px != ex) {
				px += sx;
				if (ee > 0) {
					// 縦位置が変化するタイミングの場合
                    DrawLineH( sp, py, ss, Color ); // 描画
					py += sy;
					ee -= 2 * dx;
                    ss = sx;
                    sp = px;
				} else {
                    ss += sx;
                }
				ee += 2 * dy;
            }
            DrawLineH( sp, py, ss, Color ); // 描画

		} else {
			// Y軸基準で描画すべき場合

			// 終端まで描画
			ee = 2 * dx - dy;
            sp = py;
            ss = sy;
			while (py != ey) {
				py += sy;
				if (ee > 0) {
					// 横位置が変化するタイミングの場合
                    DrawLineV( px, sp, ss, Color ); // 描画
					px += sx;
					ee -= 2 * dy;
                    ss = sy;
                    sp = py;
				} else {
                    ss += sy;
                }
				ee += 2 * dx;
			}
            DrawLineV( px, sp, ss, Color );
		}
#endif
    }


    // --------------------------------------------------------
    // 点描画
    //   PosX, PosY     位置(左上座標)
    //   Color      色
    void DrawPixel( int16_t PosX, int PosY, uint16_t Color ) 
    {
        // サイズ1x1の矩形として描画
        FillRectangle( PosX, PosY, 1, 1, Color );
    }


    // --------------------------------------------------------
    // 矩形枠描画
    //   PosX, PosY     位置(左上座標)
    //   SizeX, SizeY   サイズ
    //   Color      色
    void DrawRectangle( int16_t PosX, int16_t PosY, int16_t SizeX, int16_t SizeY, uint16_t Color )
    {
        // 引数チェック
        if ( !GetDrawableArea(PosX, PosY, SizeX, SizeY) ) return; // サイズ不正の場合、何もしない

        // 四辺を描画
        FillRectangle( PosX,             PosY,             SizeX,    1,         Color ); // 上辺
        FillRectangle( PosX + SizeX - 1, PosY + 1,         1,        SizeY - 2, Color ); // 右辺
        FillRectangle( PosX,             PosY + SizeY - 1, SizeX,    1,         Color ); // 下辺
        FillRectangle( PosX,             PosY,             1,        SizeY - 2, Color ); // 左辺

    }


    // --------------------------------------------------------
    // 画像描画
    //   PosX, PosY     位置(左上座標)
    //   SizeX, SizeY   サイズ
    //   Colors         画像データ(ピクセル色情報)の格納先
    // 補足
    // ・画像データは 16bit/pixel で、メモリ上の配置が RGB565 であること。
    //   (uint16_tに格納する場合、上下バイトを反転する必要がある)
    void DrawImage( int16_t PosX, int16_t PosY, int16_t SizeX, int16_t SizeY, const uint16_t* Colors )
    {
        // 範囲のチェックと補正
        int16_t px = PosX;      // 指定された位置
        int16_t py = PosY;
        int16_t sx = SizeX;     // 指定されたサイズ
        if ( !GetDrawableArea(PosX, PosY, SizeX, SizeY) ) return; // 描画できない場合(描画できる範囲がない場合)、中止
        int16_t tx = PosX - px; // 描画しないサイズ
        int16_t ty = PosY - py; 

        // 対象領域設定
        SetWindow( PosX, PosY, PosX + SizeX - 1, PosY + SizeY - 1 );

        // 色情報転送
        Colors += ty * sx;
        SPISendCommand( LCD_ILI9341_RAMWR );
        for ( int16_t idx = 0; idx < SizeY; idx++ ) {
            SPISend( false, reinterpret_cast<const uint8_t*>(Colors + tx), SizeX * 2 );
            Colors += sx;
        }
    }


    // --------------------------------------------------------
    // 座標変換
    //   TouchX,TouchY  物理座標(回転を考慮していない座標)
    //   DrawX,DrawY    論理座標(現在の回転設定に合わせた座標)
    // 補足
    // ・物理座標(回転なし状態での座標)を論理座標(現在の回転設定に合わせて回転させた座標)に更新します。
    // ・主にタッチパネルでの得られた座標を、描画するための座標に変換する為の機能です。
    void GetScreenPos( int16_t TouchX, int16_t TouchY, int16_t& DrawX, int16_t& DrawY ) const
    {
        switch( mRot ) {
            case LCD_ROTATE_NONE: // 回転なし
                DrawX = TouchX;
                DrawY = TouchY;
                break;

            case LCD_ROTATE_90: // 90度
                DrawX = TouchY;
                DrawY = mLCDSizeY - TouchX;
                break;

            case LCD_ROTATE_180: // 180度
                DrawX = mLCDSizeX - TouchX;
                DrawY = mLCDSizeY - TouchY;
                break;

            case LCD_ROTATE_270: // 270度
                DrawX = mLCDSizeX - TouchY;
                DrawY = TouchX;
                break;

            default: // 上記以外
                // 何もしない
                break;
        }
    }


    // --------------------------------------------------------
    // カラーテーブルの色取得
    //   Index  インデックス
    uint16_t GetColor( uint16_t Index ) const
    {
        return ColorTable[ Index % ColorTableCount ];
    }


private:


    // --------------------------------------------------------
    // GPIO初期化
    void InitializeGPIO()
    {
        // DCピン初期化
        gpio_reset_pin( LCD_PIN_DC );
        gpio_set_direction( LCD_PIN_DC, GPIO_MODE_OUTPUT );
        gpio_set_level( LCD_PIN_DC, 0 );

        // バックライト
        gpio_reset_pin( LCD_PIN_BACKLIGHT );
        gpio_set_direction( LCD_PIN_BACKLIGHT, GPIO_MODE_OUTPUT );
        gpio_set_level( LCD_PIN_BACKLIGHT, 0 );
    }


    // --------------------------------------------------------
    // SPI初期化
    void InitializeSPI()
    {
        // バス初期化
        mSPI = nullptr;
        spi_bus_config_t buscfg = {
            .mosi_io_num = LCD_PIN_MOSI,
            .miso_io_num = -1,
            .sclk_io_num = LCD_PIN_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
#if 1 // platformio.init内の platform が 6.12.0 の場合            
            .data_io_default_level = false,
            .max_transfer_sz = 0,
            .flags = 0,
            .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#else // platformio.init内の platform が 6.5.0 の場合            
            .max_transfer_sz = 0,
            .flags = 0,
            .isr_cpu_id = INTR_CPU_ID_AUTO,
#endif
            .intr_flags = 0
        };
        esp_err_t ret = spi_bus_initialize( LCD_SPI_PORT, &buscfg, SPI_DMA_CH_AUTO );
        if ( ret != ESP_OK ) {
            // 初期化できない場合
            LCD_ERRORPRINT( "spi_bus_initialize() failed.  rc=0x%x\n", ret );
            return; // 失敗
        }

        // バス割り当て
        spi_device_interface_config_t devcfg;
        memset(&devcfg, 0, sizeof(devcfg));
        devcfg.clock_speed_hz = LCD_SPI_FREQUENCY;
        devcfg.queue_size = 7;
        devcfg.mode = 3;
        devcfg.flags = SPI_DEVICE_NO_DUMMY;
        devcfg.spics_io_num = LCD_PIN_CS;
        ret = spi_bus_add_device( LCD_SPI_PORT, &devcfg, &mSPI );
        if ( ret != ESP_OK ) {
            // 追加できない場合
            LCD_ERRORPRINT( "spi_bus_add_device() failed  rc=0x%x\n", ret );
            return; // 失敗
        }
    }


    // --------------------------------------------------------
    // LCD初期化
    void InitializeLCD()
    {
        vTaskDelay( pdMS_TO_TICKS(10) );

        // ソフトウェアリセット
        SPISendCommand( LCD_ILI9341_SWRESET );
        vTaskDelay( pdMS_TO_TICKS(10) );

        // スリープ解除
        SPISendCommand( LCD_ILI9341_SLPOUT );
        vTaskDelay( pdMS_TO_TICKS(120) );

        // ピクセルフォーマット設定
        SPISendCommand(LCD_ILI9341_PIXFMT );
        SPISendData( static_cast<uint8_t>(0x55) ); // RGB:16bit/pixel, Control:16bit/pixel

        // メモリアクセス方法設定
        SPISendCommand( LCD_ILI9341_MADCTL );
        SPISendData( (uint8_t)0x00 );   // 

        // 画面表示開始
        SPISendCommand( LCD_ILI9341_DISPON );
        vTaskDelay( pdMS_TO_TICKS(150) );
    }


    // --------------------------------------------------------
    // SPI送信
    //   IsCommand          true時、コマンド送信(false時、データ送信)
    //   Data               送信データ格納領域
    //   DataLength         送信データ数
    void SPISend(bool IsCommand, const uint8_t* Data, size_t DataLength)
    {
        spi_transaction_t SPITransaction;
        esp_err_t ret;

        if ( DataLength > 0 ) {
            // データがある場合

            // 対象選択
            gpio_set_level( LCD_PIN_DC,  IsCommand ? 0:1 );

            // トランザクション情報作成
            memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
            SPITransaction.length = DataLength * 8;
            SPITransaction.tx_buffer = Data;

            // 転送開始
            ret = spi_device_transmit( mSPI, &SPITransaction );
            if ( ret != ESP_OK ) {
                LCD_ERRORPRINT( "spi_device_transmit() failed  rc=0x%x, Data=%p, Len=%d\n", ret, Data, DataLength );
            }
        }
    }


    // --------------------------------------------------------
    // SPIコマンド送信
    //   Cmd                コマンド
    void SPISendCommand( uint8_t Cmd )
    {
        // コマンド送信
        SPISend( true, &Cmd, 1 );
    }


    // --------------------------------------------------------
    // SPIデータ送信(byte)
    //   Data               データ
    void SPISendData( uint8_t Data )
    {
        // 出w－た送信
        SPISend( false, &Data, 1 );
    }


    // --------------------------------------------------------
    // SPIデータ送信(word)
    //   Data               データ
    void SPISendData( uint16_t Data )
    {
        uint8_t wk[2];
        wk[0] = (Data >> 8) & 0xff;
        wk[1] = (Data     ) & 0xff;
        SPISend( false, &wk[0], sizeof(wk) );
    }


    // --------------------------------------------------------
    // SPIデータ送信(word×2)
    //   Data1              データ１
    //   Data2              データ２
    void SPISendData( uint16_t Data1, uint16_t Data2 )
    {
        uint8_t wk[4];
        wk[0] = (Data1 >> 8) & 0xff;
        wk[1] = (Data1     ) & 0xff;
        wk[2] = (Data2 >> 8) & 0xff;
        wk[3] = (Data2     ) & 0xff;
        SPISend( false, &wk[0], sizeof(wk) );
    }

    
    // --------------------------------------------------------
    // SPI色データ送信
    //   Color              色
    //   Count              送信数
    void SPISendColor( uint16_t Color, uint32_t Count )
    {
        uint16_t wk[LCD_SPI_MAX_TRANSWORD]; // １回分の転送データ   
        uint32_t tr; // １回分の転送データ数

        // 転送データ作成
        tr = (Count > LCD_SPI_MAX_TRANSWORD) ? LCD_SPI_MAX_TRANSWORD:Count;
        for(int idx = 0; idx < tr; idx++ ) {
            wk[idx] = Color;
        }

        // 転送
        while ( Count > 0) {
            tr = (Count > LCD_SPI_MAX_TRANSWORD) ? LCD_SPI_MAX_TRANSWORD:Count;
            SPISend( false, reinterpret_cast<uint8_t*>(&wk[0]), tr * 2 );
            Count -= tr;
        }
    }


    // --------------------------------------------------------
    // 対象領域設定
    //   PosX, PosY         開始位置
    //   EndX, EndY         終了位置
    void SetWindow( int16_t PosX, int16_t PosY, int16_t EndX, int16_t EndY )
    {
        // 設定
        SPISendCommand( LCD_ILI9341_CASET );
        SPISendData( PosX, EndX );
        SPISendCommand( LCD_ILI9341_PASET );
        SPISendData( PosY, EndY );
    }


    // --------------------------------------------------------
    // 描画可能範囲取得
    //   PosX, PosY     位置(左上座標)
    //   SizeX, SizeY   サイズ
    //   戻り値 : 描画可能な場合、trueを返します。
    bool GetDrawableArea( int16_t& PosX, int16_t& PosY, int16_t& SizeX, int16_t& SizeY )
    {
        // X位置の補正
        if ( mLCDSizeX <= PosX ) return false; // X位置が右端の外の場合、描画不可
        if ( PosX < 0 ) {
            // X位置が右端の外の場合、右端位置に補正
            SizeX += PosX;
            PosX = 0;
        }

        // Y位置の補正
        if ( mLCDSizeY <= PosY ) return false; // Y位置が下端の外の場合、描画不可
        if ( PosY < 0 ) {
            // Y位置が上端の外の場合、状態位置に補正
            SizeY += PosY;
            PosY = 0;
        }

        // Xサイズの補正
        if ( mLCDSizeX < (PosX + SizeX - 1) ) {
            // 描画範囲右端が画面右端の外の場合、サイズ補正
            SizeX = mLCDSizeX - PosX;
        }

        // Yサイズの補正
        if ( mLCDSizeY < (PosY + SizeY - 1) ) {
            // 描画範囲右端が画面右端の外の場合、サイズ補正
            SizeY = mLCDSizeY - PosY;
        }

        // サイズのチェック
        if( (SizeX <= 0) || (SizeY <= 0) ) return false; // 描画範囲がない場合、描画不可

        // 描画可能
        return true;
    }


	// -------------------------------------------------------
	// 直線描画時の境界補正
	//   Pos1X, Pos1Y		始点位置(描画可能な位置に変更される)
	//   Pos2X, Pos2Y		終点位置(描画可能な位置に変更される)
	//   戻り値 : 描画可能な場合、trueを返します。
	bool CorrectToBoundary(int16_t& Pos1X, int16_t& Pos1Y, int16_t& Pos2X, int16_t& Pos2Y) const
	{
		// 横方向の境界補正
		int16_t w;
		if (Pos1X > Pos2X) {
			// 始点と終点の位置関係が補正に合わない場合、入れ替え
			w = Pos1X; Pos1X = Pos2X; Pos2X = w;
			w = Pos1Y; Pos1Y = Pos2Y; Pos2Y = w;
		}
		CorrectToBoundary(Pos1X, Pos1Y, Pos2X, Pos2Y, 0, Pos1X, Pos1Y);
		CorrectToBoundary(Pos1X, Pos1Y, Pos2X, Pos2Y, mLCDSizeX-1, Pos2X, Pos2Y);
		if ((Pos2X < 0) || (mLCDSizeX <= Pos1X)) return false; // 範囲外の場合、描画しない

		// 縦方向の境界補正
		if (Pos1Y > Pos2Y) {
			// 始点と終点の位置関係が補正に合わない場合、入れ替え
			w = Pos1X; Pos1X = Pos2X; Pos2X = w;
			w = Pos1Y; Pos1Y = Pos2Y; Pos2Y = w;
		}
		CorrectToBoundary(Pos1Y, Pos1X, Pos2Y, Pos2X, 0, Pos1Y, Pos1X);
		CorrectToBoundary(Pos1Y, Pos1X, Pos2Y, Pos2X, mLCDSizeY-1, Pos2Y, Pos2X);
		if ((Pos2Y < 0) || (mLCDSizeY <= Pos1Y)) return false; // 範囲外の場合、描画しない

		// 描画可能
		return true;
	}


	// -------------------------------------------------------
	// 直線描画時の境界補正
	//   Pos1A, Pos1B		始点位置
	//   Pos2A, Pos2B		終点位置
	//   Border			境界位置(Pos1A/Pos2Aと同じ軸の値)
	//   BorderA, BorderB	補正値の格納先
	static void CorrectToBoundary(int16_t Pos1A, int16_t Pos1B, int16_t Pos2A, int16_t Pos2B, uint16_t Border, int16_t& BorderA, int16_t& BorderB) 
	{
		if ((Pos1A <= Border) && (Border <= Pos2A) && (Pos2A != Pos1A)) {
			// 境界線を跨ぐ場合

			// 境界線上の位置に補正
			BorderA = Border;
			BorderB = Pos1B + (Pos2B - Pos1B) * (Border - Pos1A) / (Pos2A - Pos1A);
		}
	}


public:

    // --------------------------------------------------------
    // 数値に対応した色の取得
    //   Index      番号
    static uint16_t GetColor( int Index )
    {
        uint16_t col = LCD_COLOR_BLACK;

        if ( Index >= 0 ) {
            Index = Index % ColorTableCount;
            col = ColorTable[Index];
        }

        return col;
    }


    // --------------------------------------------------------
    // 色作成
    //   Red        赤要素(0～255)
    //   Green      緑要素(0～255)
    //   Blue       青要素(0～255)
    //   戻り値 : 描画用の色値を返します。
    // 補足
    // ・マクロLCD_COLORと同様の機能ですが、
    //   LCD_COLORはRedとBlueが5bitでGreenが6bitなのに対して、
    //   この関数ではRed/Blue/Greenとも8bitで指定します。
    static uint16_t ToColor( uint8_t Red, uint8_t Green, uint8_t Blue )
    {
        uint16_t rr = (Red >> 3);
        uint16_t gg = (Green >> 2);
        uint16_t bb = (Blue >> 3);
        uint16_t rgb =  (bb << 11) | (gg << 5) | rr;
        return (rgb >> 8) | (rgb << 8);
    }    


    // --------------------------------------------------------
    // 色分解
    //   Color      色
    //   Red        赤要素格納先(0～255)
    //   Green      緑要素格納先(0～255)
    //   Blue       青要素格納先(0～255)
    // 補足
    // ・ToColor関数の逆の動作となります。
    //   ただし、ToColor関数内で各要素が量子化(ビット数削減)されるため、
    //   ToColor関数の戻り値を引数にこの関数を呼び出しても、元の値とはなりません。
    // ・マクロLCD_COLORと逆の様な機能ですが、各要素の範囲が8bitとなります。
    static void ToRGB( uint16_t Color, uint8_t& Red, uint8_t& Green, uint8_t& Blue )
    {
        uint16_t rgb = (Color >> 8) | (Color << 8);
        Blue = (rgb >> 8) & 0xF8;
        Green = (rgb >> 3) & 0xFC;
        Red = (rgb << 3);
        if ( Red == 0xF8 ) Red = 0xFF;
        if ( Green == 0xFC ) Green = 0xFF;
        if ( Blue == 0xF8 ) Blue = 0xFF;
    }


    // --------------------------------------------------------
    // 色グラデーションテーブル作成
    //   Color1         開始色
    //   Color2         終了色
    //   Table          色格納先
    //   TableCount     色格納先個数(グラデーションの段階数)
    // 補足
    // ・TableはTableCount個だけ格納できる領域が必要です。
    static void MakeColorTable( uint16_t Color1, uint16_t Color2, uint16_t* Table, int16_t TableCount )
    {
        // 引数チェック
        if ( Table == nullptr ) return; // 格納先が指定されていない場合、何もしない
        if ( TableCount <= 1 ) return; // 格納可能数が足りない場合、何もしない

        // 作成準備
        uint8_t rr1;    // 開始色
        uint8_t gg1;
        uint8_t bb1;
        uint8_t rr2;    // 終了色
        uint8_t gg2;
        uint8_t bb2;
        ToRGB( Color1, rr1, gg1, bb1 );
        ToRGB( Color2, rr2, gg2, bb2 );
        int16_t rrd = static_cast<int16_t>(rr2) - static_cast<int16_t>(rr1);    // 差分
        int16_t ggd = static_cast<int16_t>(gg2) - static_cast<int16_t>(gg1);
        int16_t bbd = static_cast<int16_t>(bb2) - static_cast<int16_t>(bb1);

        // テーブル作成
        uint8_t r;
        uint8_t g;
        uint8_t b;
        for ( int16_t idx = 0; idx < TableCount; idx++ ) {
            r = (uint8_t)( static_cast<int16_t>(rr1) + rrd * idx / (TableCount - 1) );
            g = (uint8_t)( static_cast<int16_t>(gg1) + ggd * idx / (TableCount - 1) );
            b = (uint8_t)( static_cast<int16_t>(bb1) + bbd * idx / (TableCount - 1) );
            Table[idx] = ToColor( r, g, b );
        }
    }


};

