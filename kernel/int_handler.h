#ifndef INT_HANDLER_H
#define INT_HANDLER_H

#include <stdint.h>

struct intr_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void de_handler(struct intr_frame *frame, uint64_t error_code);
void db_handler(struct intr_frame *frame, uint64_t error_code);
void nmi_handler(struct intr_frame *frame, uint64_t error_code);
void bp_handler(struct intr_frame *frame, uint64_t error_code);
void of_handler(struct intr_frame *frame, uint64_t error_code);
void br_handler(struct intr_frame *frame, uint64_t error_code);
void ud_handler(struct intr_frame *frame, uint64_t error_code);
void nm_handler(struct intr_frame *frame, uint64_t error_code);
void df_handler(struct intr_frame *frame, uint64_t error_code);
void ts_handler(struct intr_frame *frame, uint64_t error_code);
void np_handler(struct intr_frame *frame, uint64_t error_code);
void ss_handler(struct intr_frame *frame, uint64_t error_code);
void gp_handler(struct intr_frame *frame, uint64_t error_code);
void pf_handler(struct intr_frame *frame, uint64_t error_code);
void mf_handler(struct intr_frame *frame, uint64_t error_code);
void ac_handler(struct intr_frame *frame, uint64_t error_code);
void mc_handler(struct intr_frame *frame, uint64_t error_code);
void xm_handler(struct intr_frame *frame, uint64_t error_code);

#endif
