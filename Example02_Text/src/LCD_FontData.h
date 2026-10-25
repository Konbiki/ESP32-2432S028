///////////////////////////////////////////////////////////////
// 文字描画用　フォントデータ定義
///////////////////////////////////////////////////////////////
#pragma once


///////////////////////////////////////////////////////////////

#include <stdint.h>

///////////////////////////////////////////////////////////////

#define FONT_MAXSIZEX   (512)           // 最大文字横サイズ
#define FONT_MAXSIZEY   (512)           // 最大文字縦サイズ

#define FONT_HALF       (0x80000000)    // 文字が半角であることを表すフラグ
#define FONT_CODEMASK   (0x00ffffff)    // コードテーブルのコード範囲マスク値


// ------------------------------------------------------------


// ピクセル形式
typedef enum
{
    // 1 bit/pixel [BYTE] mono
    FONT_PIXELFORMAT_BIT1 = 0x0001,         

    // 2 bit/pixel [BYTE] grayscale 4
    FONT_PIXELFORMAT_BIT2 = 0x0002,         

    // 4 bit/pixel [BYTE] grayscale 16
    FONT_PIXELFORMAT_BIT4 = 0x0004,         

    // 8 bit/pixel [BYTE] grayscale 256
    FONT_PIXELFORMAT_BIT8 = 0x0008,         

    // 16bit/pixel [WORD] RGB 5-6-5 LE
    FONT_PIXELFORMAT_RGB565 = 0x0565,

    // 16bit/pixel [WORD] RGB 5-6-5 BE
    FONT_PIXELFORMAT_RGB565_BE = 0xB565,

    // 16bit/pixel [WORD] BGR 5-6-5 LE
    FONT_PIXELFORMAT_BGR565 = 0x0ABC,

    // 16bit/pixel [WORD] BGR 5-6-5 BE
    FONT_PIXELFORMAT_BGR565_BE = 0xBABC,

} FONT_PIXELFORMAT;


// ------------------------------------------------------------


// 描画位置移動方向
typedef enum
{
    FONT_DIRECTION_UP,           // 上
    FONT_DIRECTION_RIGHT,        // 右
    FONT_DIRECTION_DOWN,         // 下
    FONT_DIRECTION_LEFT,         // 左
    FONT_DIRECTION_CR,           // 行先頭
} FONT_DIRECTION;


// ------------------------------------------------------------


// テキストVRAM描画フラグ
typedef enum
{
    FONT_VRAMFLAG_SPCAE         = 0x0000,       // 空白
    FONT_VRAMFLAG_HALF          = 0x0001,       // 半角
    FONT_VRAMFLAG_WIDE_L        = 0x0002,       // 全角左側
    FONT_VRAMFLAG_WIDE_R        = 0x0004,       // 全角右側
    FONT_VRAMFLAG_MASKDRAW      = 0x000F,       // 描画形式用マスク
    FONT_VRAMFLAG_UPDATE        = 0x8000,       // 更新あり

    FONT_VRAMFLAG_UPDATESPCAE   = FONT_VRAMFLAG_UPDATE | FONT_VRAMFLAG_SPCAE,     // 空白更新
    FONT_VRAMFLAG_UPDATEHALF    = FONT_VRAMFLAG_UPDATE | FONT_VRAMFLAG_HALF,      // 半角更新
    FONT_VRAMFLAG_UPDATEWIDE_L  = FONT_VRAMFLAG_UPDATE | FONT_VRAMFLAG_WIDE_L,    // 全角左側更新
    FONT_VRAMFLAG_UPDATEWIDE_R  = FONT_VRAMFLAG_UPDATE | FONT_VRAMFLAG_WIDE_R,    // 全角右側更新

} FONTVRAMFLAG;


// ------------------------------------------------------------


// フォントデータ構造体
typedef struct tagFONTDATA
{
    uint32_t            Count;          // 文字数
    uint16_t            SizeX;          // 横サイズ(半角サイズ)
    uint16_t            SizeY;          // 縦サイズ
    uint16_t            Format;         // ピクセル形式(FONT_PIXELFORMAT)
    uint16_t            Option;         // オプションパラメタ
    const uint32_t*     CodeTable;      // コードテーブル領域
    const void*         PatternTable;   // パターンテーブル領域
} FONTDATA;
