# minOSv2

![build](https://github.com/Totsugekitai/minOSv2/workflows/build/badge.svg)
![test](https://github.com/Totsugekitai/minOSv2/workflows/test/badge.svg)

x86_64 アーキテクチャで UEFI 対応のシンプルな OS です。

[minOS](https://github.com/Totsugekitai/minOS) を書き直して作られています。

`master` ブランチは GPD MicroPC にて実機動作確認済みです。

`develop` ブランチは QEMU にて動作確認済みです。

## ビルド方法

### カーネルのビルド

ルートディレクトリで `make kernel` です。

### ブートローダのビルド

#### EDK2をクローンする

以下のようなディレクトリ構造となるようにEDK2をクローンする。
```
.
├── edk2
└── minOSv2
```
git clone

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

<!--
#### `target.txt` を編集する

`Conf/target.txt` が生成されていると思うので、それを以下のように編集する。

```
# -- snip --
ACTIVE_PLATFORM       = MinLoaderPkg/MinLoaderPkg.dsc
# -- snip --
TARGET_ARCH           = X64
# -- snip --
TOOL_CHAIN_TAG        = GCC5
``` -->

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

`minOSv2` のルートディレクトリで、 `git submodule update --init` を打ちます。
すると `tests` ディレクトリに `googletest` がクローンされます。

配置したら、次のコマンドを入力します。

``` shell
$ cd path/to/minOSv2/tests
$ make gtest-gen
```

これで `tests` ディレクトリ内に `gtest` ディレクトリが生成されます。
これはテストに用いられます。

### テスト方法

`minOSv2` のルートディレクトリで `make test` です。
