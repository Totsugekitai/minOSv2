# minOSv2

x86_64 アーキテクチャで UEFI 対応のシンプルな OS です。

[minOS](https://github.com/Totsugekitai/minOS) を書き直して作られています。

`master` ブランチは GPD MicroPC にて実機動作確認済みです。

`develop` ブランチは QEMU にて動作確認済みです。

## ビルド方法

### カーネルのビルド

ルートディレクトリで `make kernel` です。

### ブートローダのビルド

#### EDK2をクローンする

```
$ git clone https://github.com/tianocore/edk2.git
```

#### EDK2の環境変数設定

まず `edk2/` ディレクトリに入って以下を行う。

```
$ git submodule update --init
$ make -C BaseTools/Source/C
```

で必要なものをビルドする。

次に`source edksetup.sh` を叩き、環境変数を設定する。
以降はこの環境変数を読み込んだシェル上で操作を行う。

#### `target.txt` を編集する

`Conf/target.txt` が生成されていると思うので、それを以下のように編集する。

```
# -- snip --
ACTIVE_PLATFORM       = MinLoaderPkg/MinLoaderPkg.dsc
# -- snip --
TARGET_ARCH           = X64
# -- snip --
TOOL_CHAIN_TAG        = GCC5
```

#### ブートローダの `make`

`minOSv2` のルートディレクトリで `make boot` です。
これでEDK2でビルドされたブートローダが `minOSv2/fs/` 以下にコピーまでされます。

## QEMU での実行方法

`minOSv2/Makefile` の `QEMU` 変数を各自の環境に合わせてパスを書き換えて下さい。

`minOSv2` のルートディレクトリで `make run` です。

## 掃除

`minOSv2` のルートディレクトリで `make clean` です。

## テスト

minOSv2は一部の処理について単体テストができます。

テストには [GoogleTest](https://github.com/google/googletest) を用いています。

### テスト環境の構築

`minOSv2` のディレクトリと同じ場所に `googletest-release-1.10.0` をダウンロードして配置します。
`googletest-release-1.10.0` のダウンロードは [こちら](https://github.com/google/googletest/archive/release-1.10.0.zip)

``` text
|- minOSv2/
|
|- googletest-release-1.10.0/

```

### テスト方法

`minOSv2` のルートディレクトリで `make test` です。
