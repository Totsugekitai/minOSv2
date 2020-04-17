# ファイルシステムメモ

## ファイルシステムの構造

基本的には木構造？

大きく分けてファイルとディレクトリの2種類がある。

## メタデータ

データ本体以外にも、メタデータと呼ばれる情報を付属させる。

- 名前
- ID
- 種類
- ストレージデバイス上の位置
- サイズ
- 権限情報
- 時刻情報

## インターフェース

- ファイルの作成： `creat()`
- ファイルの削除： `delete()`
- ファイルを探してメモリにムーブさせる： `open()`
- メモリ上のファイルコンテンツをディスクに移動させる： `close()`
- 開いたファイルからデータを読み出す： `read()`
- 開いたファイルにデータを書き込む： `write()`
- 開いたファイルの所定の位置に移動： `lseek()`
- ファイルシステム依存の特殊な処理： `ioctl()`
- ファイルの長さを変更する： `truncate()`

## inode

ディスクブロックとファイルの対応付けをするデータ構造。

以下のようなデータ構造である。

```
-------------------
|    file info    |
-------------------
| block 0 address |
-------------------
| block 1 address |
-------------------
| block 2 address |
-------------------
| block 3 address |
-------------------
| block 4 address |
-------------------
```
## ファイルシステムの実装

### ファイルシステムの階層構造

- application program
- logical filee system
- file-organization module
- basic file system
- I/O control
- devices

### デバイスドライバ

I/O control layerでI/O deviceを管理する

### basic file system

デバイスドライバに情報を渡す。

他にもメモリバッファやキャッシュの管理を行う。

### file-organization module

ファイルや論理アドレス、物理ブロックの解釈を行う。

- logical block # -> physical block # のような変換
- ディスク領域の開放、確保

### logical file system

メタデータを扱う

- ファイル名をファイル番号・ファイルハンドル・位置に変換
    - file control block(UNIXではinode)で管理する
- ディレクトリの管理
- 権限・ファイル保護

## `ext2` ファイルシステムのメモ

### `blocks`

いくつかのセクタをひとまとめにしたもの。
blockのサイズは1KiB,2KiB,4KiB,8KiBのいずれかで実装されている。

### `block groups`

複数のblockをまとめたもの。
block groupsの情報はblock group descriptorにまとめられる。
descriptorはsuperblockの直後に置かれる。

block groupの最初の2blockはblock usage bitmapとinode usage bitmapに用いられる。
ビットマップはblockとinodeの使用状況を記録する。
各ビットマップは1blockなので、block groupの最大サイズはblockのサイズの8倍。

### `directories`

directoryはファイルシステムオブジェクトで、inodeを持つ。
特別なファイルであり、各名前をinodeの番号に関連付けるレコードを保持している。

### `inodes`

inodeデータ構造はオブジェクトのデータとメタデータを含むファイルシステムブロックへのポインタを保持する、
inodeデータ構造はinodeデータ構造へのポインタを保持することもある。
最初の(どこの？)12ブロックへのポインタがあり、その12ブロックはファイルのデータを含んでいるらしい。
(図に起こしたほうが良さそう)

### `superblocks`

ファイルシステムの設定すべてを含むブロック。
デバイスの先頭から1024byteの位置に置かれる。

### `symbolic link`
シンボリックリンクは単純にtext stringを保持しており、OSによって解釈され処理される。

### block group descriptor table
ボリュームがどのようにblock groupに分割されるか、各block groupのどこにbitmapやinode tableがあるかを保管する。

### Locating an Inode
inode number は inode table のインデックスである。
inode table のサイズはフォーマット時に固定される。
大きい inode table が作成された場合には、すべての block group に均等に分割される。

#### パラメータの計算
s_inodes_per_group を用いて block group ごとにいくつの inode が定義されているか確認できる。
inode 1 は first inode であることがわかっているので、次のように計算する。

```
block group = (inode - 1) / s_inodes_per_group
```

block が特定できれば、 local inode index は次のように計算できる。

```
local inode index = (inode - 1) % s_inodes_per_group
```

## ジャーナリング

1. ファイルシステム内にジャーナル領域を用意する
2. アトミックな処理をジャーナルログとしてジャーナル領域に書き出す
3. ジャーナルログを読み出してファイルシステムの処理を行う。

## syscall 内部実装

### open()

`linux/fs/open.c` を読む。

[ここで定義](https://github.com/torvalds/linux/blob/master/fs/open.c#L1168)されている。

`ksys_open()` は以下の通り。

[linux/include/linux/syscall.h](https://github.com/torvalds/linux/blob/39bed42de2e7d74686a2d5a45638d6a5d7e7d473/include/linux/syscalls.h#L1381)

```c
static inline long ksys_open(const char __user *filename, int flags,
			     umode_t mode)
{
	if (force_o_largefile())
		flags |= O_LARGEFILE;
	return do_sys_open(AT_FDCWD, filename, flags, mode);
}
```

`do_sys_open()` は以下の通り。

[linux/fs/open.c](https://github.com/torvalds/linux/blob/6cad420cc695867b4ca710bac21fde21a4102e4b/fs/open.c#L1161)

```c
long do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode)
{
	struct open_how how = build_open_how(flags, mode);
	return do_sys_openat2(dfd, filename, &how);
}
```

`build_open_how()` は以下の通り。

[linux/fs/open.c](https://github.com/torvalds/linux/blob/6cad420cc695867b4ca710bac21fde21a4102e4b/fs/open.c#L958)

```c
// フラグ設定処理だと思われる
inline struct open_how build_open_how(int flags, umode_t mode)
{
	struct open_how how = {
		.flags = flags & VALID_OPEN_FLAGS,
		.mode = mode & S_IALLUGO,
	};

	/* O_PATH beats everything else. */
	if (how.flags & O_PATH)
		how.flags &= O_PATH_FLAGS;
	/* Modes should only be set for create-like flags. */
	if (!WILL_CREATE(how.flags))
		how.mode = 0;
	return how;
}
```

`do_sys_openat2()` は以下の通り。

[linux/fs/open.c](https://github.com/torvalds/linux/blob/6cad420cc695867b4ca710bac21fde21a4102e4b/fs/open.c#L1132)

```c
static long do_sys_openat2(int dfd, const char __user *filename,
			   struct open_how *how)
{
	struct open_flags op;
	int fd = build_open_flags(how, &op);    // フラグの設定
                                            // ここでのfdとはエラーのことのようだ？
	struct filename *tmp;

	if (fd)
		return fd;                          // エラーがあったらリターン

	tmp = getname(filename);                //
	if (IS_ERR(tmp))
		return PTR_ERR(tmp);

	fd = get_unused_fd_flags(how->flags);
	if (fd >= 0) {
		struct file *f = do_filp_open(dfd, tmp, &op);
		if (IS_ERR(f)) {
			put_unused_fd(fd);
			fd = PTR_ERR(f);
		} else {
			fsnotify_open(f);
			fd_install(fd, f);
		}
	}
	putname(tmp);
	return fd;
}
```

`build_open_flags()` は以下の通り。

[linux/fs/open.c](https://github.com/torvalds/linux/blob/master/fs/open.c#L974)

```c
// struct open_flags *op を設定して返すのが目的の関数
// opはopenするときのフラグ
inline int build_open_flags(const struct open_how *how, struct open_flags *op)
{
	int flags = how->flags;
	int lookup_flags = 0;
	int acc_mode = ACC_MODE(flags);

	/* Must never be set by userspace */
	flags &= ~(FMODE_NONOTIFY | O_CLOEXEC);

	/*
	 * Older syscalls implicitly clear all of the invalid flags or argument
	 * values before calling build_open_flags(), but openat2(2) checks all
	 * of its arguments.
	 */
    // 値をしっかりチェックしてエラーを返す
	if (flags & ~VALID_OPEN_FLAGS)
		return -EINVAL;
	if (how->resolve & ~VALID_RESOLVE_FLAGS)
		return -EINVAL;

	/* Deal with the mode. */
	if (WILL_CREATE(flags)) {
		if (how->mode & ~S_IALLUGO)
			return -EINVAL;
		op->mode = how->mode | S_IFREG;
	} else {
		if (how->mode != 0)
			return -EINVAL;
		op->mode = 0;
	}

	/*
	 * In order to ensure programs get explicit errors when trying to use
	 * O_TMPFILE on old kernels, O_TMPFILE is implemented such that it
	 * looks like (O_DIRECTORY|O_RDWR & ~O_CREAT) to old kernels. But we
	 * have to require userspace to explicitly set it.
	 */
	if (flags & __O_TMPFILE) {
		if ((flags & O_TMPFILE_MASK) != O_TMPFILE)
			return -EINVAL;
		if (!(acc_mode & MAY_WRITE))
			return -EINVAL;
	}
	if (flags & O_PATH) {
		/* O_PATH only permits certain other flags to be set. */
		if (flags & ~O_PATH_FLAGS)
			return -EINVAL;
		acc_mode = 0;
	}

	/*
	 * O_SYNC is implemented as __O_SYNC|O_DSYNC.  As many places only
	 * check for O_DSYNC if the need any syncing at all we enforce it's
	 * always set instead of having to deal with possibly weird behaviour
	 * for malicious applications setting only __O_SYNC.
	 */
	if (flags & __O_SYNC)
		flags |= O_DSYNC;

	op->open_flag = flags;

	/* O_TRUNC implies we need access checks for write permissions */
	if (flags & O_TRUNC)
		acc_mode |= MAY_WRITE;

	/* Allow the LSM permission hook to distinguish append
	   access from general write access. */
	if (flags & O_APPEND)
		acc_mode |= MAY_APPEND;

	op->acc_mode = acc_mode;

	op->intent = flags & O_PATH ? 0 : LOOKUP_OPEN;

	if (flags & O_CREAT) {
		op->intent |= LOOKUP_CREATE;
		if (flags & O_EXCL) {
			op->intent |= LOOKUP_EXCL;
			flags |= O_NOFOLLOW;
		}
	}

	if (flags & O_DIRECTORY)
		lookup_flags |= LOOKUP_DIRECTORY;
	if (!(flags & O_NOFOLLOW))
		lookup_flags |= LOOKUP_FOLLOW;

	if (how->resolve & RESOLVE_NO_XDEV)
		lookup_flags |= LOOKUP_NO_XDEV;
	if (how->resolve & RESOLVE_NO_MAGICLINKS)
		lookup_flags |= LOOKUP_NO_MAGICLINKS;
	if (how->resolve & RESOLVE_NO_SYMLINKS)
		lookup_flags |= LOOKUP_NO_SYMLINKS;
	if (how->resolve & RESOLVE_BENEATH)
		lookup_flags |= LOOKUP_BENEATH;
	if (how->resolve & RESOLVE_IN_ROOT)
		lookup_flags |= LOOKUP_IN_ROOT;

	op->lookup_flags = lookup_flags;
	return 0;
}
```
