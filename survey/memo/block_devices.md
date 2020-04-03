# ブロックデバイスについてのメモ

## ブロックデバイスへの接続方法

PCI に接続されているのでそこから読む。
PCI Configuration Spaceにアクセスして頑張る。

## PCI Configuration Space

I/O空間上のCONFIG_ADDRESS(0x0cf8)とCONFIG_DATA(0x0cfc)レジスタを用いてConfiguration Registerを読み書き。

詳細は[osdev-jpのwiki](https://github.com/osdev-jp/osdev-jp.github.io/wiki/PCI-Memo)に載っている。

[OSDev.orgのwiki](https://wiki.osdev.org/PCI)も読んで頑張る。

### 各種パラメータについて

- function number
    - 同じバススロットにつながれた個々のデバイスの論理番号
    - 各PCIカードは1つのスロットに格納されるが、PCIカードごとに独自のリソースを持つ論理デバイスを使用できる
        - この論理デバイスをfunctionと呼ぶ

## AHCIについて

[osdev-jpのwiki](https://github.com/osdev-jp/osdev-jp.github.io/wiki/AHCI-Memo)が詳しい。

- AHCIデバイスはベースクラス0x01, サブクラス0x06のPCIデバイスである。
- AHCIのConfiguration SpaceにあるABARをもとに設定が行われる。
    - ABARを設定してHBA Memory Registerを読み取るとよい
        - HBA Memory RegisterのGeneric Host Controlを設定する
            - Generic Host ControlのGlobal Host Controlを設定する
                - AE(31)でAHCIのenableを決める。1が安定。
                - IE(1)で割込みの有効化。1?
        - HBA Memory RegisterのPort x port control registerを設定する
            - PxCLBでCommandListを置く場所を決める。1Kバイトアラインなのでビット9:0は0固定。
            - PxFBでreservedFISを置く場所を決める。256バイトアラインなのでビット7:0は0固定。
            - PxISは割込み状態を表すフラグなので基本読み取り？
            - PxSACTはネイティブコマンドキューイング対象のコマンドの実行完了を知るためのレジスタ
                - PxCI[TAG]に1を書き込む前にソフトウェアはタグ番号TAGを持つコマンドが未処理であることを示すためにPxSACT[TAG]に1を書きこむ
            - PxCIはCommandListの中でデバイスに送信するコマンドスロットを選択するレジスタ。
                - ソフトウェアはコマンドスロットの送信準備が完了したことを示すために1を書き込む

### Command List

SATAディスクは複数のコマンドを一度に送るので、コマンドをCommandListに格納する

CommandListは要素数32のCommandHeaderの配列で、CommandHeaderは一つのSATAコマンドに対応

CTBA0レジスタでCommandTable構造を指し示し、CommandTable内に実際にSATAコマンドを構築する。
CommandTable内のCFISがコマンドを構築するメモリ領域である。

SATAコマンドをデバイスに送る手順は以下の通り。

1. CommandListの空いているスロットに好きなだけコマンドを構築する
2. PxCIの対応ビットに1を書き込む
3. HBAが自動的にコマンドをデバイスに送り、応答を待つ

### Command Header

1つのCommandHeaderは1つのCommandTableを示す。
オフセット00hでレジスタ群の設定を行う。

- PRDTL(31:16)はPRDTの要素数を設定するレジスタ。
- C(10)は1に設定すると、FISの送信が終わりR_OKを受信した後にHBAがPxTFD.STS.BSYビットと、そのコマンドスロットに対応するPxCIのビットをクリアする
- W(6)はデータ送受信の方向を設定する。
    - 1はメモリ->デバイス
    - 0はデバイス->メモリ
- CFL(4:0)はコマンドFISの大きさをDW単位で設定するレジスタ。
    - 0 or 1を設定してはいけない
    - 最大値は16

### Command Table

デバイスへ送る1つのSATAコマンドと送受信するデータを置く場所を指定するPRDTからなる。
SATAコマンドの戻り値はPxFBで指示されたReceivedFISに書き込まれる

#### CFIS

1つのFISを置く領域

#### PRDT

送受信するデータを格納するメモリ領域を指定する。
0から65535までの要素を持つ配列で、1つの要素は4つのDWORDのレジスタからなる。

- DBA(00h)はデータブロックの先頭を指す32ビットの物理アドレス。
    - 2バイトアライン
- DBAU(04h)はDBAの上位32ビット。
    - CAP.S64Aが1の時のみ有効
- I(0ch:bit31)はセットするとこのエントリのデータ送受信が終わったときに割り込みを発生させる。
- DBC(0ch:bit21:0)はデータブロックのバイト数

#### Received FIS

デバイスから送られてきたFISを格納する場所。
PxFBから指し示される。
1ポートにつき1つのreceived FISが存在する。

## System Software Rules(仕様書より)

やるべきことを順番通りに記す。

### Building a Command

1. portに対してPxCIとPxSACTを読み、空きのコマンドスロット(pFreeSlot)を見つける
    - PxCIとPxSACTは各ビットが各コマンドスロットに対応しており、PxCIとPxSACTのビットにおいて両方とも0であれば空きスロットである
    - 例えばコマンドスロット5が空きかどうか調べるには、PxCIのbit5とPxSACTのbit5を調べ、両方0だったら空きである
2. メモリ上のPxCLB[CH(pFreeSlot)]:CFISの位置にcommand FISを作成
3. PxCLB[CH(pFreeSlot)]に作成するcommand headerは以下を満たす
    - PRDTLはPRD tableのエントリの数を含む
    - CFLはCFIS areaのcommandに長さをセットする
    - データをデバイスに送る場合、W(Write)bitをセットする
    - (P(Prefetch)bitをセットする)(5.2.2を見る限りセットしないほうがよさそう)
    - Port Multiplierがついているなら、PMPフィールドを正しいPort Multiplier portにセットする
4. キューコマンドならば、まずPxSACT.DS(pFreeSlot)をセットする。つまり、今設定しようとしているコマンドスロットに対応するビットを立てればよい
5. PxCI.CI(pFreeSlot)をセットする。つまり、今設定しようとしているコマンドスロットに対応するビットを立てればよい

### Processing Completed Commands

コマンドが完了した際のデバイスからの割り込みを処理しなくてはならない。
割込みサービスルーチンにおいて、IS.IPSをチェックして保留している割込みがあるか確認する必要がある。

割込み保留がある各portにおいて、以下の処理をする。

1. PxISを読んで、割込み発生源を特定
2. PxISの適切なbit群(どれ？)をクリアする
3. IS.IPSの対応するportの割込みビットをクリアする
4. キューコマンドでないコマンドを実行するなら、PxCIを読んで、今の値と以前発行されてまだ解決されていないコマンドを比較する。
キューコマンドなら、PxSACTを読んで、今の値と以前発行されたコマンドを比較する。
各レジスタの対応するビットがクリアされるのをもって、ソフトウェアは未処理のコマンドを完了とみなす。
PxCIとPxSACTはvolatileなレジスタなので、ソフトウェアはこれらのレジスタの値を完了したコマンドを特定することのみに使うべきで、どのコマンドが過去に発行されたかを調べることに使うことではない。
5. エラーがあったら、PxISに伝えられ、ソフトウェアはエラーリカバリのための行動をする

## Transfer Example

### ATA DMA Write

1. CH(pFreeSlot).W = 1 にする
2. デバイスにデータを送る際、HBAはExam:FetchとExam:Transmitの状態を横断する
    - CH(pIssueSlot).P = 1の時、Exam:TransmitマクロのCFIS:Success状態の後かつ、Example:FetchマクロのP:Idle状態に戻る前に、次のstateを実行する
    - CFIS:PrefetchPRD -> CFIS:PrefetchData -> P:Idle
3. これがDMA Write Commandだとして、デバイスからの返答はDMA Activate FISである。
    - これが到着したら、HBAはこのFISをExam:AcceptNonDataマクロのstateを横断して受理し、またData FISをExam:DMATransmitマクロのstateを横断することでデバイスに送る。
4. Data FISがtransfer countを満足しなかった場合、デバイスから別のDMA Activate FISが送られ、HBAは別のData FISを送ろうとする。
このプロセスはtransfer countが満足されるまで行われる。
Data FISがすべて送られたら、次にデバイスから送られてくるFISはD2H Register FISである。
5. HBAはExam:AcceptNonDataマクロのstateを横断してこのFISを受理する。
D2H Register FISのl(小文字のL)ビットが1なら、HBAはExam:D2HIntrマクロのstateを横断する。
ビットが0なら、Exam:D2HNoIntrマクロのstateを横断する。

### ATA DMA Read

1. CH(pFreeSlot).W = 0 にする
2. デバイスにデータを送る際、HBAはExam:FetchとExam:Transmitの状態を横断する
    - CH(pIssueSlot).P = 1の時、Exam:TransmitマクロのCFIS:Success状態の後かつ、Example:FetchマクロのP:Idle状態に戻る前に、次のstateを実行する
    - CFIS:PrefetchPRD -> P:Idle
3. これがDMA write commandの場合は、デバイスからのレスポンスはDMA Activate FISである
    - Exam:AcceptNonDataマクロ状態になりFISを受け入れ、Exam:DMATransmitマクロ状態でData FISを送る
4. Data FISがtransfer countを満たさなかった場合、デバイスは別のDMA Activate FISを送る
    - その際Exam:AcceptNonDataとExam:DMATransmitマクロ状態を通過する
    - transfer countが満たされるまで続けられる
    - Data FISのtransfer countが終わったら、次にデバイスから送られてくるFISはD2H Register FISである