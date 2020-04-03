・MSIの有効化
MSI Capability Registerの0x050の16bit目を1にする。

・PCI Configuration Spaceへのアクセス方法
1. デバイスのBus Number・Device Number・Function Numberとアクセスしたい領域の
    オフセット値をEnableBitとともにCONFIG_ADDRESSレジスタにセットする。
    PCではCONFIG_ADDRESSレジスタはI/O空間の0xCF8にマップされている。
2. CONFIG_DATAレジスタに対して読み込みまたは書き込みを行う。
    PCではCONFIG_DATAレジスタはI/O空間の0xCFCにマップされている。

