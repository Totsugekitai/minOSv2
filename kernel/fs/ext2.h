#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define SBLOCK_DISK_LBA 2
#define BG_GROUP_DSC_TBL_LBA 3
#define SBLOCK_LENGTH 1024

// superblock of ext2
struct sblock_ext2 {
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
    uint32_t s_first_ino;   // index to the first inode usable for standard files
    uint16_t s_inode_size;  // the size of the inode struct
    uint16_t s_block_group_nr;  // indicate the block group number hosting this superblock struct
    uint32_t s_feature_compat;  // bitmask of compatible features
    uint32_t s_feature_incompat;    // bitmask of incompatible features
    uint32_t s_feature_ro_compat;   // bitmask of read-only features
    uint64_t s_uuid[2]; // volume id
    char s_volume_name[16]; // volume name
    char s_last_mounted[64];    // directory path where the filesystem was last mounted
    uint32_t s_algo_bitmap; // compression algorithms to determine the compression method used
    // Performance hints
    uint8_t s_prealloc_blocks;  // pre-allocate when creating a new regular file
    uint8_t s_prealloc_dir_blocks;  // pre-allocate when creating a new directory
    uint16_t align;
    // journaling support
    uint64_t s_journal_uuid[2]; // the uuid of the journal superblock
    uint32_t s_journal_inum;    // inode number of the journal file
    uint32_t s_journal_dev;     // device number of the journal file
    uint32_t s_last_orphan;     // inode number, pointing to the first inode in the list of inodes to delete
    // directory indexing support
    uint32_t s_hash_seed[4];    // the seed used for the hash algorithm for directory indexing
    uint8_t s_def_hash_version; // the default hash version used for directory indexing
    uint8_t pad[3];
    // other options
    uint32_t s_default_mount_options;   // the default mount options for this filesystem
    uint32_t s_first_meta_bg;   // the block group ID of the first meta block group
    uint64_t unused[95];
};

// block group descriptor entry
struct bg_dsc_ext2 {
    uint32_t bg_block_bitmap;   // block id of the first block of the "block bitmap" for the group represented
    uint32_t bg_inode_bitmap;   // block id of the first block of the "inode bitmap" for the group represented
    uint32_t bg_inode_table;    // block id of the first block of the "inode table" for the group represented
    uint16_t bg_free_blocks_count;  // total number of free blocks for the represented group
    uint16_t bg_free_inodes_count;  // total number of free inodes for the represented group
    uint16_t bg_used_dirs_count;    // the number of inodes allocated to directories for the represented group
    uint16_t bg_pad;    // padding
    uint32_t bg_reserved[3];    // reserved
};

#include "../device/ahci.h"
void ext2_sblock_check(HBA_PORT *port, int portno);

#endif
