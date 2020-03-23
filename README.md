# minOSv2

x86_64 アーキテクチャで UEFI 対応のシンプルな OS です。

[minOS](https://github.com/Totsugekitai/minOS) を書き直して作られています。

`master` ブランチは GPD MicroPC にて実機確認済みです。

## ビルド方法

### ブートローダのビルド

#### EDK2の環境変数設定

まず `edk2/` ディレクトリに入って `source edksetup.sh` を叩き、環境変数を設定する。
以降はこの環境変数を読み込んだシェル上で操作を行う。

#### ブートローダの `make`

ルートディレクトリで `make boot` です。

### カーネルのビルド

ルートディレクトリで `make kernel` です。

## 実行方法

ルートディレクトリで `make run` です。

## 掃除

ルートディレクトリで `make clean` です。
