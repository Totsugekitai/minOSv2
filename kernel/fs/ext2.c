#include <stdint.h>
#include "ext2.h"
#include "../device/ahci.h"
#include "../device/serial.h"

uint16_t block_size;
uint32_t s_first_ino;

#define BLOCK (block_size / 512)

static inline void ext2_check_sblock(struct sblock_ext2 *sb)
{
    struct port_and_portno p = probe_impl_port();
    ahci_read(p.port, p.portno, SBLOCK_DISK_LBA, SBLOCK_LENGTH / AHCI_COUNT, sb);

    puts_serial("---------- ext2 filesystem super block parameter ----------\n");
    putsn_serial("s_inodes_count:      ", sb->s_inodes_count);
    putsn_serial("s_blocks_count:      ", sb->s_blocks_count);
    putsn_serial("s_r_blocks_count:    ", sb->s_r_blocks_count);
    putsn_serial("s_free_blocks_count: ", sb->s_free_blocks_count);
    putsn_serial("s_free_inodes_count: ", sb->s_free_inodes_count);
    putsn_serial("s_first_data_block:  ", sb->s_first_data_block);
    putsn_serial("s_log_block_size:    ", sb->s_log_block_size);
    putsn_serial("s_log_frag_size:     ", sb->s_log_frag_size);
    putsn_serial("s_blocks_per_group:  ", sb->s_blocks_per_group);
    putsn_serial("s_frags_per_group:   ", sb->s_frags_per_group);
    putsn_serial("s_inodes_per_group:  ", sb->s_inodes_per_group);
    putsn_serial("s_mtime:             ", sb->s_mtime);
    putsn_serial("s_wtime:             ", sb->s_wtime);
    putsn_serial("s_mnt_count:         ", sb->s_mnt_count);
    putsn_serial("s_max_mnt_count:     ", sb->s_max_mnt_count);
    putsn_serial("s_magic:             ", sb->s_magic);
    putsn_serial("s_state:             ", sb->s_state);
    putsn_serial("s_errors:            ", sb->s_errors);
    putsn_serial("s_minor_rev_level:   ", sb->s_minor_rev_level);
    putsn_serial("s_lastcheck:         ", sb->s_lastcheck);
    putsn_serial("s_checkinterval:     ", sb->s_checkinterval);
    putsn_serial("s_creator_os:        ", sb->s_creator_os);
    putsn_serial("s_rev_level:         ", sb->s_rev_level);
    putsn_serial("s_def_resuid:        ", sb->s_def_resuid);
    putsn_serial("s_def_resgid:        ", sb->s_def_resgid);
    puts_serial("--- EXT2_DYNAMIC_REV specific -----------------------------\n");
    putsn_serial("s_first_ino:         ", sb->s_first_ino);
    putsn_serial("s_inode_size:        ", sb->s_inode_size);
    putsn_serial("s_block_group_nr:    ", sb->s_block_group_nr);
    putsn_serial("s_feature_compat:    ", sb->s_feature_compat);
    putsn_serial("s_feature_incompat:  ", sb->s_feature_incompat);
    putsn_serial("s_feature_ro_compat: ", sb->s_feature_ro_compat);
    putsn_serial("s_uuid[0]:           ", sb->s_uuid[0]);
    putsn_serial("s_uuid[1]:           ", sb->s_uuid[1]);
    puts_serial("s_volume_name:        ");
    nputs_serial(sb->s_volume_name, 16);
    puts_serial("s_last_mounted:       ");
    nputs_serial(sb->s_last_mounted, 64);
    putsn_serial("s_algo_bitmap:       ", sb->s_algo_bitmap);
    puts_serial("--- performance hints -------------------------------------\n");
    putsn_serial("s_prealloc_blocks:   ", sb->s_prealloc_blocks);
    putsn_serial("s_prealloc_dir_blocks:", sb->s_prealloc_dir_blocks);
    puts_serial("--- journaling support ------------------------------------\n");
    putsn_serial("s_journal_uuid[0]:   ", sb->s_journal_uuid[0]);
    putsn_serial("s_journal_uuid[1]:   ", sb->s_journal_uuid[1]);
    putsn_serial("s_journal_inum:      ", sb->s_journal_inum);
    putsn_serial("s_journal_dev:       ", sb->s_journal_dev);
    putsn_serial("s_last_orphan:       ", sb->s_last_orphan);
    puts_serial("--- directory indexing support ----------------------------\n");
    putsn_serial("s_hash_seed[0]:      ", sb->s_hash_seed[0]);
    putsn_serial("s_hash_seed[1]:      ", sb->s_hash_seed[1]);
    putsn_serial("s_hash_seed[2]:      ", sb->s_hash_seed[2]);
    putsn_serial("s_hash_seed[3]:      ", sb->s_hash_seed[3]);
    putsn_serial("s_def_hash_version:  ", sb->s_def_hash_version);
    puts_serial("--- other options -----------------------------------------\n");
    putsn_serial("s_default_mount_options:", sb->s_default_mount_options);
    putsn_serial("s_first_meta_bg:     ", sb->s_first_meta_bg);
    puts_serial("-----------------------------------------------------------\n");
}

struct bg_dsc_ext2 ext2_check_bg_dsc(void)
{
    struct bg_dsc_ext2 bg[32];
    struct port_and_portno p = probe_impl_port();
    ahci_read(p.port, p.portno, SBLOCK_DISK_LBA + 2, BLOCK, &bg);
    puts_serial("============ Ext2 Block Group Descriptor Table ============\n");
    for (int i = 0; i < 32; i++) {
        if (bg[i].bg_block_bitmap) {
            putsn_serial("--- number ", i);
            putsn_serial("bg_block_bitmap: ", bg[i].bg_block_bitmap);
            putsn_serial("bg_inode_bitmap: ", bg[i].bg_inode_bitmap);
            putsn_serial("bg_inode_table: ", bg[i].bg_inode_table);
            putsn_serial("bg_free_blocks_count: ", bg[i].bg_free_blocks_count);
            putsn_serial("bg_free_inodes_count: ", bg[i].bg_free_inodes_count);
            putsn_serial("bg_used_dir_count: ", bg[i].bg_used_dirs_count);
            puts_serial("----------\n");
        }
    }
    puts_serial("===========================================================\n");
    return bg[0];
}

void ext2_check_inode_bitmap(uint32_t inode_bitmap_location)
{
    char bitmap[block_size];
    struct port_and_portno p = probe_impl_port();
    ahci_read(p.port, p.portno, inode_bitmap_location * 2, BLOCK, bitmap);
    for (int i = 0; i < block_size; i++) {
        putn_serial(bitmap[i]);
    }
}

void ext2_check_inode_table(uint32_t inode_table_location, uint32_t inodes_per_group)
{
    //struct inode_ext2 inodes[inodes_per_group];
    //char block[1024];
    //struct port_and_portno p = probe_impl_port();
    uint32_t inodes_per_block = BLOCK / sizeof(struct inode_ext2);
    uint32_t nblock_inode_table = inodes_per_group / inodes_per_block;
    if (inodes_per_group % inodes_per_block) {
        nblock_inode_table++;
    }
    puts_serial("========== inodes ==========\r\n");
    for (uint32_t i = 0; i < nblock_inode_table; i++) {
        struct inode_ext2 inode[8];
        ahci_read_byte(inode_table_location * 2, BLOCK, &inode, sizeof(struct inode_ext2));
        //ahci_read(p.port, p.portno, (inode_table_location + i) * 2, BLOCK, block);
        //struct inode_ext2 *inode = (struct inode_ext2 *)block;
        //inodes[i] = *inode;
        for (unsigned long j = 0; j < inodes_per_block; j++) {
            putsn_serial("--------- inode number: ", i);
            putsn_serial("i_mode:        ", inode[j].i_mode);
            putsn_serial("i_uid:         ", inode[j].i_uid);
            putsn_serial("i_size:        ", inode[j].i_size);
            putsn_serial("i_atime:       ", inode[j].i_atime);
            putsn_serial("i_ctime:       ", inode[j].i_ctime);
            putsn_serial("i_mtime:       ", inode[j].i_mtime);
            putsn_serial("i_dtime:       ", inode[j].i_dtime);
            putsn_serial("i_gid:         ", inode[j].i_gid);
            putsn_serial("i_links_count: ", inode[j].i_links_count);
            putsn_serial("i_blocks:      ", inode[j].i_blocks);
            putsn_serial("i_flags:       ", inode[j].i_flags);
            putsn_serial("i_osd1:        ", inode[j].i_osd1);
            putsn_serial("i_block[0]:    ", inode[j].i_block[0]);
            putsn_serial("i_block[1]:    ", inode[j].i_block[1]);
            putsn_serial("i_block[2]:    ", inode[j].i_block[2]);
            putsn_serial("i_block[3]:    ", inode[j].i_block[3]);
            putsn_serial("i_block[4]:    ", inode[j].i_block[4]);
            putsn_serial("i_block[5]:    ", inode[j].i_block[5]);
            putsn_serial("i_block[6]:    ", inode[j].i_block[6]);
            putsn_serial("i_block[7]:    ", inode[j].i_block[7]);
            putsn_serial("i_block[8]:    ", inode[j].i_block[8]);
            putsn_serial("i_block[9]:    ", inode[j].i_block[9]);
            putsn_serial("i_block[10]:   ", inode[j].i_block[10]);
            putsn_serial("i_block[11]:   ", inode[j].i_block[11]);
            putsn_serial("i_block[12]:   ", inode[j].i_block[12]);
            putsn_serial("i_block[13]:   ", inode[j].i_block[13]);
            putsn_serial("i_block[14]:   ", inode[j].i_block[14]);
            putsn_serial("i_generation:  ", inode[j].i_generation);
            putsn_serial("i_file_acl:    ", inode[j].i_file_acl);
            putsn_serial("i_dir_acl:     ", inode[j].i_dir_acl);
            putsn_serial("i_faddr:       ", inode[j].i_faddr);
        }
            }
    puts_serial("=======================================\r\n");
}

void ext2_check_rootdir_inode(uint32_t inode_table_location)
{
    struct inode_ext2 root_inode[2];
    ahci_read_byte(inode_table_location * BLOCK, BLOCK, &root_inode, sizeof(struct inode_ext2) * 2);
    putsn_serial("i_mode:        ", root_inode[1].i_mode);
    putsn_serial("i_uid:         ", root_inode[1].i_uid);
    putsn_serial("i_size:        ", root_inode[1].i_size);
    putsn_serial("i_atime:       ", root_inode[1].i_atime);
    putsn_serial("i_ctime:       ", root_inode[1].i_ctime);
    putsn_serial("i_mtime:       ", root_inode[1].i_mtime);
    putsn_serial("i_dtime:       ", root_inode[1].i_dtime);
    putsn_serial("i_gid:         ", root_inode[1].i_gid);
    putsn_serial("i_links_count: ", root_inode[1].i_links_count);
    putsn_serial("i_blocks:      ", root_inode[1].i_blocks);
    putsn_serial("i_flags:       ", root_inode[1].i_flags);
    putsn_serial("i_osd1:        ", root_inode[1].i_osd1);
    putsn_serial("i_block[0]:    ", root_inode[1].i_block[0]);
    putsn_serial("i_block[1]:    ", root_inode[1].i_block[1]);
    putsn_serial("i_block[2]:    ", root_inode[1].i_block[2]);
    putsn_serial("i_block[3]:    ", root_inode[1].i_block[3]);
    putsn_serial("i_block[4]:    ", root_inode[1].i_block[4]);
    putsn_serial("i_block[5]:    ", root_inode[1].i_block[5]);
    putsn_serial("i_block[6]:    ", root_inode[1].i_block[6]);
    putsn_serial("i_block[7]:    ", root_inode[1].i_block[7]);
    putsn_serial("i_block[8]:    ", root_inode[1].i_block[8]);
    putsn_serial("i_block[9]:    ", root_inode[1].i_block[9]);
    putsn_serial("i_block[10]:   ", root_inode[1].i_block[10]);
    putsn_serial("i_block[11]:   ", root_inode[1].i_block[11]);
    putsn_serial("i_block[12]:   ", root_inode[1].i_block[12]);
    putsn_serial("i_block[13]:   ", root_inode[1].i_block[13]);
    putsn_serial("i_block[14]:   ", root_inode[1].i_block[14]);
    if ((root_inode->i_mode & 0x4000) && !(root_inode->i_mode & 0x2000)) {
        puts_serial("This is directory\r\n");
    } else {
        puts_serial("This is not directory\r\n");
    }
}

void init_ext2(void)
{
    struct sblock_ext2 sb;
    struct port_and_portno p = probe_impl_port();
    ahci_read(p.port, p.portno, SBLOCK_DISK_LBA, SBLOCK_LENGTH / AHCI_COUNT, &sb);
    block_size = (1024 << sb.s_log_block_size);
    s_first_ino = sb.s_first_ino;
    putsn_serial("block size: ", block_size);
}

void check_ext2(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    init_ext2();
    struct sblock_ext2 sb;
    ext2_check_sblock(&sb);
    struct bg_dsc_ext2 bg = ext2_check_bg_dsc();
    //ext2_check_inode_bitmap(bg.bg_inode_bitmap);
    //ext2_check_inode_table(bg.bg_inode_table, sb.s_inodes_per_group);
    ext2_check_rootdir_inode(bg.bg_inode_table);
}
