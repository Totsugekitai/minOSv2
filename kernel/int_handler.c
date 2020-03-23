#include <stdint.h>
#include "util.h"
#include "graphics.h"
#include "int_handler.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

__attribute__((interrupt))
void de_handler(struct intr_frame *frame, uint64_t error_code)
{
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
    printstrnum(100, 100, white, black, "error code: ", error_code);
    printstrnum(100, 116, white, black, "rip       : ", frame->rip);
    printstrnum(100, 132, white, black, "cs        : ", frame->cs);
    printstrnum(100, 148, white, black, "rflags    : ", frame->rflags);
    printstrnum(100, 164, white, black, "rsp       : ", frame->rsp);
    printstrnum(100, 180, white, black, "ss        : ", frame->ss);
    halt();
}

