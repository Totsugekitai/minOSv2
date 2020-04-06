#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

struct superblk_ext2 {
    uint32_t s_inodes_count;    // total number of inodes
    uint32_t s_blocks_count;    // total number of blocks
    uint32_t s_r_blocks_count;  // total number of blocks reserved fo the usage of the superuser
    uint32_t s_free_blocks_count;   // total number of free blocks
    uint32_t s_free_inodes_count;   // total number of free inodes
    uint32_t s_first_data_block;    // info of the superblock
    uint32_t s_log_block_size;  // using to calc block size
    uint32_t s_log_frag_size;   // using to calc fragment size
    uint32_t s_blocks_per_group;    // total number of blocks per group
    uint32_t s_frags_per_group;     // total number of fragments per group
    uint32_t s_inodes_per_group;    // total number of inodes per group
    uint32_t s_mtime;   // the last time the filesystem was mounted
    uint32_t s_wtime;   // the last write access
    uint16_t s_mnt_count;   // how many time the filesystem was mounted
    uint16_t s_max_mnt_count;   // the max number of times the filesystem may be mounted
    uint16_t s_magic;   // magic number
    uint16_t s_state;   // state: valid = 1, error = 2
    uint16_t s_errors;  // what the filesystem driver should do when error is detected
    uint16_t s_minor_rev_level; // minor revision level
    uint32_t s_lastcheck;   // unix time of the last filesystem check
    uint32_t s_checkinterval;   // max unix time interval
    uint32_t s_creator_os;  // Linux = 0
    uint32_t s_rev_level;   // revision level
    uint16_t s_def_resuid;  // default user id for reserved blocks: if OS is Linux, this is 0
    uint16_t s_def_resgid;  // default group id for reserved blocks: if OS is Linux, this is 0
    // EXT2_DYNAMIC_REV specific
};

#endif
