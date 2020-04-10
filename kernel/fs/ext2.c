#include <stdint.h>
#include "ext2.h"
#include "../device/ahci.h"
#include "../device/serial.h"

void ext2_sblock_check(HBA_PORT *port, int portno)
{
    struct sblock_ext2 sb;

    //HBA_PORT *port = 0;
    //int portno = probe_impl_port(port);

    ahci_read(port, portno, SBLOCK_DISK_LBA, SBLOCK_LENGTH / AHCI_COUNT, &sb);

    puts_serial("---------- ext2 filesystem super block parameter ----------\n");
    putsn_serial("s_inodes_count:      ", sb.s_inodes_count);
    putsn_serial("s_blocks_count:      ", sb.s_blocks_count);
    putsn_serial("s_r_blocks_count:    ", sb.s_r_blocks_count);
    putsn_serial("s_free_blocks_count: ", sb.s_free_blocks_count);
    putsn_serial("s_free_inodes_count: ", sb.s_free_inodes_count);
    putsn_serial("s_first_data_block:  ", sb.s_first_data_block);
    putsn_serial("s_log_block_size:    ", sb.s_log_block_size);
    putsn_serial("s_log_frag_size:     ", sb.s_log_frag_size);
    putsn_serial("s_blocks_per_group:  ", sb.s_blocks_per_group);
    putsn_serial("s_frags_per_group:   ", sb.s_frags_per_group);
    putsn_serial("s_inodes_per_group:  ", sb.s_inodes_per_group);
    putsn_serial("s_mtime:             ", sb.s_mtime);
    putsn_serial("s_wtime:             ", sb.s_wtime);
    putsn_serial("s_mnt_count:         ", sb.s_mnt_count);
    putsn_serial("s_max_mnt_count:     ", sb.s_max_mnt_count);
    putsn_serial("s_magic:             ", sb.s_magic);
    putsn_serial("s_state:             ", sb.s_state);
    putsn_serial("s_errors:            ", sb.s_errors);
    putsn_serial("s_minor_rev_level:   ", sb.s_minor_rev_level);
    putsn_serial("s_lastcheck:         ", sb.s_lastcheck);
    putsn_serial("s_checkinterval:     ", sb.s_checkinterval);
    putsn_serial("s_creator_os:        ", sb.s_creator_os);
    putsn_serial("s_rev_level:         ", sb.s_rev_level);
    putsn_serial("s_def_resuid:        ", sb.s_def_resuid);
    putsn_serial("s_def_resgid:        ", sb.s_def_resgid);
    puts_serial("--- EXT2_DYNAMIC_REV specific -----------------------------\n");
    putsn_serial("s_first_ino:         ", sb.s_first_ino);
    putsn_serial("s_inode_size:        ", sb.s_inode_size);
    putsn_serial("s_block_group_nr:    ", sb.s_block_group_nr);
    putsn_serial("s_feature_compat:    ", sb.s_feature_compat);
    putsn_serial("s_feature_incompat:  ", sb.s_feature_incompat);
    putsn_serial("s_feature_ro_compat: ", sb.s_feature_ro_compat);
    putsn_serial("s_uuid[0]:           ", sb.s_uuid[0]);
    putsn_serial("s_uuid[1]:           ", sb.s_uuid[1]);
    putsn_serial("s_algo_bitmap:       ", sb.s_algo_bitmap);
    puts_serial("-----------------------------------------------------------\n");
}
