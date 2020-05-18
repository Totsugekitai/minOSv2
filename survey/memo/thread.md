# スレッドについて

## スレッドについて調べたこと

スレッドはシステムコールか割り込みが呼ばれたときに行われる。

- 次に動作すべきスレッドの選択：スケジューリング
- 選択されたスレッドの再開：ディスパッチ

スタックはスレッドごとに持っている必要がある。

ディスパッチの時にスタックポインタを再開するスレッドのスタックのものに置き換える。

処理再開にCPUが必要とする情報をコンテキストと呼ぶ。

コンテキストは言ってしまえば各種レジスタのこと。

## Linux v0.01のタスクスケジューラについて

タスクを管理する配列がそのままランキュー(長さは64)

タイムスライスは固定150ms

curentのタイムスライス切れ or 全タスクがsleepならスケジューラを呼ぶ

ランキューを全捜査して残りタイムスライスが最大のものを次に動かす

該当者がいなければ全タスクのタイムスライスをリセット

タイムスライスが切れているタスクには150ms与える

sleep中のタスクには残りタイムスライス/2をボーナスとして与える

## スレッドの実装

- スタックの保存
    - スレッドごとに固定配列をもたせる？
- レジスタの保存
    - スレッドのスタックに保存しておく、rspだけはthread構造体で管理しておく？
- 周期タイマ
    - タイマ割り込み関数内で周期を数える
- スケジューラ呼び出し
    - 周期タイマで一定の周期を数え、周期が来たら周期タイマ内でスケジューラを呼び出す

## `fork_thread()` の実装

まず `struct thread` に、子スレッドのtidである `cpid` を追加する。

``` c
+ #define NTHREAD_CHILD 40

struct thread_func {
    void (*func)(int, char**);
    int argc;
    char **argv;
};

typedef enum thread_state {
    RUNNABLE,
    WAIT,
    SLEEP,
    DEAD,
} thread_state;

typedef struct thread {
    uint64_t *stack;
    uint64_t *rsp;
    uint64_t *rip;
    struct thread_func func_info;
    enum thread_state state;
    tid_t tid;
    tid_t ptid;
+   tid_t ctid[NTHREAD_CHILD];
    int index;
    sid_t sem;
} thread;
```

これをコピーしたい。

次の動作をする `fork_thread()` を実装する。

1. まずスタックのコピー
2. 次は `struct thread` のコピーと `rsp,stack` の値の調整
3. threadを `threads` 配列に格納して `index` の調整
4. `tid,ptid,ctid` の調整

