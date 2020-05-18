# UEFIブートローダを作ろう

この章では、自作OS向けのUEFIブートローダを作ります。

## まえおき

UEFIブートローダを作るために必要なことと、それについて必要な文書や技術をまずは整理します。

### UEFIブートローダを作るために必要なこと

UEFIブートローダは、EDK2を用いてUEFIアプリケーションとして作成します。

#### UEFIとは

Unified Extensible Firmware Interface の頭文字を取った略称です。
BIOSの後継として開発されており、PCの電源を入れるとまずUEFIが立ち上がります。

ブートローダは、UEFI上で動くアプリケーションとして開発します。

##### UEFIの仕様書

以下から入手します。この文書では、version 2.8に準拠することとします。

[UEFI Specification version 2.8](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf)

#### EDK2とは

EDK2とは、tianocoreという団体が主導で開発しているファームウェア開発環境です。
UEFIの仕様に準拠しています。

このツールキットを用いてブートローダを開発します。

##### EDK2のドキュメント

以下からドキュメント群を入手します。

(https://github.com/tianocore/tianocore.github.io/wiki/EDK-II-Documents)

### ブートローダで期待される動作

ブートローダは、以下の動作を期待されます。

1. 必要なパラメータの取得
2. カーネルのロード
3. UEFI環境からの脱出
4. カーネルへのジャンプ

これらを実現するようにプログラムを書いていきます。

## 開発する

それでは開発をしていきます。

### EDK2の設定をする

最初に開発環境を整えましょう。

#### 開発環境を整える

EDK2を手に入れましょう。
`git` を用いて公式リポジトリからクローンし、サブモジュールをアップデートします。

``` shell
$ cd workdir
$ git clone https://github.com/tianocore/edk2.git
$ cd edk2
$ git submodule update --init
```

次にアプリケーションに必要なツールをビルドします。

``` shell
$ make -C BaseTools/Source/C
```

次は環境変数の設定をします。
次のファイルを実行して変数を読み込みます

``` shell
$ source edksetup.sh
```

また、この時点で `Conf/target.txt` が生成されるので、これを以下のように編集します。

``` text
# -- snip --
ACTIVE_PLATFORM       = MinLoaderPkg/MinLoaderPkg.dsc
# -- snip --
TARGET_ARCH           = X64
# -- snip --
TOOL_CHAIN_TAG        = GCC5
```

`ACTIVE_PLATFORM` でビルドするターゲットを指定します。

`TARGET_ARCH` でビルドするアーキテクチャを指定します。
今回は64bitの自作OSに合わせたブートローダを作るので、 `X64` に指定します。

`TOOL_CHAIN_TAG` でビルドに使うツールチェインを指定します。

最後にディレクトリを作って整理しておきましょう。

``` shell
$ cd path/to/minOSv2
$ mkdir -p boot/MinLoaderPkg/Applications/MinLoader/boot
```

今後はこの中にファイルを追加していきます。

### 設定ファイルを記述する

次は設定ファイルを書いていきます。

必要な設定ファイルは `MinLoaderPkg.dec`, `MinLoaderPkg.dsc`, `MinLoader.inf` の3つです。

#### `.dec` ファイルを書く

##### `.dec` ファイルとは

`.dec` ファイルはパッケージ宣言ファイルとよばれるファイルです。

TODO: もう少し説明する

#### `MinLoaderPkg.dec` を書く

以下のように記述します。

``` text
[Defines]
DEC_SPECIFICATION       = 0x00010005
PACKAGE_NAME            = MinLoaderPkg
PACKAGE_GUID            = dc89db21-066d-4e66-882b-a13bc4915236
PACKAGE_VERSION         = 0.1
```

- `DEC_SPECIFICATION` : `.dec` ファイルの仕様のバージョンです。
- `PACKAGE_NAME` : パッケージの名前です。
- `PACKAGE_GUID` : パッケージのGUIDです。 `uuidgen` コマンドで生成した値を入れます。
- `PACKAGE_VERSION` : パッケージのバージョンです。
