///////////////////////////////////////////////////////////////
// テキストコンソールモジュール
///////////////////////////////////////////////////////////////
#pragma once


///////////////////////////////////////////////////////////////

#if !defined( CONSOLE_NOT_USE_ARGV )
#include <stdarg.h>
#endif

#include "LCD_Manager.h"
#include "LCD_FontData.h"
#include "LCD_TextWriter.h"

///////////////////////////////////////////////////////////////

#if !defined( CONSOLE_NOT_USE_ARGV )
#define CONSOLE_FORMAT_EXPANSION_MAX        (256) // 書式展開時の最大長
#endif

// テキストVRAMアクセスマクロ
#define VRAMINDEX(x,y)  ((x) + (y) * mCountX)
#define VRAM(x,y)       mVRAM[ VRAMINDEX( (x), (y) ) ]
#define VRAMSIZE        (mCountX * mCountY)

///////////////////////////////////////////////////////////////

// テキストVRAM描画フラグ
typedef enum
{
    CONSOLE_FLAG_SPACE      = 0x0000,   // 空白
    CONSOLE_FLAG_HALF       = 0x0001,   // 半角
    CONSOLE_FLAG_WIDE_R     = 0x0002,   // 全角右側
    CONSOLE_FLAG_WIDE_L     = 0x0004,   // 全角左側
    CONSOLE_FLAGMASK_DRAW   = 0x000F,   // 描画属性マスク

    CONSOLE_FLAG_UPDATE     = 0x8000,   // 更新あり

    CONSOLE_FLAG_UPDATESPACE    = CONSOLE_FLAG_UPDATE | CONSOLE_FLAG_SPACE,     // 空白更新
    CONSOLE_FLAG_UPDATEHALF     = CONSOLE_FLAG_UPDATE | CONSOLE_FLAG_HALF,      // 半角更新
    CONSOLE_FLAG_UPDATEWIDE_R   = CONSOLE_FLAG_UPDATE | CONSOLE_FLAG_WIDE_R,    // 全角右側更新
    CONSOLE_FLAG_UPDATEWIDE_L   = CONSOLE_FLAG_UPDATE | CONSOLE_FLAG_WIDE_L,    // 全角左側更新
} CONSOLE_FLAG;


// テキストVRAMデータ
typedef struct tagCONSOLE_DATA
{
    uint32_t        Code;       // 文字コード
    CONSOLE_FLAG    Flag;       // 描画フラグ
    uint16_t        ForeColor;  // 文字色
    uint16_t        BackColor;  // 背景色
} CONSOLE_DATA;


// カーソル移動種別
typedef enum 
{
    CONSOLE_CURSORMOVE_NONE,        // 移動なし(初期値用)
    CONSOLE_CURSORMOVE_HOME,        // 原点(0,0)
    CONSOLE_CURSORMOVE_NEXT,        // 次(スクロールあり)
    CONSOLE_CURSORMOVE_BACK,        // 前
    CONSOLE_CURSORMOVE_NEXTLINE,    // 次の行(スクロールあり)
    CONSOLE_CURSORMOVE_UP,          // 上(スクロールなし)
    CONSOLE_CURSORMOVE_DOWN,        // 下(スクロールなし)
    CONSOLE_CURSORMOVE_LEFT,        // 左(スクロールなし)
    CONSOLE_CURSORMOVE_RIGHT,       // 右(スクロールなし)
    CONSOLE_CURSORMOVE_CR,          // 行先頭
    CONSOLE_CURSORMOVE_TAB,         // 次のタブ位置へ移動
} CONSOLE_CURSORMOVE;


// 画面クリア種別
typedef enum 
{
    CONSOLE_CLEARTYPE_ALL = 0,      // 全体をクリア
    CONSOLE_CLEARTYPE_AFTER,        // カーソル位置以降をクリア
    CONSOLE_CLEARTYPE_BEFORE,       // カーソル位置以前をクリア
    CONSOLE_CLEARTYPE_LINE,         // カーソル行
    CONSOLE_CLEARTYPE_LINERIGHT,    // カーソル行の右側
    CONSOLE_CLEARTYPE_LINELEFT,     // カーソル行の左側
} CONSOLE_CLEARTYPE;


///////////////////////////////////////////////////////////////


// 文字色/背景色テーブル
static const uint16_t mConsole_Colors[] = {
    LCD_COLOR_BLACK,        // 0:黒
    LCD_COLOR_RED,          // 1:赤
    LCD_COLOR_GREEN,        // 2:緑
    LCD_COLOR_YELLOW,       // 3:黄
    LCD_COLOR_BLUE,         // 4:青
    LCD_COLOR_MAGENTA,      // 5:紫
    LCD_COLOR_CYAN,         // 6:水
    LCD_COLOR_GRAY,         // 7:灰
    LCD_COLOR_WHITE,        // 8:白
};
static const uint16_t mConsole_ColorCount = sizeof(mConsole_Colors) / sizeof(mConsole_Colors[0]);   // 色数
static const uint16_t mConsole_Default_ForeColor = LCD_COLOR_WHITE; // デフォルトの文字色
static const uint16_t mConsole_Default_BackColor = LCD_COLOR_BLACK; // デフォルトの背景色


///////////////////////////////////////////////////////////////

// ============================================================
// テキストコンソール制御クラス
class ConsoleManager
{
private:

    TextWriter&     mWriter;    // 文字描画制御
    uint16_t        mPosX;      // 表示基準位置X
    uint16_t        mPosY;      // 表示基準位置Y
    int16_t         mCountX;    // 文字数X
    int16_t         mCountY;    // 文字数Y
    CONSOLE_DATA    *mVRAM;     // テキストVRAM
    uint16_t        mForeColor; // 文字色
    uint16_t        mBackColor; // 背景色
    int16_t         mCurX;      // カーソル位置X
    int16_t         mCurY;      // カーソル位置Y
    bool            mAutoRefresh;       // true時、自動的に画面更新
    bool            mRequestRefresh;    // true時、自動更新での再描画必要
    bool            mConvertLFtoCRLF;   // true時、LF('\n')をCRLF('\r\n')として処理する
    uint8_t         mTabCount;          // タブ長

public:


    // --------------------------------------------------------
    // コンストラクタ
    //   Writer     文字描画
    ConsoleManager( TextWriter& Writer ) : 
        mWriter( Writer ),
        mPosX( 0 ), mPosY( 0 ), mCountX( 0 ), mCountY( 0 ),
        mVRAM( nullptr ),
        mCurX( 0 ) , mCurY( 0 ),
        mAutoRefresh( true ), mRequestRefresh( false ), mConvertLFtoCRLF( true ), mTabCount( 4 )
    {
        // 初期化
        Reset();
    }


    // --------------------------------------------------------
    // デストラクタ
    ~ConsoleManager()
    {
        // 開放
        Release();
    }


    // --------------------------------------------------------
    // 描画準備
    //   PosX, PosY     表示基準位置[pixel]
    //   CountX, CountY 文字数(0の場合、現在の画面サイズで最大)
    void Setup( uint16_t PosX = 0, uint16_t PosY = 0, uint16_t CountX = 0, uint16_t CountY = 0 )
    {
        // 現在の領域を解放
        Release();

        // 位置/サイズ決定
        if ( (CountX == 0) || (CountY == 0) ) {
            // 文字数が指定されていない場合

            // 描画可能サイズ取得
            uint16_t sx, sy; // 描画可能な画面サイズ
            mWriter.GetLCDManager().GetSize( sx, sy );
            const FONTDATA* fnt = mWriter.GetFontData();
            if ( fnt != nullptr ) {
                // フォントデータを取得できた場合

                // 文字サイズ取得
                uint16_t fx = fnt->SizeX;
                uint16_t fy = fnt->SizeY;

                // サイズ決定
                CountX = ( (CountX == 0) && (PosX < sx) ) ? ((sx - PosX + fx - 1) / fx) : 0; // 切り上げ
                CountY = ( (CountY == 0) && (PosY < sy) ) ? ((sy - PosY + fy - 1) / fy) : 0; // 切り上げ
            }
        }
        mPosX = PosX;
        mPosY = PosY;
        mCountX = static_cast<int16_t>(CountX);
        mCountY = static_cast<int16_t>(CountY);

        // VRAM領域確保
        if ( (mCountX > 0) && (mCountY > 0) ) {
            // 表示領域がある場合

            // 確保
            mVRAM = new CONSOLE_DATA[VRAMSIZE]; // 新しいVRAM領域
            if ( mVRAM != nullptr ) {
                // 確保できた場合、初期化
                Clear();
            }
        }
    }


    // --------------------------------------------------------
    // 描画可能文字数取得
    //   CountX, CountY     文字数格納先
    void GetCount( uint16_t& CountX, uint16_t& CountY )
    {
        // 取得
        CountX = mCountX;
        CountY = mCountY;
    }


    // --------------------------------------------------------
    // 使用可否判定
    //   戻り値 : 使用可能な場合、trueを返します。
    bool CanUse()
    {
        // 判定
        if ( mVRAM == nullptr ) return false; // VRAMが確保されていない場合、使用不可

        // 使用可
        return true;
    }


    // --------------------------------------------------------
    // 自動更新設定
    //   Enable     true時、自動更新有効
    void SetAutoRefresh( bool Enable )
    {
        // 更新
        mAutoRefresh = Enable;
    }
    
    
    // --------------------------------------------------------
    // LFからCRLFへの変換設定
    //   Enable     true時、LFはCRLFとして扱う
    void SetConvertLFtoCRLF( bool Enable )
    {
        // 更新
        mConvertLFtoCRLF = Enable;
    }


    // --------------------------------------------------------
    // タブ長設定
    //   Count          タブ長(1以上)
    void SetTab( uint8_t Count )
    {
        // 引数チェック
        if ( Count == 0 ) return; // タブ長が不正な場合、何もしない

        // 更新
        mTabCount = Count;
    }


   // --------------------------------------------------------
    // 画面クリア
    //   ClearType  種別
    void Clear( CONSOLE_CLEARTYPE Type = CONSOLE_CLEARTYPE_ALL )
    {
        // 状態確認
        if ( !CanUse() ) return; // 使用不可の場合、何もしない

        // クリア処理
        switch ( Type ) {
            case CONSOLE_CLEARTYPE_ALL:         // 全体クリア
                for ( int16_t idx = 0; idx < VRAMSIZE; idx++ ) {
                    ClearToBlank( mVRAM[idx] );
                }

                // カーソル位置を基準位置へ
                CursorMove( CONSOLE_CURSORMOVE_HOME );

                break;

            case CONSOLE_CLEARTYPE_AFTER:       // カーソル位置以降をクリア
                for ( int16_t idx = VRAMINDEX( mCurX, mCurY ); idx < VRAMSIZE; idx++ ) {
                    ClearToBlank( mVRAM[idx] );
                }
                break;

            case CONSOLE_CLEARTYPE_BEFORE:      // カーソル位置以前をクリア
                for ( int16_t idx = 0; idx <= VRAMINDEX( mCurX, mCurY ); idx++ ) {
                    ClearToBlank( mVRAM[idx] );
                }
                break;

            case CONSOLE_CLEARTYPE_LINE:        // カーソル行をクリア
                for ( int16_t idx = 0; idx < mCountX; idx++ ) {
                    ClearToBlank( VRAM(idx, mCurY) );
                }
                break;

            case CONSOLE_CLEARTYPE_LINERIGHT:   // カーソル行の右側をクリア
                for ( int16_t idx = mCurX; idx < mCountX; idx++ ) {
                    ClearToBlank( VRAM(idx, mCurY) );
                }
                break;

            case CONSOLE_CLEARTYPE_LINELEFT:    // カーソル行の左側をクリア
                for ( int16_t idx = 0; idx <= mCurX; idx++ ) {
                    ClearToBlank( VRAM(idx, mCurY) );
                }
                break;

            default: // 上記以外、何もしない
                break;
        }
    }


    // --------------------------------------------------------
    // 有効範囲判定
    //   PosX, PosY     位置
    //   戻り値 : 位置が有効範囲の場合、trueを返します。
    bool InRange( int16_t PosX, int16_t PosY )
    {
        // 判定
        if ( !CanUse() ) return false; // 使用不可の場合、何もしない
        if ( (PosX < 0) || (PosY < 0) ) return false; // 位置が負数の場合、範囲外
        if ( (mCountX <= PosX) ||
             (mCountY <= PosY) ) return false; // 位置が右端/下端以上の場合、範囲外

        // 範囲内
        return true;
    }


    // --------------------------------------------------------
    // 文字色設定
    //   ForeColor      文字色
    void SetForeColor( uint16_t ForeColor )
    {
        // 更新
        mForeColor = ForeColor;
    }


    // --------------------------------------------------------
    // 背景色設定
    //   BackColor      背景色
    void SetBackColor( uint16_t BackColor )
    {
        // 更新
        mBackColor = BackColor;
    }


    // --------------------------------------------------------
    // 描画属性のリセット
    void Reset()
    {
        // 色リセット
        mForeColor = mConsole_Default_ForeColor;
        mBackColor = mConsole_Default_BackColor;
    }



    // --------------------------------------------------------
    // 書き込み
    //   PosX, PosY     位置
    //   Value          文字情報
    //   戻り値 : 書き込まれた場合、treuを返します。
    bool Write( int16_t PosX, int16_t PosY, const CONSOLE_DATA& Value )
    {
        // 引数チェック
        if ( !InRange( PosX, PosY ) ) return false; // 範囲外の場合、失敗

        // 状態確認
        if ( !CanUse() ) return false; // 使用不可の場合、何もしない

        // 書き込み
        VRAM(PosX, PosY) = Value;

        // 成功
        return true;
    }


    // --------------------------------------------------------
    // 書き込み
    //   PosX, PosY     位置
    //   Code           文字コード
    //   Flag           描画フラグ
    //   戻り値 : 書き込まれた場合、treuを返します。
    bool Write( int16_t PosX, int16_t PosY, uint32_t Code, CONSOLE_FLAG Flag )
    {
        // 引数チェック
        if ( !InRange( PosX, PosY ) ) return false; // 範囲外の場合、失敗

        // 状態確認
        if ( !CanUse() ) return false; // 使用不可の場合、何もしない

        // 書き込み
        CONSOLE_DATA val = {
            .Code = Code,
            .Flag = Flag,
            .ForeColor = mForeColor,
            .BackColor = mBackColor
        };
        return Write( PosX, PosY, val );
    }


    // --------------------------------------------------------
    // 書込位置設定
    //   PosX, PosY     位置
    void SetLocate( int16_t PosX, int16_t PosY )
    {
        // 更新
        mCurX = PosX;
        mCurY = PosY;

        // 補正
        CorrectPosition();
    }


    // --------------------------------------------------------
    // 書込位置取得
    //   PosX, PosY     位置
    void GetLocate( int16_t& PosX, int16_t& PosY )
    {
        // 取得
        PosX = mCurX;
        PosY = mCurY;
    }


    // --------------------------------------------------------
    // 書き込み
    //   Text           書き込み文字列('\0'終端)
    void Print( const char* Text )
    {
        // 引数チェック
        if ( Text == nullptr ) return; // 文字列が指定されていない場合、何もしない

        // 一文字づつ処理
        mRequestRefresh = false;
        for (;;) {

            // 一文字取得
            uint32_t cc = GetUnicode( Text );
            if ( cc == '\0' ) break; // 終端に達した場合、終了

            // 制御文字処理
            if ( cc < 0x20 ) {
                // 制御文字の場合

                if ( cc == 0x1b ) {
                    // エスケープシーケンスの場合

                    EscapeSequence( Text );
                    cc = 0; // 書き込みなし

                } else if ( ControlChar( static_cast<uint8_t>(cc) ) ) {
                    // 制御文字として処理された場合
                    cc = 0; // 書き込みなし
                }
            }

            // 書き込み
            if ( cc != 0 ) {
                // 書き込み対象の場合

                if ( IsHalf(cc) ) {
                    // 半角の場合
                    Write( mCurX, mCurY, cc, CONSOLE_FLAG_UPDATEHALF );
                    CursorMove( CONSOLE_CURSORMOVE_NEXT );

                } else {
                    // 全角の場合
                    Write( mCurX, mCurY, cc, CONSOLE_FLAG_UPDATEWIDE_L );
                    CursorMove( CONSOLE_CURSORMOVE_NEXT );
                    Write( mCurX, mCurY, cc, CONSOLE_FLAG_UPDATEWIDE_R );
                    CursorMove( CONSOLE_CURSORMOVE_NEXT );
                }
            }
        }

        // 自動更新処理
        if ( mAutoRefresh ) {
            // 自動更新が有効な場合
            if ( mRequestRefresh ) {
                // 更新が必要な場合
                Refresh(); // 更新
            }
        } 
    }


#if !defined( CONSOLE_NOT_USE_ARGV )

    // --------------------------------------------------------
    // 書式指定書き込み
    //   Fmt            書式('\0'終端)
    //   ...            書式引数
    void Printf( const char* Fmt, ... )
    {
        // 書式展開
        char wk[CONSOLE_FORMAT_EXPANSION_MAX];
        va_list arp;
        va_start( arp, Fmt );
        (void)vsnprintf( &wk[0], sizeof(wk), Fmt, arp );
        va_end( arp );

        // 書込
        Print( &wk[0] );
    }

#endif    


    // --------------------------------------------------------
    // カーソル移動
    //   Dir        移動方向
    void CursorMove( CONSOLE_CURSORMOVE Dir )
    {
        // 移動
        switch( Dir ) {
            case CONSOLE_CURSORMOVE_HOME:       // 原点(0,0)
                mCurX = 0;
                mCurY = 0;
                break;

            case CONSOLE_CURSORMOVE_NEXT:       // 次(スクロールあり)
                mCurX++;
                if ( mCountX <= mCurX ) {
                    // 範囲外になる場合

                    // 次の行の先頭へ
                    mCurX = 0;
                    mCurY++;
                    if ( mCountY <= mCurY ) Scroll(); // 範囲外になる場合、スクロール
                }
                break;

            case CONSOLE_CURSORMOVE_BACK:       // 前
                mCurX--;
                break;

            case CONSOLE_CURSORMOVE_NEXTLINE:   // 次の行(スクロールあり)
                mCurY++;
                if ( mCountY <= mCurY ) Scroll(); // 範囲外になる場合、スクロール
                break;

            case CONSOLE_CURSORMOVE_UP:         // 上(スクロールなし)
                mCurY--;
                break;

            case CONSOLE_CURSORMOVE_DOWN:       // 下(スクロールなし)
                mCurY++;
                break;

            case CONSOLE_CURSORMOVE_LEFT:       // 左(スクロールなし)
                mCurX--;
                break;

            case CONSOLE_CURSORMOVE_RIGHT:      // 右(スクロールなし)
                mCurX++;
                break;

            case CONSOLE_CURSORMOVE_CR:         // 行先頭            
                mCurX = 0;
                break;

            case CONSOLE_CURSORMOVE_TAB:        // 次のタブ位置へ移動
                mCurX = ((mCurX + mTabCount) / static_cast<int16_t>(mTabCount)) * static_cast<int16_t>(mTabCount);
                break;

            default: // 上記以外
                return; // 何もしない
        }

        // 補正
        CorrectPosition();
    }
    

    // --------------------------------------------------------
    // 上方向にスクロール
    void Scroll()
    {
        // データスクロール
        for ( int16_t y = 0; y < mCountY - 1; y++ ) {
            for ( int16_t x = 0; x < mCountX; x++ ) {
                Copy( VRAM( x, y ), VRAM( x, y + 1 ) );
            }
        }

        // 最終行のクリア
        for ( int16_t x = 0; x < mCountX; x++ ) {
            ClearToBlank( VRAM( x, mCountY - 1) );
        }

        // カーソル位置補正
        CorrectPosition();

        // 自動更新時は、要再描画
        mRequestRefresh = true;
    }


    // --------------------------------------------------------
    // 画面更新
    //   Force      true時、全体再描画
    void Refresh(bool Force = false)
    {
        // 状態確認
        if ( !CanUse() ) return; // 使用不可の場合、描画しない

        // 再描画確認
        if ( Force ) {
            // 強制再描画の場合、全体を再描画対象へ
            SetUpdateFlag(); 
        }

        // 描画準備
        uint16_t fx, fy;
        const FONTDATA* fnt = mWriter.GetFontData();
        if ( fnt == nullptr ) return; // フォント情報が取得できない場合、中止
        fx = fnt->SizeX;
        fy = fnt->SizeY;
        LCDManager& lcd = mWriter.GetLCDManager();

        // 描画
        uint16_t px;
        uint16_t py = mPosY;
        bool wd; // true時、全角描画
        for ( int16_t y = 0; y < mCountY; y++ ) {
            px = mPosX;
            for (int16_t x = 0; x < mCountX; x++ ) {

                // 描画判定
                CONSOLE_DATA& dd = VRAM( x, y ); // 描画位置の文字情報
                if ( (dd.Flag & CONSOLE_FLAG_UPDATE) != 0 ) {
                    // 描画が必要な場合

                    // 更新フラグ除去
                    ResetUpdateFlag( dd );

                    // １文字描画
                    CONSOLE_FLAG ff = static_cast<CONSOLE_FLAG>(dd.Flag & CONSOLE_FLAGMASK_DRAW ); // 描画フラグ
                    if ( ff == CONSOLE_FLAG_SPACE ) {
                        // 空白の場合
                        lcd.FillRectangle( px, py, fx, fy, dd.BackColor );  // 消去

                    } else if ( ff == CONSOLE_FLAG_HALF ) {
                        // 半角文字の場合
                        mWriter.Draw( px, py, dd.Code, dd.ForeColor, dd.BackColor ); // 描画

                    } else if ( ff == CONSOLE_FLAG_WIDE_L ) {
                        // 全角左側の場合

                        // 描画方法決定
                        wd = false;
                        if ( InRange(x + 1, y) ) {
                            // 右隣が範囲内の場合
                            CONSOLE_DATA& dd2 = VRAM(x + 1, y); // 右隣の文字情報
                            if ( (dd.Code == dd2.Code) &&               // 同一コード、かつ、
                                 (dd.ForeColor == dd2.ForeColor) &&     // 同一文字色、かつ、
                                 (dd.BackColor == dd2.BackColor) &&     // 同一背景色、かつ、
                                 (dd2.Flag == CONSOLE_FLAG_UPDATEWIDE_R)) {  // 全角右側で描画が必要な場合

                                // 全角描画
                                wd = true;
                                ResetUpdateFlag( dd2 );
                            }
                        } 

                        // 描画
                        if ( wd ) {
                            // 全角描画可能の場合
                            mWriter.Draw( px, py, dd.Code, dd.ForeColor, dd.BackColor ); // 全角描画
                            px += fx;
                            x++;

                        } else {
                            // 全角描画不可の描画
                            mWriter.Draw( px, py, dd.Code, dd.ForeColor, dd.BackColor, 0 ); // 左側のみ描画
                        }

                    } else if ( ff == CONSOLE_FLAG_WIDE_R ) {
                        // 全角右側の場合
                        mWriter.Draw( px, py, dd.Code, dd.ForeColor, dd.BackColor, 1 ); // 右側のみ描画
                    }
                }

                // 描画位置(横)更新
                px += fx;

            }

            // 描画位置(縦)更新
            py += fy;
        }
    }


    void DebugPrint()
    {
        for ( uint16_t y = 0; y < 3; y++ ) {
            for ( uint16_t x = 0; x < 6; x++ ) {
                CONSOLE_DATA& dd = VRAM( x, y );
                printf( "[%08lX:%04X]", dd.Code, dd.Flag );
            }
            printf( "\n" );
        }
    }


private:


    // --------------------------------------------------------
    // 開放処理
    void Release()
    {
        // VRAMクリア
        if ( mVRAM != nullptr ) {
            // 確保されている場合

            // 開放
            delete[] mVRAM;
            mVRAM = nullptr;
        }

        // カーソル位置クリア
        mCurX = 0;
        mCurY = 0;
    }


    // --------------------------------------------------------
    // 文字情報のクリア
    //   Data       文字情報
    void ClearToBlank( CONSOLE_DATA& Data )
    {
        Data.Code = ' ';
        Data.Flag = CONSOLE_FLAG_UPDATESPACE;
        Data.ForeColor = mForeColor;
        Data.BackColor = mBackColor;
    }


    // --------------------------------------------------------
    // 文字情報コピー
    //   Dst        コピー先
    //   Src        コピー元
    void Copy( CONSOLE_DATA& Dst, CONSOLE_DATA& Src )
    {
        // 最適化
        if ( (Dst.Flag == CONSOLE_FLAG_SPACE) && (Src.Flag == CONSOLE_FLAG_SPACE) ) {
            // どちらも描画不要の空白の場合、何もしない
            return;
        }

        // コピー
        Dst = Src;
        SetUpdateFlag( Dst );
    }


    // --------------------------------------------------------
    // 描画フラグ設定
    void SetUpdateFlag( CONSOLE_DATA& Data )
    {
        // 解除
        Data.Flag = static_cast<CONSOLE_FLAG>( Data.Flag | CONSOLE_FLAG_UPDATE );
    }


    // --------------------------------------------------------
    // 全体の描画フラグ設定
    void SetUpdateFlag()
    {
        for ( int16_t y = 0; y < mCountY; y++ ) {
            for ( int16_t x = 0; x < mCountX; x++ ) {
                SetUpdateFlag( VRAM( x, y ) );
            }
        }
    }


    // --------------------------------------------------------
    // 描画フラグ除去
    void ResetUpdateFlag( CONSOLE_DATA& Data )
    {
        // 解除
        Data.Flag = static_cast<CONSOLE_FLAG>(Data.Flag & ~CONSOLE_FLAG_UPDATE);
    }


    // --------------------------------------------------------
    // カーソル位置補正
    void CorrectPosition()
    {
        // 補正
        if ( mCurX < 0 ) {
            mCurX = 0;

        } else if ( mCountX <= mCurX ) {
            mCurX = mCountX - 1;
        }
        if ( mCurY < 0 ) {
            mCurY = 0;

        } else if ( mCountY <= mCurY ) {
            mCurY = mCountY - 1;
        }
    }


    // --------------------------------------------------------
    // 制御文字処理
    //   Code       制御文字
    //   戻り値 : 制御文字として処理された場合、trueを返します。
    bool ControlChar( uint8_t Code )
    {
        bool ret = true; // 処理の有無(true:処理済み)

        switch ( Code ) {
            case '\b':  // BS(0x08)の場合
                CursorMove( CONSOLE_CURSORMOVE_BACK ); // 前に戻る
                break;

            case '\t':  // HT(0x09)の場合   
                CursorMove( CONSOLE_CURSORMOVE_TAB ); // 次のタブ位置へ移動
                break;

            case '\n':  // LF(0x0A)の場合
                CursorMove( CONSOLE_CURSORMOVE_NEXTLINE ); // 次の行へ移動(スクロールあり)
                if ( mConvertLFtoCRLF ) {
                    // LFをCRLFとして処理する場合
                    CursorMove( CONSOLE_CURSORMOVE_CR ); // 行先頭へ移動
                }
                mRequestRefresh = true;
                break;

            case '\r':  // CR(0x0D)の場合
                CursorMove( CONSOLE_CURSORMOVE_CR ); // 行先頭へ移動
                mRequestRefresh = true;
                break;

            default: // 上記以外
                ret = false; // 処理なし
                break;
        }

        return ret;
    }


    // --------------------------------------------------------
    // エスケープシーケンス処理
    //   Text       文字列('\033'の次の位置であること。'\0'終端。エスケープシーケンスの次の位置に更新される)
    void EscapeSequence( const char* &Text )
    {
        // 引数チェック
        if ( Text == nullptr ) return; // 文字列が指定されていない場合、何もしない

        // エスケープシーケンスのパラメタと種別取得
        const char* sp = Text; // 取得前の位置
        int8_t code = -1; // 種別(-1:種別なし(処理なし))
        int16_t param[2] = {0}; // パラメタ(-1:パラメタなし)
        int16_t paramcnt = 0; // パラメタ数
        uint32_t cc = GetUnicode( Text ); // 最初の文字
        if ( cc == '\0' ) return; // 終端に達した場合、何もしない
        if ( cc == '[' ) {
            // 対応するエスケープシーケンスの場合

            for(;;) {

                cc = GetUnicode( Text );
                if ( cc == '\0' ) break; // 終端に達した場合、中止

                if ( ('0' <= cc) && (cc <= '9')) {
                    // 数字の場合
                    // 桁追加
                    if ( paramcnt == 0 ) paramcnt++;
                    param[paramcnt-1] *= 10;
                    param[paramcnt-1] += static_cast<uint16_t>(cc - '0');
                    if ( param[paramcnt-1] > 999 ) break; // 指定可能最大を超えた場合、中止

                } else if ( cc == ';' ) {
                    // パラメタ区切りの場合
                    if ( (sizeof(param) / sizeof(param[0])) <= paramcnt ) break; // パラメタ数が最大を超える場合、中止
                    paramcnt++; // パラメタ追加

                } else {
                    // 上記以外の場合
                    if ( 0x20 <= cc ) {
                        // 制御コードでない場合
                        // コートと認識
                        code = static_cast<uint8_t>( cc );
                    }
                    break; // 処理へ
                }
            }
        }

        // エスケープシーケンスの処理
        if ( !EscapeSequence( code, param, paramcnt ) ) {
            // 処理されなかった場合、処理前に戻して、中止
            Text = sp;
            return;
        }

        // 成功
    }


    // --------------------------------------------------------
    // エスケープシーケンス処理
    //   Code       種別
    //   Params     パラメタ配列
    //   ParamCount パラメタ数
    //   戻り値 : 処理された場合、trueを返します。
    bool EscapeSequence( uint8_t Code, const int16_t* Params, int16_t ParamCount )
    {
        // 引数チェック
        if ( (ParamCount > 0) && (Params == nullptr) ) return false; // パラメタ配列が指定されていない場合、処理なし

        // 種別毎の処理
        CONSOLE_CLEARTYPE ct = CONSOLE_CLEARTYPE_ALL; // 消去方法
        int16_t cnt = 0; // 回数
        int16_t px, py; // 位置
        int16_t val; // 属性等
        switch ( Code ) {
            case 'J': // 消去

                // クリア種別決定
                if ( ParamCount > 0 ) {
                    // 引数がある場合
                    switch( Params[0] ) {
                        case 0: ct = CONSOLE_CLEARTYPE_AFTER; break;
                        case 1: ct = CONSOLE_CLEARTYPE_BEFORE; break;
                        case 2: ct = CONSOLE_CLEARTYPE_ALL; break;
                        default: break;
                    }
                }

                // クリア
                Clear( ct );
                break;

            case 'K': // 行消去

                // クリア種別決定
                if ( ParamCount > 0 ) {
                    // 引数がある場合
                    switch( Params[0] ) {
                        case 0: ct = CONSOLE_CLEARTYPE_LINERIGHT; break;
                        case 1: ct = CONSOLE_CLEARTYPE_LINELEFT; break;
                        case 2: ct = CONSOLE_CLEARTYPE_LINE; break;
                        default: break;
                    }
                }

                // クリア
                Clear( ct );
                break;

            case 'A':   // カーソル移動：上
            case 'B':   // カーソル移動：下
            case 'C':   // カーソル移動：右
            case 'D':   // カーソル移動：左

                // 移動回数決定
                cnt = 1;
                if ( ParamCount > 0 ) cnt = Params[0];

                // 移動方向決定
                CONSOLE_CURSORMOVE dir;
                switch (Code ) {
                    case 'A': dir = CONSOLE_CURSORMOVE_UP; break;
                    case 'B': dir = CONSOLE_CURSORMOVE_DOWN; break;
                    case 'C': dir = CONSOLE_CURSORMOVE_LEFT; break;
                    case 'D': dir = CONSOLE_CURSORMOVE_RIGHT; break;
                    default: // ありえない
                        dir = CONSOLE_CURSORMOVE_NONE;
                        break;
                } 

                // 移動
                for ( int16_t idx = 0; idx < cnt; idx++ ) {
                    CursorMove( dir );
                }
                break;

            case 'H':   // カーソル移動：任意位置

                // 位置決定
                px = (ParamCount > 0) ? Params[0]:1;
                py = (ParamCount > 1) ? Params[1]:1;
                if ( px == 0 ) px = 1;
                if ( py == 0 ) py = 1;
                
                // 位置更新
                SetLocate( px, py );
                break;

            case 'm':   // 属性変更

                // 属性決定
                val = (ParamCount > 0) ? Params[0]:0; 

                // 属性毎の処理
                if ( val == 0 ) {
                    // リセット
                    Reset();

                } else if ( (30 <= val) && (val <= 39) ) {
                    // 文字色指定の場合
                    if ( val == 39 ) {
                        // デフォルト指定の場合
                        SetForeColor( mConsole_Default_ForeColor ); // デフォルト

                    } else {
                        // デフォルト以外指定の場合
                        val -= 30; // 色インデックスに変換
                        if ( (0 <= val) && (val < mConsole_ColorCount) ) {
                            // 有効範囲の場合
                            SetForeColor( mConsole_Colors[val] ); // 文字色設定
                        }
                    }

                } else if ( (40 <= val) && (val <= 49) ) {
                    // 背景色指定の場合
                    if ( val == 49 ) {
                        // デフォルト指定の場合
                        SetBackColor( mConsole_Default_BackColor ); // デフォルト

                    } else {
                        // デフォルト以外指定の場合
                        val -= 40; // 色インデックスに変換
                        if ( (0 <= val) && (val < mConsole_ColorCount) ) {
                            // 有効範囲の場合
                            SetBackColor( mConsole_Colors[val] ); // 文字色設定
                        }
                    }
                } else {
                    // 上記以外の場合、処理不可
                    return false;
                }

                break;

            default: // 上記以外、処理なし
                return false;
        }

        // 処理あり
        return true;
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
        if ( Text == nullptr ) return 0;

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



