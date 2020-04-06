# Segmentation

セグメンテーションのまとめです。

## LGDT instruction

### 用法

```
lgdt    [reg]    # regから6byte読み込む
```

### regの内訳

- 下位2byte: GDTのlimit(範囲)を表す
- 上位4byte: GDTのbase address(開始アドレス)を表す
