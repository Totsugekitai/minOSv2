# VFS層

## object 一覧

- `superblock` object: スーパーブロック。
- `inode` object: ファイルのinode。開いていないのも含む。
- `dentry` object: ディレクトリエントリ。ファイルの名前。
- `file` object: プロセスが開いたファイル

## `file` object と `FILE` 構造体に関しての情報

どうやら `struct file` と `FILE` 構造体は違うっぽい？

`struct file` はプロセスがファイルを open() した割り当てられる。
ディスク上には対応するデータ構造は存在しない。

```c
 953: struct file {
...
 962:         struct path             f_path;
 963: #define f_dentry        f_path.dentry
 965:         const struct file_operations    *f_op;
...
 970:         atomic_long_t           f_count;
 972:         fmode_t                 f_mode;
 973:         loff_t                  f_pos;
...
 983:         void                    *private_data;
...
 993: };
```

- f_dentry: 対応する dentry オブジェクトへのポインタ。
- f_mode: open() システム・コールのモードを少し変形したもの。 FMODE_READ, FMODE_WRITE 等。
- f_pos: ファイルを読み書きする位置。シークポインタ。
- f_count: 参照カウンタ。ファイルを開くと 1 で、fork() すると増え、 close() すると減る。
- private_data: ファイルシステムに固有のデータを保存する。（継承の一実装方法。構造体を拡張する方法もある。）

### `task_struct` と `struct file` の関係

``` c
linux-3.1.3/include/linux/sched.h
1220: struct task_struct {
...
1386:         struct files_struct *files;
...
1572: };

linux 1.3/include/linux/fdtable.h
  44: struct files_struct {
...
  58:         struct file __rcu * fd_array[NR_OPEN_DEFAULT];
  59: };
```

`task_struct` が `files_struct` へのポインタを持っており、 `files_struct` がファイルディスクリプタを保持している。
`files_struct` の `fd` メンバのそれぞれが `struct file` になっている。

### `FILE` 構造体についての情報

`FILE` 構造体はだいたいこんな感じになっている。

```c
typedef struct {
char mode;
char *ptr;
int rcount;
int wcount;
char *base;
unsigned bufsiz;
int fd;
char smallbuf[1];
} FILE;
```

- `mode`: r,w などのファイルアクセスモード
- `ptr`: 読み書きしている位置（先頭からのバイト数）
- `base`: メモリ上のファイルバッファの先頭アドレス
- `rcount/wcount`: 多分参照カウント。これが0になったときファイルが開放される
- `bufsize`: ファイルバッファのサイズ
- `fd`: ファイルディスクリプタ番号

