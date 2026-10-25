###############################################################
# PlatformIOプロジェクトを ESP32-3248S035(CYD) 用にセットアップ
###############################################################


###############################################################
# 定義
Set-Variable -Name PROJECT_TYPE -Value "CYD ESP32-2432S028" -Option Constant
Set-Variable -Name ENV_NAME -Value "ESP32_2432S028" -Option Constant
Set-Variable -Name BOARD_NAME -Value "ESP32-2432S028" -Option Constant
Set-Variable -Name COM_PORT -Value "COM5" -Option Constant # 空白時は指定なし

Set-Variable -Name SCRIPT_BACKUP -Value "script_backup.py" -Option Constant
Set-Variable -Name SCRIPT_DOWNLOAD -Value "script_download.py" -Option Constant

Set-Variable -Name BACKUP_NAME -Value "source.zip" -Option Constant
Set-Variable -Name FOLDER_DOWNLOAD -Value "download" -Option Constant
Set-Variable -Name FOLDER_TEMP -Value "temp" -Option Constant


###############################################################
# 汎用関数

# -------------------------------------------------------------
# ファイル作成
function Create {
    [CmdletBinding()]
    Param( 
        [Parameter(Mandatory=$true)][string] $Name, 
        [Parameter(Mandatory=$true)][string] $Text, 
        [Parameter(Mandatory=$true)][string] $File,
        [string] $Encoding = "utf8"
    )

    # 存在確認
    $tt = "[作成]"
    if ( Test-Path "${File}" ) { $tt = "[更新]" }

    # 表示
    #Write-Output "${tt} ${Name} : ${File}"
    Write-Output "${tt} ${File}"

    # 出力
    if ( $Encoding -eq "utf8" ) {
        # UTF-8の場合、BOMなしで出力
        $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
        [System.IO.File]::WriteAllText( "${File}", "${Text}", $utf8NoBom)

    } else {
        # UTF-8以外の場合
        "${Text}" | Out-File -FilePath "${File}" -Encoding $Encoding
    }
}


# -------------------------------------------------------------
# ファイル/フォルダ削除
function Delete {
    [CmdletBinding()]
    Param( 
        [Parameter(Mandatory=$true)][string] $Name, 
        [Parameter(Mandatory=$true)][string] $Path
    )

    # 存在確認
    if ( !(Test-Path "${Path}") ) { return }

    # 表示
    #Write-Output "[削除] ${Name} : ${Path}"
    Write-Output "[削除] ${Path}"

    # 削除
    Remove-Item -Path "${Path}" -Recurse -Force -ErrorAction SilentlyContinue
}


###############################################################
# 対象の確認
[String]$Target = $args[0]
if ( [String]::IsNullOrWhiteSpace( "${Target}" ) ) { 
    # 引数が指定されなかった場合、カレントディレクトリで補完
    $Target = $PWD
}
if ( !(Test-Path "${Target}") ) {
    # 対象フォルダが存在しない場合
    $Target = $null # 引数不正

} else {
    # 存在する場合
    $Target = (Get-Item "${Target}").FullName # フルパス取得
    if ( !(Test-Path "${Target}/platformio.ini") ) {
        # 設定ファイルが存在しない場合、終了
        $Target = $null # 引数不正
    }
}
if ( [String]::IsNullOrWhiteSpace( "${Target}" ) ) {
    # 引数不正の場合
    Write-Output "PlatformIOで作成したプロジェクトのフォルダを指定してください。"
    Write-Output "  Name      : (適当な名前)"
    Write-Output "  Board     : Espressif ESP32 Dev Module"
    Write-Output "  Framework : Espidf"
    Write-Output "  Location  : (チェックするとC:\Users\(user)\Documents\PlatformIO\Projects下)"
    pause
    exit
}


###############################################################
# 実行確認
$sel_yes = New-Object System.Management.Automation.Host.ChoiceDescription "&Yes","はい"
$sel_no  = New-Object System.Management.Automation.Host.ChoiceDescription "&No","いいえ"
$selitems = [System.Management.Automation.Host.ChoiceDescription[]]($sel_yes, $sel_no)
$sel = $host.ui.PromptForChoice( "実行確認", "下記フォルダのプロジェクトを ${PROJECT_TYPE} 用に変更します。`n`n${Target}`n`nよろしいですか？", $selitems, 1 )
if ( $sel -ne 0 ) {
    # 許可しなかった場合、終了
    Write-Output "中止します。"
    pause
    exit
}
Write-Output "セットアップ開始"


###############################################################
# ボード定義の準備
$boarddir = "${Target}/boards"
New-Item -Path "${boarddir}" -ItemType Directory -Force | Out-Null
Create "ボード定義" @"
{
  "build": {
    "core": "esp32",
    "f_cpu": "240000000L",
    "f_flash": "40000000L",
    "flash_mode": "dio",
    "mcu": "esp32",
    "variant": "esp32"
  },
  "connectivity": [
    "wifi",
    "bluetooth"
  ],
  "frameworks": [
    "arduino",
    "espidf"
  ],
  "name": "${BOARD_NAME}",
  "upload": {
    "flash_size": "4MB",
    "maximum_ram_size": 327680,
    "maximum_size": 4194304,
    "require_upload_port": true,
    "speed": 921600
  },
  "url": "https://example.com",
  "vendor": "Custom Vendor"
}

"@ "${boarddir}/${BOARD_NAME}.json"


###############################################################
# パーティション定義の準備
Create "パーティション定義定義" @"
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,           ,   20K,
factory,  app,  factory,       , 2048K,
spiffs,   data, spiffs,        , 1984K,

"@ "${Target}/partitions.csv"

# 実際のメモリ配置
# 00000000 - 00008FFF 00009000(  36K) システム領域:予約済み (ブートローダ等)
# 00009000 - 0000DFFF 00005000(  20K) nvs (システム設定等の保存)
# 0000E000 - 0000FFFF 00002000(   8K) otadata:予約済み (OTAデータ)
# 00010000 - 0020FFFF 00200000(2048K) app (プログラム格納先) 
# 00210000 - 003FFFFF 001F0000(1984K) SPIFFS (内蔵ファイルシステム)
# -----------------------------------------
#                       Total  4096K = 4MB

# OTA未対応(OTA用領域は固定の為、存在)


###############################################################
# ビルドオプションの準備

# 既存のビルドオプションファイルを削除
Delete "既存ビルドオプション" "${Target}/sdkconfig.*"

# 差分ファイル作成
Create "ビルドオプション差分" @"
; PSRAM enable
CONFIG_SPIRAM=y
CONFIG_SPIRAM_IGNORE_NOTFOUND=y

; CPU speed
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240

; Long file name
;CONFIG_FATFS_LFN_STACK=y
;CONFIG_FATFS_MAX_LFN=255

; Japanese file name
;CONFIG_FATFS_API_ENCODING_UTF_8=y
;CONFIG_FATFS_CODEPAGE_932=y
;CONFIG_FATFS_CODEPAGE=932

"@ "${Target}/sdkconfig.defaults"

# ESP32-2432S028にはPSRAMは搭載されていないが、有効にだけしておく


###############################################################
# spiffs用ツールの準備

# ソースバックアップ機能
Create "ソースバックアップ機能" @"
import os
import zipfile
from pathlib import Path

# 定義
srczip = 'data/${BACKUP_NAME}'
excludedir = { ".pio", "data", "${FOLDER_TEMP}", "${FOLDER_DOWNLOAD}" }

# PlatformIO環境取得
Import("env")

# プロジェクトバックアップ処理
def BackupProject(source, target, env):
    # 既存アーカイブファイル削除
    if os.path.exists( srczip ):
        os.remove( srczip )

    # 新アーカイブファイル作成
    with zipfile.ZipFile( srczip, "w", compression=zipfile.ZIP_DEFLATED ) as zf:
        for fpath in Path(".").rglob("*"):
            if any( part in excludedir for part in fpath.parts):
                continue

            if fpath.is_file():
                arcname = fpath.relative_to(Path("."))
                zf.write( fpath, arcname )
                print( f"compress... {arcname}" )

	# 完了
    print( "----------------------------------------------------------------" )
    print( f"\033[35m現在のソースのバックアップを ./{srczip} に作成しました。\033[0m" )
    print( f"\033[35mPlatformIOメニューの[Platform]-[Upload Filesystem Image]にて、\033[0m" )
    print( f"\033[35mターゲットのspiffs領域に保存できます。\033[0m" )


# メニュー登録処理
env.AddCustomTarget(
    name="backupproject",
    dependencies=None,
    actions=[BackupProject],
    title="Backup project to ./" + srczip,
    description="Back up the project to store the source code in SPIFS."
)

"@ "${Target}/${SCRIPT_BACKUP}"

# SPIFFS領域ファイルのダウンロード機能
Create "SPIFFS領域ダウンロード機能" @"
import os
import zipfile
from pathlib import Path
import subprocess
from os.path import isfile, join
import shutil



# 定義
download_dir = "${FOLDER_DOWNLOAD}"
temp_dir = "${FOLDER_TEMP}"

# PlatformIO環境取得
Import("env")
platform = env.PioPlatform()
board = env.BoardConfig()
mcu = board.get("build.mcu", "esp32")
esptoolpy = join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
mkspiffs =  join(platform.get_package_dir("tool-mkspiffs") or "", "mkspiffs_espressif32_espidf.exe" )
upload_port = join(env.get("UPLOAD_PORT", "none"))
download_speed = join(str(board.get("download.speed", "921600")))
if "none" in upload_port:
    env.AutodetectUploadPort()
    upload_port = join(env.get("UPLOAD_PORT", "none"))
pythonexe = env["PYTHONEXE"]
tempdir = join(env["PROJECT_DIR"], temp_dir)
partitiontablefile = join( tempdir, "partition_table.bin")
partitionfile = join( tempdir, "spiffs_partition.bin")
print( f"upload port    : {upload_port}" )
print( f"download speed : {download_speed}" )
print( f"esptool  : {esptoolpy}" )
print( f"mkspiffs : {mkspiffs}" )

# フォルダ作成
def CreateCleanDir(folder):

    # 削除
    if os.path.exists( folder ):
        # 存在する場合、削除
        shutil.rmtree( folder )

    # 新規作成
    os.makedirs( folder )


# パーティションテーブル取得
def GetPartitionTable():

    # ダウンロード
    cmd = [
            pythonexe, esptoolpy,
            "--chip", mcu,
            "--port", upload_port,
            "--baud", download_speed,
            "--before", "default_reset",
            "--after", "hard_reset",
            "read_flash", "0x8000", "0x1000",
            partitiontablefile
    ]
    subprocess.call( cmd, shell=False )


# パーティションテーブルからSPIFFSパーティションの位置とサイズを取得
def GetSPIFFSPartionInfo(content) -> tuple[int,int]:

    # SPIFFSパーティションを検索
    partitions = [e for e in content.split(b'\xaaP') if len(e) > 0]
    for partition in partitions:
        ptype = partition[1]
        if ptype == 0x82: 
            # SPIFFSの場合、発見
            offset = int.from_bytes(partition[2:5], byteorder="little", signed=False)
            size = int.from_bytes(partition[6:9], byteorder="little", signed=False)
            return offset, size
        
    # 見つからない
    return 0, 0


# SPIFFSパーティションダウンロード
def DownloadPatition( offset, size ):

    # ダウンロード
    cmd = [
            pythonexe, esptoolpy,
            "--chip", mcu,
            "--port", upload_port,
            "--baud", download_speed,
            "--before", "default_reset",
            "--after", "hard_reset",
            "read_flash", hex(offset), hex(size),
            partitionfile
    ]
    subprocess.call( cmd, shell=False )


# SPIFFSパーティションからファイル取り出し
def UnpackFiles(image, outputdir):
    
    # 出力先作成
    CreateCleanDir( outputdir )

    # ファイル生成
    fsz = os.path.getsize( image )
    cmd = [
        mkspiffs,
        "-b", "4096",
        "-p", "256",
        "-s", str(fsz),
        "--unpack", outputdir, image
    ]
    subprocess.call( cmd, shell=True )


# SPIFFSのファイルダウンロード
def DownloadSPIFFS(source, target, env):
    
    # 準備(一時フォルダ作成)
    CreateCleanDir(tempdir)

    # パーティションテーブル取得
    GetPartitionTable()

    # SPIFFSのパーティション情報取得
    with open(partitiontablefile, mode="rb") as file:
        content = file.read()
        offset, size = GetSPIFFSPartionInfo(content)
    print( f"SPIFFS partition offset={hex(offset)}, size={hex(size)}")

    # SPIFFSパーティションをダウンロード
    DownloadPatition( offset, size )

    # ファイル出力
    UnpackFiles( partitionfile, download_dir )

	# 一時フォルダ削除
    shutil.rmtree( tempdir )

	# 完了
    print( "----------------------------------------------------------------" )
    print( f"\033[35mターゲットのspiffs領域にあるファイルを ./{download_dir} に取得しました。\033[0m" )


# メニュー登録処理
env.AddCustomTarget(
    name="download_spiffs",
    dependencies=None,
    actions=[DownloadSPIFFS],
    title="Download SPIFFS files to ./" + download_dir,
    description="Download the files located in spiffs on device."
)

"@ "${Target}/${SCRIPT_DOWNLOAD}"

# SPIFFS領域用フォルダ作成
New-Item -Path "${Target}/data" -ItemType Directory -Force | Out-Null


###############################################################
# platformio.iniの更新
Create "設定ファイル" @"
[env:${ENV_NAME}]
platform = espressif32
board = ${BOARD_NAME}
framework = espidf
monitor_speed = 115200
$(if( ![string]::IsNullOrEmpty("${COM_PORT}") ) { "monitor_port = ${COM_PORT}" } else { "#monitor_port = " } )
$(if( ![string]::IsNullOrEmpty("${COM_PORT}") ) { "upload_port = ${COM_PORT}" } else { "#upload_port = " } )
board_build.partitions = partitions.csv
board_build.esp-idf.sdkconfig_defaults = sdkconfig.defaults
extra_scripts = ${SCRIPT_BACKUP}, ${SCRIPT_DOWNLOAD}
#build_flags = -I"`${PROJECT_DIR}/../Include"

"@ "${Target}/platformio.ini"


###############################################################
# ソース準備

# cppソース作成
Create "cppソースファイル" @"
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
    size_t sram_total = heap_caps_get_total_size( MALLOC_CAP_INTERNAL );
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

"@ "${Target}/src/main.cpp"

# cソース削除
Delete "Cソースファイル" "${Target}/src/main.c"


###############################################################
# 再作成させたいファイルの削除
Delete "既存ビルドファイル" "${Target}/.pio"
Delete "vscode設定" "${Target}/.vscode"


###############################################################
# 終了
Write-Output "完了"
pause
