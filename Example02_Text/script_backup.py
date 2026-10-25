import os
import zipfile
from pathlib import Path

# 定義
srczip = 'data/source.zip'
excludedir = { ".pio", "data", "temp", "download" }

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
