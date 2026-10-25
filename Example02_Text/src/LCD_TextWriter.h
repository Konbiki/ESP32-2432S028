///////////////////////////////////////////////////////////////
// 文字描画モジュール
///////////////////////////////////////////////////////////////
#pragma once


///////////////////////////////////////////////////////////////

#include "LCD_Manager.h"
#include "LCD_FontData.h"

///////////////////////////////////////////////////////////////

// ============================================================
// 文字描画クラス
class TextWriter
{
protected:
    LCDManager&     mLCD;       // LCD制御
    const FONTDATA* mFont;      // フォントデータ
    uint16_t*       mWriteBuff; // 作業用領域(パターン展開領域)


public:

    // --------------------------------------------------------
    // コンストラクタ
    //   Lcd        LCD制御
    //   Font       フォントデータ
    TextWriter( LCDManager& Lcd, const FONTDATA* Font ) : mLCD( Lcd )
    {
        // 初期化
        mFont = nullptr;
        mWriteBuff = nullptr;

        // 引数チェック
        if ( Font == nullptr ) return; // フォントが指定されていない場合、初期化中断(描画不可)
        if ( (FONT_MAXSIZEX < Font->SizeX) || 
             (FONT_MAXSIZEY < Font->SizeY) ) return; // フォントサイズが不正な場合、異常
        if ( Font->CodeTable == nullptr ) return; // コードテーブルがない場合、異常
        if ( Font->PatternTable == nullptr ) return; // パターンテーブルがない場合、異常
        mFont = Font;
        mWriteBuff = new uint16_t[Font->SizeX * 2 * Font->SizeY];
    }


    // --------------------------------------------------------
    // デストラクタ
    ~TextWriter()
    {
        if ( mWriteBuff != nullptr ) {
            // 作業領域が確保されている場合

            // 解放
            delete[] mWriteBuff;
            mWriteBuff = nullptr;
        }
    }


    // --------------------------------------------------------
    // LCD制御取得
    LCDManager& GetLCDManager()
    {
        return mLCD;
    }


    // --------------------------------------------------------
    // 描画可否状態取得
    //   戻り値 : 描画可能な場合、trueを返す。
    bool CanDraw()
    {
        // 判定
        return mFont != nullptr; // フォントデータが設定されている場合、描画可
    }


    // --------------------------------------------------------
    // 文字描画
    //   PosX, PosY         位置
    //   Code               文字コード
    //   ForeColor          前景色
    //   BackColor          背景色
    //   戻り値 : 半角文字を描画した場合1、全角文字を描画した場合2、それ以外の場合0を返します。
    uint16_t Draw( uint16_t PosX, uint16_t PosY, uint32_t Code, uint16_t ForeColor, uint16_t BackColor )
    {
        // 状態チェック
        if ( !CanDraw() ) return 0; // 描画不可能な場合、何もしない

        // パターンの準備
        uint16_t sx; // 横幅
        uint16_t sy; // 縦幅
        const uint16_t* img = MakeImage( Code, ForeColor, BackColor, sx, sy );
        if( img == nullptr  ) return 0; // 失敗した場合、描画なし

        // 描画
        mLCD.DrawImage( PosX, PosY, sx, sy, img );

        return (sx == mFont->SizeX) ? 1:2;
    }


    // --------------------------------------------------------
    // 文字描画
    //   PosX, PosY         位置
    //   Text               文字列(UTF-8、'\0'終端)
    //   ForeColor          前景色
    //   BackColor          背景色
    void Draw( uint16_t PosX, uint16_t PosY, const char* Text, uint16_t ForeColor, uint16_t BackColor )
    {
        // 引数チェック
        if ( Text == nullptr ) return; // 文字列が指定されていない場合、何もしない

        // 状態チェック
        if ( !CanDraw() ) return; // 描画不可能な場合、何もしない

        // 描画
        for(;;) {

            // １文字取り出し
            uint32_t ucode = GetUnicode( Text );
            if ( ucode == 0 ) break; // 終端

            // 一文字描画
            uint16_t fw = Draw( PosX, PosY, ucode, ForeColor, BackColor );
            PosX += mFont->SizeX * fw;
        }
        
    }


    // --------------------------------------------------------
    // 半分描画
    //   PosX, PosY         位置
    //   Code               文字コード
    //   ForeColor          前景色
    //   BackColor          背景色
    //   Side               0:左側、1:右側
    //   戻り値 : 描画された場合、trueを返す。
    bool Draw( uint16_t PosX, uint16_t PosY, uint32_t Code, uint16_t ForeColor, uint16_t BackColor, uint16_t Side )
    {
        // 状態チェック
        if ( !CanDraw() ) return 0; // 描画不可能な場合、何もしない

        // パターンの準備
        uint16_t sx; // 横幅
        uint16_t sy; // 縦幅
        const uint16_t* img = MakeImage( Code, ForeColor, BackColor, sx, sy );
        if( img == nullptr ) return false; // 失敗した場合、描画なし
        if ( sx == mFont->SizeX ) {
            // 半角文字の場合

            // そのまま描画
            mLCD.DrawImage( PosX, PosY, sx, sy, img );
            return true;
        }

        // パターンの間引き
        uint16_t ww = sx / 2; // 描画する横幅
        uint16_t si = Side * ww; 
        uint16_t di = 0;
        for ( uint16_t y = 0; y < sy; y++ ) {
            for ( uint16_t x = 0; x < ww; x++ ) {
                mWriteBuff[di++] = img[si++];
            }
            si += ww;
        }

        // 描画
        mLCD.DrawImage( PosX, PosY, ww, sy, mWriteBuff );

        // 成功
        return true;
    }


    // --------------------------------------------------------
    // フォントデータ取得
    //   戻り値 : フォントデータへのポインタを返す。
    const FONTDATA* GetFontData()
    {
        return mFont;
    }

    
private:


    // --------------------------------------------------------
    // パターン展開
    //   Code               文字コード
    //   ForeColor          前景色
    //   BackColor          背景色
    //   SizeX              横幅格納先
    //   SizeY              縦幅格納先
    //   戻り値 : 成功した場合、パターンが格納されている領域ポインタを返す。
    const uint16_t* MakeImage( uint32_t Code, uint16_t ForeColor, uint16_t BackColor, uint16_t& SizeX, uint16_t& SizeY )
    {
        const uint16_t* buff = mWriteBuff; // パターン格納先：初期値（パターン展開領域）

        // パターンの準備
        int32_t idx = FindCodeIndex( Code );
        if ( idx < 0 ) {
            // 存在しない文字の場合

            // ダミーイメージ作成
            WriteDummayImage( Code, ForeColor, BackColor, SizeX, SizeY );

        } else {
            // 存在する文字の場合
            uint16_t coltbl[256]; // 色テーブル
            switch( mFont->Format ) {
                case FONT_PIXELFORMAT_BIT1:
                    coltbl[0] = BackColor;
                    coltbl[1] = ForeColor;
                    WriteBitPattern( idx, 1, &coltbl[0], SizeX, SizeY );
                    break;

                case FONT_PIXELFORMAT_BIT2:
                    LCDManager::MakeColorTable( BackColor, ForeColor, &coltbl[0], 4 );
                    WriteBitPattern( idx, 2, &coltbl[0], SizeX, SizeY );
                    break;

                case FONT_PIXELFORMAT_BIT4:
                    LCDManager::MakeColorTable( BackColor, ForeColor, &coltbl[0], 16 );
                    WriteBitPattern( idx, 4, &coltbl[0], SizeX, SizeY );
                    break;

                case FONT_PIXELFORMAT_BIT8:
                    LCDManager::MakeColorTable( BackColor, ForeColor, &coltbl[0], 256 );
                    WriteBitPattern( idx, 8, &coltbl[0], SizeX, SizeY );
                    break;

                case FONT_PIXELFORMAT_RGB565:
                case FONT_PIXELFORMAT_BGR565:
                    WriteRGB565LEPattern( idx, SizeX, SizeY );
                    break;

                case FONT_PIXELFORMAT_RGB565_BE:
                case FONT_PIXELFORMAT_BGR565_BE:
                    {
                        const uint16_t** pptbl = (const uint16_t**)(mFont->PatternTable); // パターンテーブル
                        buff = pptbl[idx]; // パターンデータ格納先から直接描画
                        GetCharSize( idx, SizeX, SizeY );
                    }
                    break;

                default: // 上記以外
                    // ダミーイメージ作成
                    WriteDummayImage( Code, ForeColor, BackColor, SizeX, SizeY );
                    break;
            }
        }        

        // 成功
        return buff;
    }


    // --------------------------------------------------------
    // 不明文字パターンを作業領域に描画
    //   Code               コード
    //   ForeColor          前景色
    //   BackColor          背景色
    //   SizeX              横幅格納先
    //   SizeY              縦幅格納先
    void WriteDummayImage( uint32_t Code, uint16_t ForeColor, uint16_t BackColor, uint16_t& SizeX, uint16_t& SizeY )
    {
        // 状態チェック
        if ( !CanDraw() ) return; // 描画不可能な場合、何もしない

        // サイズ決定
        SizeX = mFont->SizeX;
        SizeY = mFont->SizeY;
        if ( !IsHalf( Code ) ) SizeX *= 2; // 全角の場合、横幅２倍

        // パターン作成
        uint16_t* buff = mWriteBuff; // パターン展開先
        for ( int y = 0; y < SizeY; y++ ) {
            for ( int x = 0; x < SizeX; x++ ) {
                uint16_t col = BackColor;
                if ( (0 <= x) && (0 <= y) && (x <= SizeX - 2) && (y <= SizeY - 2) ) {
                    // 表示範囲内の場合

                    if ( (x == 0) || (x == SizeX - 2) || (y == 0) || (y == SizeY - 2) ) {
                        // 外枠部分の場合
                        col = ForeColor; // 前景色

                    } else {
                        // 内側部分の場合
                        col = (((x + y) & 1) == 0) ? ForeColor:BackColor; // 市松模様
                    }
                }
                *(buff++) = col;
            }
        }

    }


    // --------------------------------------------------------
    // Xbit/pixel形式(FONT_PIXELFORMAT_BITX)のパターンを作業領域に描画
    //   CodeIndex          コードインデックス
    //   ColorBit           色ビット数
    //   ColorTable         色テーブル
    //   SizeX              横幅格納先
    //   SizeY              縦幅格納先
    void WriteBitPattern( uint32_t CodeIndex, uint8_t ColorBit, const uint16_t* ColorTable, uint16_t& SizeX, uint16_t& SizeY )
    {
        // 引数チェック
        if ( (ColorBit != 1) && (ColorBit != 2) && (ColorBit != 4) && (ColorBit != 8) ) return; // 色ビット数が不正な場合、何もしない
        if ( ColorTable == nullptr ) return; // 色テーブルが不正な場合、何もしない

        // 状態チェック
        if ( !CanDraw() ) return; // 描画不可能な場合、何もしない

        // パターン展開
        const uint8_t** pptbl = (const uint8_t**)(mFont->PatternTable); // パターンテーブル
        const uint8_t* pp = pptbl[CodeIndex]; // パターンデータ格納先
        uint8_t sft = 8 - ColorBit; // パターンデータから色を取り出す時のシフト数
        uint8_t msk = (1 << ColorBit) - 1; // パターンデータから色を取り出す時のマスク
        if ( pp != nullptr ) {
            // パターンデータがある場合

            // パターン展開
            uint16_t* buff = mWriteBuff; // パターン展開先
            uint8_t dd; // パターンデータ
            int ddcnt = 0; // パターンデータ中の残りビット数
            uint8_t colidx; // 色インデックス
            GetCharSize( CodeIndex, SizeX, SizeY );
            for ( int y = 0; y < SizeY; y++ ) {
                for ( int x = 0; x < SizeX; x++ ) {

                    // パターンデータの確認
                    if ( ddcnt == 0 ) {
                        // パターンデータ(ビット)がなくなった場合
                        dd = *(pp++); // 次のデータを取得
                        ddcnt = 8;
                    }

                    // １ピクセル分を用意
                    colidx = (dd >> sft) & msk;
                    *(buff++) = ColorTable[colidx];
                    dd <<= ColorBit;
                    ddcnt -= ColorBit;
                }
            }
        }
    }


    // --------------------------------------------------------
    // RGB565(LE)形式(FONT_PIXELFORMAT_RGB565)のパターンを展開
    //   CodeIndex          コードインデックス
    //   SizeX              横幅格納先
    //   SizeY              縦幅格納先
    void WriteRGB565LEPattern( uint32_t CodeIndex, uint16_t& SizeX, uint16_t& SizeY )
    {
        // 状態チェック
        if ( !CanDraw() ) return; // 描画不可能な場合、何もしない

        // サイズ取得
        GetCharSize( CodeIndex, SizeX, SizeY );

        // H/Lを入れ替えて格納
        const uint16_t** pptbl = reinterpret_cast<const uint16_t**>(const_cast<void*>(mFont->PatternTable)); // パターンテーブル
        const uint16_t* src = pptbl[CodeIndex];
        uint16_t* dst = mWriteBuff;
        for ( int idx = 0; idx < SizeX * SizeY; idx++ ) {
            uint16_t dat = *(src++);
            dat = (dat << 8) | (dat >>8);
            *(dst++) = dat;
        }
    }


    // --------------------------------------------------------
    // 文字サイズ取得
    //   CodeIndex          コードインデックス
    //   SizeX              横幅格納先
    //   SizeY              縦幅格納先
    // 補足
    // ・描画不可能な場合、(0,0)が返される。
    void GetCharSize(uint32_t CodeIndex, uint16_t& SizeX, uint16_t& SizeY )
    {
        // 状態チェック
        if ( !CanDraw() ) {
            // 描画できない場合

            // 異常値を返す
            SizeX = 0;
            SizeY = 0;
            return;
        }

        SizeX = mFont->SizeX;
        SizeY = mFont->SizeY;
        if((mFont->CodeTable[CodeIndex] & FONT_HALF) == 0 ) {
            // 全角の場合
            SizeX *= 2; // 横2倍
        }
    }


    // --------------------------------------------------------
    // 文字のテーブルインデックスを取得
    //   Code           文字コード
    //   戻り値 : コードテーブル、及び、パターンテーブルのインデックスを返す。存在しない場合、-1を返す。
    int32_t FindCodeIndex( uint32_t Code )
    {
        int32_t     sidx; // 検索範囲先頭インデックス
        int32_t     eidx; // 検索範囲最終インデックス
        int32_t     idx; // 中間位置インデックス
        uint32_t    cd;

        // 状態チェック
        if ( !CanDraw() ) return -1; // 描画不可能な場合、存在しない

        // 検索
        sidx = 0;
        eidx = mFont->Count - 1;
        while ( sidx <= eidx) {

            // 目的のコード判定
            idx = (sidx + eidx) / 2; // 中間地点
            cd = mFont->CodeTable[idx] & FONT_CODEMASK;
            if ( cd == Code ) return idx; // 目的のコードの場合、発見

            // 検索範囲更新
            if ( cd < Code ) {
                // 目的のコードが中間位置より上にある場合
                sidx = idx + 1; // 後半を探す
                
            } else {
                // 目的のコードが中間位置より下にある場合
                eidx = idx - 1; // 前半を探す
            }
        }

        // 見つからない
        return -1;
    }    

    
public:


    // --------------------------------------------------------
    // UTF-8からUnicodeへの変換
    //   Text       UTF-8文字列の格納先(次の位置に更新される)
    //   戻り値 : 取り出したUnicodeを返す。
    static uint32_t GetUnicode(const char*& Text)
    {
        uint32_t unicode = 0;

        // 引数チェック
        if (Text == nullptr) return 0;

        // 1バイト目取得
        uint8_t c1; // 一文字目
        for (;;) {
            c1 = *(Text++);
            if (c1 < 0x7F)	return c1; // 1バイト文字範囲の場合(又は終端の場合)、１バイト文字(終端の場合は0)を返す
            if ((c1 & 0xC0) == 0x80) continue; // 2文字目以降の場合、読み飛ばす
            break;
        }

        // バイト数決定
        uint8_t cc = c1 << 2; // 情報
        int len = 1; // 残りバイト数
        while ((cc & 0x80) == 0x80) { // MSBが1の間
            len++;
            cc <<= 1;
            if (len > 5) return c1; // 不正な文字(0xFE or 0xFF)の場合、１バイト文字として返す
        }
        unicode = cc >> (len + 1);

        // 2バイト目以降読み込み
        while (len > 0) {

            cc = *Text;
            if ((cc & 0xc0) != 0x80) return '!'; // 不正な値の場合(2バイト目以降に使用できないデータの場合)、不正文字を返す
            Text++;

            unicode <<= 6;
            unicode |= cc & 0x3f;

            len--;
        }

        return unicode;
    }

    // --------------------------------------------------------
    // 半角文字判定
    //   Code           文字コード
    //   戻り値 : 半角文字の場合、trueを返す。
    // 補足
    // ・１バイト文字、及び、半角カタカナを半角とみなす。
    static bool IsHalf(uint32_t Code)
    {
        // 判定
        if (Code <= 0x00ff) return true;  // 255以下の場合、半角
        if ((0xff61 <= Code) && (Code <= 0xffee)) return true; // 半角範囲の場合、半角

        // 上記以外、半角でない
        return false;
    }

};
