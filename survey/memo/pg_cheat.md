# Paging Cheat-Sheet

まとめのまとめです。

## Flow of Initialization

1. ページング構造を初期化
2. CR3にページング構造の物理アドレスを追加
3. 以下の手順でビットを立てる
    1. CR4.PAE = 1
    2. IA32_EFER.LME = 1
    3. CR4.PG = 1

## Initialization of Paging-Structures

### types of Paging-Structures

4-level Pagingの中でもページサイズが2MBのモデルを用いる。
以下の3つのページング構造がある。

- PML4
- PDP
- PD

### Flow of translation

リニアアドレスから物理アドレスの流れとしては、

```
              Linear  Address
        ↓       ↓      ↓           ↓
CR3 -> PML4 -> PDP -> PD -> Physical Address
```

である。

### Initialization of Paging-Tables

以下は各テーブルの初期化パラメータである。
なお物理アドレス空間のサイズは4GB、リニアアドレス空間のサイズも4GBとする。

- 共通
    - size: 8byte(64bit) × 512エントリ = 4096byte = 4KB

- PML4 Entry Table
    - エントリ数は1個と決める(OSの設計次第)
    - エントリはPDPテーブルの先頭アドレスを格納

- PDP Entry Table
    - エントリ数は4個と決める(OSの設計次第)
    - エントリは各PDテーブルの先頭アドレスを格納

- Page Directory
    - エントリ数は512個
    - 各エントリに物理アドレスを2MBずつインクリメントしながら格納(ページサイズが2MBということ)

### Address width

- Physical Address: 52bitぶん確保されているが、だいたいは36bitまで
- Linear Address: だいたいは48bit

多分余りは0が入る。


