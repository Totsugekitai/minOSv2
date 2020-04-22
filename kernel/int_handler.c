#include <stdint.h>
#include "util.h"
#include "graphics.h"
#include "task.h"
#include "int_handler.h"
#include "device/serial.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

/* ------------------------------------
 * user defined handlers
 * ------------------------------------
 */
uint64_t tick;
extern uint64_t timer_period;
extern uint64_t previous_interrupt;
__attribute__((interrupt))
void timer_handler(struct intr_frame *frame)
{
    tick++;
    io_out8(PIC0_OCW2, PIC_EOI);
    io_out8(PIC1_OCW2, PIC_EOI);

    if ((tick % 0x10) == 0) {
        printstrnum(400, 0, blue, white, "tick: ", tick);
    }
    /** 周期が来たらスケジューラを呼び出す
     * 各種パラメータはint_handler.hで設定
     */
    if (tick > previous_interrupt + timer_period && tick > 90) {
        previous_interrupt = tick;
        puts_serial("scheduler\r\n");
        thread_scheduler();
    }
}

char key = '\n';
uint32_t x_ichi = 0;
uint32_t y_ichi = 0;
extern uint16_t comport;
__attribute__((interrupt))
void com_handler(struct intr_frame *frame)
{
    io_cli();
    key = io_in8(comport);
    io_out8(PIC0_OCW2, PIC_EOI);
    io_out8(PIC1_OCW2, PIC_EOI);
    puts_serial("COM Interrupt: ");
    putn_serial(key);
    puts_serial("\r\n");

    if (key != '\n' && key != '\r' && key != 0x7f) {    // 0x7f = DEL
        char s[2];
        s[0] = key;
        s[1] = '\0';
        printstr(50 + x_ichi, 50 + y_ichi, green, black, s);
        x_ichi += 8;
    } else if (key == 0x7f) {
        x_ichi -= 8;
        printstr(50 + x_ichi, 50 + y_ichi, white, white, " ");
    } else {
        x_ichi = 0;
        y_ichi += 16;
    }

    io_sti();
}

/* ------------------------------------
 * exception handlers
 * ------------------------------------
 */
__attribute__((interrupt))
void de_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#DE");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void db_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#DB");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void nmi_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#NMI");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void bp_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#BP");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void of_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#OF");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void br_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#BR");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void ud_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#UD");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void nm_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#NM");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void df_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#DF");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void ts_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#TS");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void np_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#NP");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void ss_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#SS");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void gp_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#GP");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void pf_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#PF");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void mf_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#MF");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void ac_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#AC");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void mc_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#MC");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

__attribute__((interrupt))
void xm_handler(struct intr_frame *frame, uint64_t error_code)
{
    printstr(100, 84, white, black, "#XM");
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

