# minOSのブートプロセス

## 起動からブートローダ実行までの流れ

minOSはUEFIから起動します。

PCが起動するとUEFIが立ち上がり、UEFIはFAT32ファイルシステムを探し、
FAT32内部の `EFI/BOOT` ディレクトリにある `BOOTX64.EFI` をデフォルトで実行します。
したがって `BOOTX64.EFI` という名前でブートローダを作ればよさそうです。

## ブートローダで行う処理

minOSのブートローダで行うべき処理は以下の通りです。

- OSで必要なデバイスパラメータの取得
- OSをメモリ上に展開
- OSにジャンプ

## ブートローダの作成

### 用いるツールキット

minOSのブートローダは **edk2** を用いて作成します。
edk2はUEFIアプリケーションを作成するためのSDKです。

### main関数の作成

これからedk2を用いてブートローダを作成していきます。

まずは普通のプログラムの `main` 関数に相当する `Uefi_Main` 関数を作成します。
`boot/MinLoader.c` を見てみましょう。

```c
EFI_STATUS
EFIAPI
Uefi_Main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *st)
{
    ...
}
```

`Uefi_Main` という関数から処理をスタートさせます。
引数が2つありますが、1つ目に `EFI_HANDLE` 型の引数を、
2つ目に `EFI_SYSTEM_TABLE *` 型の引数を指定してあげることで必要な情報を受け取ることができます。

ところでこの `EFI_STATUS` やその他の自明ではない型はどこで定義されているのでしょうか。

*EDK II Module Writer's Guide* を読んでみると、 *2.1.1 EDK II Packages* にて

> The MdePkg contains the complete definitions in EFI1.1, UEFI2.0, UEFI2.1, PI1.0 Specifications and all library classes and instances defined in EDK II MDE (Module Development Environment) Library Specification. UEFI and PI drivers can be developed based solely on this package.

と記述されています。
この `MdePkg` ディレクトリ内を探してみると、どうやら `Include/Uefi/UefiBaseType.h` と `Include/Uefi/UefiSpec.h` を読み込めばよさそうです。
この二つのヘッダファイルは `Include/Uefi.h` でインクルードされているのでこれを自分のヘッダファイルである `boot/MinLoader.h` にインクルードしましょう。
ちなみにインクルードパスですが、どうやらedk2が `MdePkg/Include/` を追加してくれているようなので、以下のように書けます。

```c
#include <Uefi.h>
```

この定義は仕様書を探しても見つけられなかったので、 [tianocore/edk2 のリポジトリ](https://github.com/tianocore/edk2) を検索して見つけました。
情報が見つからず困ったときは、リポジトリ内検索をかけると見つかるかもしれません。

ちなみに `Uefi_Main` の書き方は *EDK II Module Writer's Guide* の *4.2 Write UEFI Application Entry Point* に載っていまず。

### デバイスパラメータの取得

minOSではOSに渡すデバイスのパラメータとして、

- フレームバッファの情報

があります。まずはこれを取得します。

以下は断りがない限り `Uefi_Main` 内に記述するものとします。

```c
struct bootinfo_t bootinfo;
EFI_STATUS status;

EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
do {
    status = gBS->LocateProtocol(&gop_guid, NULL, (void **)&gop);
} while (EFI_ERROR(status));
```

`struct bootinfo_t bootinfo` と `EFI_STATUS status` を宣言します。

`bootinfo` はOSに渡したい情報を定義した独自の構造体です。
`boot/MinLoader.h` で以下のように定義しています。

```c
struct video_info_t {
    unsigned long *fb;
    unsigned long fb_size;
    unsigned int x_axis;
    unsigned int y_axis;
    unsigned int ppsl;
};

struct bootinfo_t {
    struct video_info_t vinfo;
};
```

`status` はエラーチェックに用います。この変数の型 `EFI_STATUS` は `Uefi.h` をインクルードすれば使えます。
`EFI_GUID` も `Uefi.h` をインクルードすれば使えます。

`do while` 内部で
```c
status = gBS->LocateProtocol(&gop_guid, NULL, (void **)&gop);
```
なる処理を行っています。
変数 `gBS` は `EFI_BOOT_SERVICE *` 型のグローバル変数で、この変数の中に様々なライブラリが定義されています。

いきなり出現したグローバル変数ですが、これは *EDK II Module Writer's Guide* の *Table 8* を読むと存在が確認できます。

確認すると、 `UefiBootServicesTableLib` で定義されているようなのでこれを読み込みたいです。
`MdePkg` 内を探してみると、 `Include/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.h` 内に `gBS` が確認できるのでこのヘッダファイルを読み込みましょう。

`gBS` から参照できる `LocateProtocol` という関数を用いてフレームバッファの情報を取得します。

`LocateProtocol` は以下のように用います。

```c
status = gBS->LocateProtocol({プロトコルGUID}, NULL, {プロトコルインターフェースを詰める変数});
```

いきなり出てきたプロトコルとは何ぞやと思うかもしれませんが、
*UEFI Specification* における *1.5 UEFI Design Overview* の *Boot services* に

> Device access is abstracted through “handles” and “protocols.”

と記述されています。プロトコルとはデバイスへのアクセスを抽象化したものととらえればよさそうです。
デバイスの機能を使いたいときにはプロトコルインターフェースからアクセスします。
上記のソースコードなら `gop` からアクセスします（この後すぐに行います）。

フレームバッファの情報は *GraphucsOutputProtocol* を用いればよさそうです（これは *UEFI Specification* を読みました）。
*HogeProtocol* を使うためには `MdePkg` の `Include/Protocol` 内を探せばよいです。
`Protocol/GraphicsOutput` を `MinLoader.h` にインクルードしておきましょう。

```c
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
```

無事フレームバッファの情報が取れたら `bootinfo` に情報を詰めます。

```c
bootinfo.vinfo.fb = (uint64_t *)gop->Mode->FrameBufferBase;
bootinfo.vinfo.fb_size = (uint64_t)gop->Mode->FrameBufferSize;
bootinfo.vinfo.x_axis = (uint32_t)gop->Mode->Info->HorizontalResolution;
bootinfo.vinfo.y_axis = (uint32_t)gop->Mode->Info->VerticalResolution;
bootinfo.vinfo.ppsl = (uint32_t)gop->Mode->Info->PixelsPerScanLine;
```

`gop` からアクセスできるデバイスパラメータは *UEFI Specification* に載っているので各自調べて下さい。

### OSをメモリ上に展開

ほしいデバイスの情報は取得できたので、次はOSをメモリ上に展開します。

#### OSのファイルを開く

まずはOSのファイルを開きます。

```c
EFI_GUID sfsp_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;
do {
    status = gBS->LocateProtocol(&sfsp_guid, NULL, (void **)&sfsp);
} while(EFI_ERROR(status));
```

ファイルシステムには *SimpleFileSystemProtocol* を用いると簡単にアクセスできます。

ファイルシステムに関する関数を使用するために `LocateProtocol` を用いて `sfsp` に情報を詰めます。
*GraphicsOutputProtocol* と同じ要領で、 `MdePkg` の `Include/Protocol/SimpleFileSystem.h` を `boot/MinLoader.h` にインクルードします。

```c
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
```

```c
EFI_FILE_PROTOCOL *root = NULL;
do {
    sfsp->OpenVolume(sfsp, &root);
} while(EFI_ERROR(status));
```

*SimpleFileSystemProtocol* の `OpenVolume` 関数を用いてファイルシステムのrootディレクトリの情報を取得します。
ここでは `root` 変数にrootディレクトリの情報を詰めています。

以下からはOSのバイナリファイルがファイルシステムのrootディレクトリにあると仮定します。

```c
EFI_FILE_PROTOCOL *kernel_file = NULL;
CHAR16 *file_name;
UINT64 fr_num = (UINT64)EFI_FILE_READ_ONLY;
file_name = (CHAR16 *)KERNEL_FILE_NAME;
do {
    status = root->Open(root, &kernel_file,
            file_name, fr_num, 0);
} while(EFI_ERROR(status));
```

`file_name` 変数にはファイルの名前を代入します。
`EFI_FILE_PROTOCOL` は `SimpleFileSystem.h` で定義されています。

`Open` 関数でOSのファイルを開きます。使い方は以下の通りです。

```c
status = root->Open({ディレクトリ情報}, {ファイル情報を詰める変数}, {ファイルの名前}, {モード}, {オプション});
```

```c
EFI_FILE_INFO *file_info = NULL;
EFI_GUID fi_guid = EFI_FILE_INFO_ID;
UINTN buf_size = (UINTN)BUF_256B;
do {
    status = kernel_file->GetInfo(kernel_file,
            &fi_guid, &buf_size, file_info);
} while(EFI_ERROR(status));
UINTN file_size = file_info->FileSize;
```

OSファイルの情報を取得します。

`EFI_FILE_INFO` という型が登場したので探してみると、 `MdePkg/Include/Guid/FileInfo.h` で定義されているようです。
`EFI_FILE_INFO_ID` も同じファイルで定義されています。
`boot/MinLoader.h` にインクルードしておきましょう。

```c
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
```

`GetInfo` 関数で `file_info` 変数にファイルの情報を詰めます。
そして `file_info` 変数からファイルサイズを取り出します。

```c
uint64_t *kernel_program = NULL;
uint64_t *start_address =
    (uint64_t *)KERNEL_START_QEMU;
do {
    status = kernel_file->Read(kernel_file,
            &file_size, kernel_program);
} while(EFI_ERROR(status));

gBS->CopyMem(start_address, kernel_program, file_size);
```

ファイルを読み込みます。

`Read` 関数で `kernel_program` にファイルを読み込んだのち、 `CopyMem` 関数で `start_address` アドレスに `file_size` サイズ分だけコピーします。
`CopyMem` 関数はいくつかありますが、今回は `EFI_BOOT_SERVICES` のものを使うことにしました。

### OSにジャンプ

OSをメモリに展開できたのでOSにジャンプします。
ここでUEFIの機能は使わなくなるので、UEFI環境から抜ける処理をします。

UEFIから抜ける処理には `ExitBootServices` という関数を用いますが、これには現在のメモリマップを渡してあげる必要があります。
したがって処理は以下のようになります。

```c
UINTN mmapsize = 0, mapkey, descsize;
UINT32 descver;
EFI_MEMORY_DESCRIPTOR *mmap = NULL;
do {
    status = gBS->GetMemoryMap(&mmapsize, mmap, &mapkey,
                                    &descsize, &descver);
    while (status == EFI_BUFFER_TOO_SMALL) {
        if (mmap) {
            gBS->FreePool(mmap);
        }
        mmapsize += 0x1000;
        status = gBS->AllocatePool(EfiLoaderData, mmapsize, (void **)&mmap);
        status = gBS->GetMemoryMap(&mmapsize, mmap, &mapkey,
                                    &descsize, &descver);
    }
    status = gBS->ExitBootServices(ImageHandle, mapkey);
} while (EFI_ERROR(status));
```

`do while` 内部ではまず `GetMemoryMap` 関数を用いて `mmap` にメモリマップを詰めています。
`EFI_MEMORY_DESCRIPTOR` 型は `UefiSpec.h` で定義されているので追加でインクルードするファイルはありません。
この時、 `mmapsize` が小さいと処理に失敗する可能性があります。
小さかった場合は一旦 `mmap` のメモリ領域を解放して、 `mmapsize` を増やしてから領域を確保しなおします(`AllocatePool` 関数の部分です)。
そして再度 `GetMemoryMap` を行います。

無事メモリマップを取得出来たら、 `ExitBootServices` を呼び出してUEFIを抜けます。

```c
kernel_jump(&bootinfo, start_address);
```

最後に `start_address` アドレスにジャンプします。
`kernel_jump` は `boot/kernel_jump.S` にアセンブラで書きますが、以下のようなコードになります。

```asm
.global kernel_jump

kernel_jump:
    jmp     *%rsi
```

このとき `kernel_jump` の第一引数は `rdi` 、第二引数は `rsi` にセットされます。
また `%rsi` についている `*` は絶対番地へジャンプしろという意味です。

## edk2でビルドする

コードが書けたのでedk2を用いてビルドします。

edk2のルートディレクトリで `LoaderPkg/` というディレクトリを作ります。
また `LoaderPkg/Applications/MinLoader/` ディレクトリも作ります。

つぎに `LoaderPkg.dec` 、 `LoaderPkg.dsc` 、 `MinLoader.inf` の3ファイルを以下の場所に作成します。
また、これまで書いたソースコードも配置します。

```
LoaderPkg/
|
|-LoaderPkg.dec
|
|-LoaderPkg.dsc
|
+-Applications/
  |
  +-MinLoader/
    |
    |-MinLoader.inf
    |
    +-boot/
      |
      |-MinLoader.c
      |
      |-MinLoader.h
      |
      +-kernel_jump.S
```

### `.dec` ファイルの書き方

パッケージ宣言ファイルです。
以下のように書きます。

```
[Defines]
DEC_SPECIFICATION       = 0x00010005
PACKAGE_NAME            = LoaderPkg
PACKAGE_GUID            = dc89db21-066d-4e66-882b-a13bc4915236
PACKAGE_VERSION         = 0.1
```

`DEC_SPECIFICATION` はよくわかりませんが、edk2ディレクトリのそのほかのパッケージの該当部分と同じ番号にしておけば問題ないでしょう。
`PACKAGE_NAME` はパッケージの名前です。
`PACKAGE_GUID` は `uuidgen` コマンドで生成した値を入れれば良いらしいです。
`PACKAGE_VERSION` はパッケージのバージョンです。

### `.dsc` ファイルの書き方

以下のように書きます。

```
[Defines]
  PLATFORM_NAME           = LoaderPkg
  PLATFORM_GUID           = dc89db21-066d-4e66-882b-a13bc4915236
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/LoaderPkg$(ARCH)
  SUPPORTED_ARCHITECTURES = IA32|X64
  BUILD_TARGETS           = DEBUG|RELEASE|NOOPT

  DEFINE DEBUG_ENABLE_OUTPUT     = FALSE

[LibraryClasses]
  # Entry point
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

  # Common Libraries
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  !if $(DEBUG_ENABLE_OUTPUT)
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  !else   ## DEBUG_ENABLE_OUTPUT
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif  ## DEBUG_ENABLE_OUTPUT

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf

[Components]
  LoaderPkg/Applications/MinLoader/MinLoader.inf
```

`[Defines]` から見ていきます。

`PLATFORM_GUID` は `uuidgen` で生成した値を入れます。
`OUTPUT_DIRECTORY` で指定したディレクトリにビルド結果が吐き出されます。
`SUPPORTED_ARCHITECTURES` でアーキテクチャを指定します。

次に `[LibraryClasses]` を見ます。

ここには使ったライブラリを書いていきます。

```
LibraryName|path/to/files.inf
```

と指定することで、 `ffiles.inf` のライブラリを `LibraryName` という名前で用いることができます。

最後に `[Components]` です。

ここに `.inf` ファイルを書くと、対応するパッケージがビルドされます。
今回は `MinLoader.inf` を書きましょう。

### `.inf` ファイルの書き方

以下のように書きます。

```
[Defines]
INF_VERSION             = 0x00010005
BASE_NAME               = MinLoader
FILE_GUID               = dc89db21-066d-4e66-882b-a13bc4915236
MODULE_TYPE             = UEFI_APPLICATION
VERSION_STRING          = 0.1
ENTRY_POINT             = Uefi_Main

[Sources]
boot/MinLoader.h
boot/MinLoader.c
boot/kernel_jump.S

[Packages]
MdePkg/MdePkg.dec

[LibraryClasses]
UefiLib
UefiApplicationEntryPoint
```

`[Defines]` から見ていきます。
`FILE_GUID` は `uuidgen` で生成しましょう。
`MODULE_TYPE` は `UEFI_APPLICATION` を指定しましょう。
`ENTRY_POINT` は `Uefi_Main` を指定します。これでプログラムのエントリポイントが指定されます。

`[Sources]` にはソースコードを指定します。

`[Packages]` にはこのモジュールが依存するパッケージを列挙します。

`[LibraryClasses]` にはこのモジュールが依存するライブラリを列挙します。

### target.txt

最後に `edk2/Conf/target.txt` を設定します。

- `ACTIVE_PLATFORM` に `LoaderPkg/LoaderPkg.dsc`
- `TARGET` に `RELEASE`
- `TARGET_ARCH` に `X64`
- `TOOL_CHAIN_TAG` に `GCC5`

で良いでしょう。

### ビルド

```
source edksetup.sh
build
```

でビルドできます。

