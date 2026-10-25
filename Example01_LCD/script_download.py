import os
import zipfile
from pathlib import Path
import subprocess
from os.path import isfile, join
import shutil



# 定義
download_dir = "download"
temp_dir = "temp"

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
