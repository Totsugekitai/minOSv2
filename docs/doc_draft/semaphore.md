# 並行スレッドの協調

スレッド同士を通信させたいとする。
スレッドAからスレッドBへメッセージを送りたいが、何が必要だろうか。

とりあえずメッセージの受け渡しに共有できるキューが必要そうである。

このときに考えなくてはならないことがある。
キューに対してスレッドAがデータを書き込み、スレッドBがデータを読み出すのであるが、順番が重要だということだ。

スレッドAがもし先にキューを読み出してしまったらキューは空である。
スレッドBの書き込みの途中にスレッドAが読みだしたら不完全なデータがスレッドAに渡ってしまう。

ここで覚えておくことは、スレッド同士はお互いの様子を直接覗くことができないということだ。
スレッドAはスレッドBが書き終えたかを直接知ることはできないし、逆もしかりである。

これらの問題を解決するにはどうしたらいいだろうか。

## セマフォの導入

そこでセマフォという概念を導入して、これを解決する。
セマフォとは、並行処理の各スレッドの動きを制御する機構である。

### セマフォの概念

![「分かりそう」で「分からない」でも「分かった」気になれるIT用語辞典](https://wa3.i-3-i.info/word13357.html) にはこのように書かれている。

> 同時に使える人の数が決まっている物に割り当てられた「あと何人の人が使えるよ」を表した数字

セマフォは、スレッド間でセマフォカウンタと呼ばれるものを共有する。
スレッド達はセマフォカウンタを見て自身がどう動くかを決定する。

minOS には `wait()` と `signal()` という関数を導入し、セマフォカウンタの操作を行う。
セマフォカウンタをインクリメントする動作を **V操作** 、デクリメントする動作を **P操作** という。

`wait()` はP操作に、 `signal()` はV操作に対応する。

また、 `wait()` と `signal()` はスレッドから呼び出すことを述べておく。

### セマフォカウンタの同時アクセスをどう防ぐか

minOS ではタイマ割り込み時にスレッドのスケジューラを動作させてスレッドを切り替えている。

つまり、セマフォカウンタの書き換えとその参照時には、割り込みを禁止しておけばスケジューラが呼び出されること無く、スレッドの切替が走ることがない。

したがって、セマフォの実装の際には以下のようになる。

``` c
ret sem_func(arg)
{
    io_cli();

    //セマフォの処理

    io_sti();
    return ret;
}
```

### `wait()` の設計

`wait()` はその名の通り、セマフォカウンタが正の値になるまで呼び出したスレッドを待たせる関数である。

`wait()` はP操作であり、セマフォカウンタをデクリメントする。
デクリメントした際にセマフォカウンタが負であれば、そのスレッドは `WAIT` 状態に入り、スケジューラの対象から外される。

またその際に、スレッドIDがセマフォ上のキューに記録され、再開時に参照される。

つまり以下のような感じになる。

``` c
int wait(struct semaphore s)
{
    io_cli();

    s.counter--;

    if (s.counter < 0) {
        thread_id id = get_thread_id();
        struct thread *t = get_thread(id);
        t->thread_state = WAIT;
        enqueue(s.thread_id_queue, id);
    }

    io_sti();
    return 1;
}
```

### `signal()` の設計

`signal()` はセマフォ上で待機しているスレッドに対して通知を送る関数である。

`signal()` はV操作であり、セマフォカウンタをインクリメントさせる。
インクリメントした際にセマフォカウンタが正の値であれば、セマフォ上のキューからスレッドIDを取り出し、そのスレッドを `RUNNABLE` 状態にして、スケジューラの対象とする。

つまり、以下のような感じになる。

``` c
int signal(struct semaphore s)
{
    io_cli();

    s.counter++;

    if (s.counter >= 0) {
        thread_id id = dequeue(s.thread_id_queue);
        struct thread *t = get_thread(id);
        t->thread_state = RUNNABLE;
    }

    io_sti();
    return 1;
}
```

### セマフォの設計

ここまでの議論で、セマフォには次の要素が必要だとわかる。

- セマフォカウンタ
- スレッドIDキュー

また、複数のセマフォを管理したり、セマフォに状態を付けたりするためのパラメータも追加する。

- セマフォID
- セマフォの状態

つまり、このような感じになる。

``` c
struct semaphore {
    int semaphore_counter;
    int semaphore_id;
    enum state semaphore_state;
    struct queue thread_id_queue;
};
```

## 実装解説

`kernel/semaphore.h` と `kernel/semaphore.c` に実装があります。
これを見ていきます。

### `semaphore.h` の定義確認

``` c
typedef int sid_t;

#define NSEM 100
#define NSEMQUEUE 30

// if semaphore table entry is used
#define S_FREE 0
#define S_USED 1

typedef struct queue_sem {
    int head;
    int num;
    tid_t que[NSEMQUEUE];
} queue_sem;

// semaphore table entry
typedef struct semtable_entry {
    int semstate;
    int semcount;
    queue_sem semqueue;
} semtable_entry;

#define isbadsem(s) ((s) < 0 || (s) >= NSEM)
```

まず `sid_t` 型を定義します。
セマフォIDです。
複数のセマフォを管理する必要があるので、IDの型を設定します。

`NSEM` で全体のセマフォの数を、 `NSEMQUEUE` で一つのセマフォ上のキューに入るスレッドの最大数を定義します。

`S_FREE` と `S_USED` でセマフォの状態を管理します。
そのセマフォIDが使われているか、空いているかを判別します。

`queue_sem` はセマフォ上でスレッドIDを格納するキューです。
`head` が先頭、 `num` が格納されている数、 `que` がキュー本体です。
キューは配列をリングバッファにして扱います。

`semtable_entry` はセマフォ本体です。
この構造体を配列に格納して管理します。
`semstate` が状態で、 `S_FREE` か `S_USED` が入ります。
`semcount` がセマフォカウンタです、この値を上げたり下げたりします。
`semqueue` は上で説明したスレッドIDのキューです。

## `semaphore.c` の実装確認

``` c
semtable_entry semtable[NSEM];
...
int wait(sid_t sem)
{
    io_cli();

    STI_RET0_IF_ERR(!isbadsem(sem));
    STI_RET0_IF_ERR(semtable[sem].semstate == S_USED);

    semtable[sem].semcount--;
    if (semtable[sem].semcount < 0) {
        tid_t tid = get_cur_thread_tid();
        STI_RET0_IF_ERR(change_state(tid, WAIT));
        thread *t = get_thread_ptr(tid);
        *t->sid_ptr = sem;
        STI_RET0_IF_ERR(enqueue_sem(tid, sem));
        putsn_serial("enqueue tid: ", tid);
        thread_scheduler();
    }

    io_sti();
    return 1;
}

int signal(sid_t sem)
{
    io_cli();
    STI_RET0_IF_ERR(!isbadsem(sem));
    STI_RET0_IF_ERR(semtable[sem].semstate == S_USED);

    if (semtable[sem].semcount < 0) {
        semtable[sem].semcount++;
        tid_t tid;
        STI_RET0_IF_ERR(dequeue_sem(&tid, sem));
        putsn_serial("dequeue tid: ", tid);
        STI_RET0_IF_ERR(change_state(tid, RUNNABLE));
        thread_scheduler();
    }

    io_sti();
    return 1;
}
...
```

最初にセマフォを格納するグローバルな配列を定義しています。
この配列にセマフォを格納していきます。

#### まず `wait` の確認

`wait()` を見てみましょう。

ちょくちょく出てくる `STI_RET0_IF_ERR()` は値のチェックです。
正しい値でなければ `io_sti()` して `0` をリターンするマクロです。

`if` の手前で `semcount` をデクリメントします。

そして `semcount` をチェックし、もし `0` 未満だったらスレッドを待機させる処理に移ります。

現在のスレッドIDを取得し、スレッドへのポインタを獲得します。
`change_state()` でスレッドの状態を `WAIT` へ変更します。
そしてキューにスレッドIDを格納します。

最後にスケジューラを呼び出し、リスケジュールします。

#### 次に `signal` の確認

`signal()` を見ます。

`semcount` が `0` 以上だった場合は特に何もしません。
待っているスレッドがいないのでなにもする必要がないです。

`semcount` が `0` 未満だったときは待っているスレッドがいるので、処理をします。
まず `semcount` をインクリメントし、セマフォ上のキューからスレッドIDを取り出します。
取り出したスレッドIDを用いてスレッドの状態を `change_state()` で `RUNNABLE` に変更します。

最後にスケジューラを呼び出し、リスケジュールします。
