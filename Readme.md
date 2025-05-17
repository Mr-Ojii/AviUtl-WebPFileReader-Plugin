# AviUtl-WebPFileReader-Plugin
Animated WebP を読み込むために開発した AviUtl 用入力プラグイン

## 導入方法
1. Visual C++ 再頒布可能パッケージ 2015-2022 X86 の導入
2. Releases から最新の zip を DL
3. zip を解凍
4. 中に入っている webpinput.aui を以下に配置
    - aviutl.exe と同一のディレクトリ内
    - aviutl.exe と同一のディレクトリに配置されている plugins ディレクトリ内

## 注意点
- 導入後、以下のような汎用プラグインより優先度を高くすることを推奨します。
    * L-SMASH Works File Reader (InputPipePlugin)
    * DirectShow File Reader
    * MFVideoReader
    * FFmpeg Decoder
- ループ数は無視します。手動でループを行なってください。
- VFR は勝手に CFR として読み込みます。
    - 動画の全体の長さを保持した上で均等にフレーム長を割り振る方式です。
    - L-SMASH Works の VFR->CFR のような機能は搭載していません。
