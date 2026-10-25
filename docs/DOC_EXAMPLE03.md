# Example03_Touch : タッチパネルサンプル

## サンプルについて

このサンプルは、タッチパネルがタッチされた時、  
その座標をコンソールに出力するサンプルです。

タッチパネル制御部分は、Touch_Manager.h にコードがあります。(ヘッダオンリー)  
ESP32-2432S028専用なので、初期化等にピン番号とかの引数もありません。  


## XPT2046
ESP32_2432S028には抵抗膜方式のタッチパネルが搭載されています。  
抵抗膜方式は沢山のスイッチが画面上にあるようなもので、基本的に「押さないと」反応しません。  
(対して静電方式は「触れば」反応する)

ないことはないはずですが、通常低硬膜方式は複数のタッチには対応していません。  
ESP32-2432S028に搭載されているタッチパネルもマルチタッチは検知できません。

XPT2046はSPI接続なので、LCDと同様にフレームワークで用意されている関数を使用しています。  
ただしLCDとは異なるピンであるため、LCDとは異なるバス(ホストID)で使用します。  
サンプルでは下記の様な使い分けをしています。  

| デバイス | SPI ホストID |
| --- | --- |
| LCD | SPI3_HOST |
| Touch | SPI2_HOST |

なおデータシートは下記あたりで見れます。  
[XPT2046 Data Sheet (PDF)](https://www.google.com/url?sa=i&source=web&rct=j&url=CAESeAHrOzAVUBDK1e4StAeDCiE4TzzmtEqIIFVUY1w8G-3DL16bm3kgsbGCu09oISJDNMFkB_laih-pyZP8ZwEhZzBepHRX5bQhBzgWqSZ9IGNYl2WzlFxXGearXlqY8L1d-U0c0EJSLnjSufbM6b0iFtTYD2bW2Sq4Sg&uoh=2&ved=2ahUKEwj09KTRx8-XAxWKj68BHceIInMQy_kOegoIAggACAAIEhAA&opi=89978449&cd&psig=AOvVaw337f5zOJxLysM_ZPd4Fd5_&ust=1792824978742000)  
[AllDatasheet - XPT2046](https://www.google.com/url?sa=i&source=web&rct=j&url=CAESgAEB6zswFWF-ubD9P2vVKur7dpUVUhDr4wDnkmhru2tHvykpy3CeMXtUL8OMAE8MU_iy1XX4JfNhRH7q5DqwnnhKokkR8N_DkVcvcvgc4CCJNmNlHb566Hesq_AZil_MPH5rOspLHLTCPvJydOZipWo9nWa4FPGQWvnWE_xpQzLzqg&uoh=2&ved=2ahUKEwj09KTRx8-XAxWKj68BHceIInMQy_kOegoIAggACAAIFBAA&opi=89978449&cd&psig=AOvVaw337f5zOJxLysM_ZPd4Fd5_&ust=1792824978742000)


本来は割り込みとか使って押下を検知するのがスマートなんですが、  
サンプルではポーリングでタッチ情報を取得しています。  
また必要最低限の情報しか取得していません。  
（デバイス的には押された強さも取得できるはずなんですが取っ手いません）  

ESP32-2432S028よりもちょっと良い ESP32-3248S035というモジュールも持っているのですが、  
そっちは静電方式でマルチタップや圧なんかもばっちり取れます。   
実はサンプルのタッチ制御モジュール(TouchManager)は  
元々ESP32-3248S035で作ったモジュールを流用したものです。  
なのでちょっと余計なもの(マルチタップ関連)が含まれています。  
適当に流してください。  

