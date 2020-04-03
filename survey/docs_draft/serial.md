# シリアル通信について

## 用語解説

- ### シリアル通信

シリアル通信とは、コンピュータのバス上を一度に1bitずつ送る通信方法のこと。
PCとはシリアルポートを用いて通信する。

- ### UART

UARTとは、非同期方式のシリアル通信をパラレル通信に変換したり、その逆を行うための集積回路である。

- ### ボーレート

情報が通信チャネルで送信される速度のこと。

## シリアル通信の設定

以下の設定がシリアル通信には必要である。

- 通信速度

    いわゆるボーレートの設定。送信側と受信側の速度を一致させる必要がある。

- 文字データごとのビット数

    何ビットごとに文字データとして認識するかを設定する。一般には8bitである。

- パリティ

    誤り検出のためのビットの設定である。
    None: N, Odd: O, Even: E, Mark: M, Space: Sに設定できる。
    一般的にはNに設定する。

- 文字ごとのストップビット

    各文字の終わりに送信される信号のこと。
    この信号を見てハードウェアは文字を認識する。

## プログラムに用いる設定や数値

- ポートアドレス

以下に各COMポートのI/O Port Addressを記す。

    - COM1: 0x3f8
    - COM2: 0x2f8
    - COM3: 0x3e8
    - COM4: 0x2e8

- ポートオフセット

各ポートは8つのレジスタを持っており、以下のような働きをする。
オフセット+3にはDLABという設定ビットがあり、切り替えることによってオフセット+0と+1の動作を変えることができる。

    - I/O Port Offset : Setting of DLAB : Register mapped to this port の順で書く
    - +0 : 0 : データレジスタ。ここにデータを書き込んだりここからデータを読み取ったりする
    - +1 : 0 : 割り込み許可レジスタ。
    - +0 : 1 : ボーレートの設定に使う除数の下位バイト
    - +1 : 1 : ボーレートの設定に使う除数の上位バイト
    - +2 : - : 割り込み特定/FIFOコントロールレジスタ
    - +3 : - : ラインコントロールレジスタ。DLABもこの中に含まれる
    - +4 : - : モデムコントロールレジスタ。
    - +5 : - : ラインステータスレジスタ。
    - +6 : - : モデムステータスレジスタ。
    - +7 : - : スクラッチレジスタ。

### ボーレートの設定

DLAB = 1の時の+0, +1を見て115,200を割るとボーレートの計算ができる。

ここではI/Oアドレス空間にアクセスするためにin命令とout命令を用いるが、簡略のため以下のような疑似コードを用いる。

```
return = in(address) // returnは変数
out(address, output)
```

DLABは+3の7bit目にある(ここでは0オリジンとする)ので、DLAB = 1にするには

```
out(PORT + 3 ,0x80)
```

のようにすればよい。

ボーレートの設定は、例えば

```
out(PORT + 0, 0x03)
out(PORT + 1, 0x00)
```

のように設定すると、除数は0x0002となり、ボーレートは

```
115,200 / 3 = 38,400
```

になる。

##＃# 文字データごとのビット数の設定

[PORT + 3]のbit 0, bit 1を用いて設定する。

以下が設定表である。

| bit 1 | bit 0 | 文字長 |
|:-----:|:-----:|:------:|
|   0   |   0   |    5   |
|   0   |   1   |    6   |
|   1   |   0   |    7   |
|   1   |   1   |    8   |

### ストップビット

[PORT + 3]のbit 2を用いて設定する。

以下が設定表である。

| bit 2 | Stop bit |
|:-----:|:--------:|
|   0   |     1    |
|   1   |   1.5/2  |

### パリティ

[PORT + 3]のbit 3, bit 4, bit 5を用いて設定する。

| bit 5 | bit 4 | bit 3 | Parity |
|:-----:|:-----:|:-----:|:------:|
|   -   |   -   |   0   |  NONE  |
|   0   |   0   |   1   |   ODD  |
|   0   |   1   |   1   |  EVEN  |
|   1   |   0   |   1   |  MARK  |
|   1   |   1   |   1   |  SPACE |

### 割り込み許可レジスタ

[PORT + 1]のbit 0を操作する。0で割り込み禁止、1で割り込み許可。

### ラインステータスレジスタ

このレジスタを設定することによってデータがあるかどうかなどを調べることができる。

詳しくは [OSDev.orgのこの記事](https://wiki.osdev.org/Serial_Ports#Line_status_register) を見られたし。

一部だけ紹介すると、bit 0はData Ready(DR)と呼ばれるビットで、データが読み出せるかどうかを確認するためのビットである。

以下のようにして用いるとデータの読み出しが可能かどうか調べられる。

```
int can_data_read(void)
{
    return in(PORT + 5) & 0x01;
}
```

### 割り込み特定/FIFOコントロールレジスタ

割り込み特定レジスタとFIFOコントロールレジスタが一緒になっているのでややこしいが、次のように設定すればよさそう。

[このサイト](https://www.lammertbies.nl/comm/info/serial-uart.html) が詳しい。

- bit 0 = 1: No interrupt pending/Enable FIFO
- bit 1 = 1: Clear receive FIFO
- bit 2 = 1: Clear transmit FIFO
- bit 3 = 0: Select DMA mode 0
- bit 4 = 0: reserved.
- bit 5 = 0: reserved.
- bit 6 = 1, bit 7 = 1: Receive FIFO interrupt trigger level 14 bytes

### モデムコントロールレジスタ

IRQの設定がbit 3でできる(らしい)。
とりあえず0x0bを入れておく。

## 実際の設定例

初期化は次のようにやればよい。

1. 割り込みを禁止する
2. DLAB = 1にする
3. ボーレートの除数を設定する
4. 文字ビット数、パリティ、ストップビットを設定する
5. 割り込み特定/FIFOコントロールレジスタの設定
6. モデムコントロールレジスタの設定

つまり、以下のようにすればよい。

```
#define PORT 0x3f8   /* COM1 */
 
void init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}
```

受け取るときはこうで、

```
int serial_received() {
   return inb(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inb(PORT);
}
```

送るときはこうである。

```
int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void write_serial(char a) {
   while (is_transmit_empty() == 0);
 
   outb(PORT,a);
}
```
