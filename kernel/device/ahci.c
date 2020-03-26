#include <stdint.h>
#include "../util.h"
#include "serial.h"
#include "ahci.h"

#define SATA_SIG_ATA 0x00000101   // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101    // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define CMD_LIST_BASE 0x50000000    // size: 0x20 * 32 * 32 = 0x8000
#define RCVD_FIS_BASE 0x50008000    // size: 0x100 * 32     = 0x2000
#define CMD_TBL_BASE  0x50010000

// AHCI Address Base
// It is defined at pci.c
extern uint64_t *abar;

void put_hba_memory_register(void)
{
    HBA_MEM_REG *hba_memreg = (HBA_MEM_REG *)abar;
    puts_serial("status of Generic Host Control\r\n");
    putsn_serial("cap: ", (uint64_t)hba_memreg->cap);
    putsn_serial("ghc: ", (uint64_t)hba_memreg->ghc);
    putsn_serial("is: ", (uint64_t)hba_memreg->is);
    putsn_serial("pi: ", (uint64_t)hba_memreg->pi);
    putsn_serial("vs: ", (uint64_t)hba_memreg->vs);
    putsn_serial("ccc_ctl: ", (uint64_t)hba_memreg->ccc_ctl);
    putsn_serial("ccc_pts: ", (uint64_t)hba_memreg->ccc_pts);
    putsn_serial("em_loc: ", (uint64_t)hba_memreg->em_loc);
    putsn_serial("em_ctl: ", (uint64_t)hba_memreg->em_ctl);
    putsn_serial("cap2: ", (uint64_t)hba_memreg->cap2);
    putsn_serial("bohc: ", (uint64_t)hba_memreg->bohc);
    puts_serial("\r\n");
}

void put_port_status(HBA_PORT *port)
{
    puts_serial("status of HBA Port\r\n");
    putsn_serial("clb: ", (uint64_t)port->clb);
    putsn_serial("clbu: ", (uint64_t)port->clbu);
    putsn_serial("fb: ", (uint64_t)port->fb);
    putsn_serial("fbu: ", (uint64_t)port->fbu);
    putsn_serial("is: ", (uint64_t)port->is);
    putsn_serial("ie: ", (uint64_t)port->ie);
    putsn_serial("cmd: ", (uint64_t)port->cmd);
    putsn_serial("tfd: ", (uint64_t)port->tfd);
    putsn_serial("sig: ", (uint64_t)port->sig);
    putsn_serial("ssts: ", (uint64_t)port->ssts);
    putsn_serial("sctl: ", (uint64_t)port->sctl);
    putsn_serial("serr: ", (uint64_t)port->serr);
    putsn_serial("sact: ", (uint64_t)port->sact);
    putsn_serial("ci: ", (uint64_t)port->ci);
    putsn_serial("sntf: ", (uint64_t)port->sntf);
    putsn_serial("fbs: ", (uint64_t)port->fbs);
    puts_serial("\r\n");
}

/* ----------------------------------------------------------------------------
 * AHCI minimal initialization utility procedures and functions
 * ----------------------------------------------------------------------------
 */

static inline void hba_reset(void)
{
    HBA_MEM_REG *memreg = (HBA_MEM_REG *)abar;
    // GHC.AE = 1 and GHC.HR = 1 to reset HBA
    memreg->ghc = 0x80000000;
    memreg->ghc |= 0x00000001;
    // during HR = 1, polling
    while (memreg->ghc & 0x1) {
        puts_serial(".");
        __asm__ volatile("hlt");
    }
    puts_serial("\r\n");
}

// if PxCMD.ST, PxCMD.CR, and PxCMD.FRE is clear, the port is idle.
static uint32_t probe_idle_port(uint32_t pi)
{
    HBA_MEM_REG *memreg = (HBA_MEM_REG *)abar;
    uint32_t pidle = 0;
    for (int i = 0; i < 32; i++) {
        if (pi >> i) {
            if ((memreg->ports[i].cmd & 0x811) == 0) {
                pidle |= 1 << i;
            } else {
                putsn_serial("port is busy - PxCMD: ", (uint64_t)memreg->ports[i].cmd);
            }
        }
    }
    return pidle;
}

static inline void alloc_mem_for_ports(uint32_t pi_list, uint16_t slot_num)
{
    mymemset((void *)CMD_LIST_BASE, 0, 0x8000 + 0x2000);
    HBA_PORT *ports = (HBA_PORT *)&(((HBA_MEM_REG *)abar)->ports[0]);
    for (int i = 0; i < 32; i++) {
        if (pi_list >> i) {
            ports[i].clb = CMD_LIST_BASE + sizeof(HBA_CMD_HEADER) * 32 * i;
            ports[i].fb = RCVD_FIS_BASE + sizeof(RCVD_FIS) * i;
            ports[i].cmd |= 0x10;  // PxCMD.FRE is set to 1
        }
    }
}

static inline void clear_ports_serr(uint32_t pi_list)
{
    HBA_PORT *ports = (HBA_PORT *)&(((HBA_MEM_REG *)abar)->ports[0]);
    for (int i = 0; i < 32; i++) {
        if (pi_list >> i) {
            ports[i].serr |= 0x7ff0f03;    // clear by writing 1s to each bit
            while (ports[i].serr)
                __asm__ volatile("hlt");
        }
    }
}

static inline void enable_ahci_interrupt(uint32_t pi_list)
{
    // first, PxIS are cleared to 0
    HBA_MEM_REG *ghc = (HBA_MEM_REG *)abar;
    HBA_PORT *ports = (HBA_PORT *)&(ghc->ports[0]);
    for (int i = 0; i < 32; i++) {
        if (pi_list >> i) {
            ports[i].is = 0xffffffff; // clear pending interrupt bits
            while (ports[i].is) {
                __asm__ volatile("hlt");
                putsn_serial("PxIS clear cannot be finished: ", (uint64_t)ports[i].is);
            }
        }
    }
    // second, IS.IPS is cleared to 0
    ghc->is &= ~(ghc->is);
    while (ghc->is) {
        __asm__ volatile("hlt");
        puts_serial("GHC.IS.IPS clear cannot be finished.\r\n");
    }
    // enable PxIE bit
    for (int i = 0; i < 32; i++) {
        if (pi_list >> i) {
            ports[i].ie = 0xfdc000ff;  // Interrupt Enable all set
        }
    }
    // set GHC.IE to 1
    ghc->ghc |= 0x2;
}

/*
 * Initialization procedure
 * This is MINIMAL initialization.
 * (10.1.2 System Software Specific Initialization)
 * */
static inline void ahci_init(void)
{
    puts_serial("AHCI initialization start.\r\n");
    // initial HBA reset
    hba_reset();
    puts_serial("HBA reset end.\r\n");

    HBA_MEM_REG *memreg = (HBA_MEM_REG *)abar;
    // step 1: GHC.AE = 1
    memreg->ghc |= 0x80000000;
    puts_serial("AHCI init step 1 end.\r\n");
    // step 2: determine implemented port
    uint32_t pi = memreg->pi;
    puts_serial("AHCI init step 2 end.\r\n");
    // step 3: ensure that the controller is not running state
    // if PxCMD.ST, PxCMD.CR, and PxCMD.FRE is clear, the port is idle.
    uint32_t pidle = probe_idle_port(pi);
    while (pi != pidle) {
        putsn_serial("Implement port is: ", pi);
        putsn_serial("Implement port is not idle: ", pidle);
        __asm__ volatile("hlt");
    }
    puts_serial("AHCI init step 3 end.\r\n");
    // step 4: determine how many command slots the HBA supports
    uint16_t slot_num = (uint16_t)((memreg->cap >> 8) & 0x1f);
    puts_serial("AHCI init step 4 end.\r\n");
    // step 5: allocate memory for implemented ports
    // required params: PxCLB and PxFB
    alloc_mem_for_ports(pi, slot_num);
    puts_serial("AHCI init step 5 end.\r\n");
    // step 6: clear PxSERR
    clear_ports_serr(pi);
    puts_serial("AHCI init step 6 end.\r\n");
    // step 7: enable interrupt
    enable_ahci_interrupt(pi);
    puts_serial("AHCI init step 7 end.\r\n");
    puts_serial("All of AHCI initialization end.\r\n");
}

/* ----------------------------------------------------------------------------
 * AHCI Read/Write utility procedures and functions
 * ----------------------------------------------------------------------------
 */

static int find_free_cmdslot(HBA_PORT *port)
{
    uint32_t slots = (port->ci | port->sact);
    for (int i = 0; i < 32; i++) {
        if ((slots & 1) == 0) {
            return i;
        }
        slots >>= 1;
    }
    puts_serial("Cannot find free command list entry\r\n");
    return -1;
}

static inline void build_cmd_table(CMD_PARAMS *params, uint64_t *table_addr)
{
    mymemset((void *)table_addr, 0, 0x80 + 16 * 65536); // zero clear
    HBA_CMD_TBL *table = (HBA_CMD_TBL *)table_addr;

    // build CFIS
    if (params->fis_type == 0x27) { // if Register H2D
        FIS_REG_H2D *h2dfis = (FIS_REG_H2D *)table_addr;
        h2dfis->fis_type = 0x27;    // H2D FIS type magic number
        h2dfis->c = 1;              // This is command
        // command type is referenced in ATA command set
        h2dfis->command = params->cmd_type;
        // device
        //h2dfis->device = 0xe0;
        h2dfis->device = 1 << 6;
        // LBA
        h2dfis->lba0 = (uint8_t)(((uint64_t)(params->lba) >>  0) & 0xff);
        h2dfis->lba1 = (uint8_t)(((uint64_t)(params->lba) >>  8) & 0xff);
        h2dfis->lba2 = (uint8_t)(((uint64_t)(params->lba) >> 16) & 0xff);
        h2dfis->lba3 = (uint8_t)(((uint64_t)(params->lba) >> 24) & 0xff);
        h2dfis->lba4 = (uint8_t)(((uint64_t)(params->lba) >> 32) & 0xff);
        h2dfis->lba5 = (uint8_t)(((uint64_t)(params->lba) >> 40) & 0xff);
        // block count
        h2dfis->countl = (uint8_t)((params->count >> 0) & 0xff);
        h2dfis->counth = (uint8_t)((params->count >> 8) & 0xff);
    } else {
        puts_serial("fis type error\r\n");
        return;
    }

    // build PRD Table
    // 8 KB (16 sectors) per PRD Table
    // 1 sectors = 512 KB
    uint16_t count = params->count;
    int prdtl = (int)((params->count - 1) >> 4) + 1;
    uint8_t *buf = (uint8_t *)params->dba;
    int i;
    for (i = 0; i < prdtl - 1; i++) {
        table->prdt_entry[i].dba  = (uint32_t)buf;
        table->prdt_entry[i].dbau = 0;
        table->prdt_entry[i].dbc = 8 * 1024 - 1;
        table->prdt_entry[i].i = 1; // notify interrupt
        buf += 8 * 1024; // 8K bytes
        count -= 16; // 16 sectors
    }

    // Last entry
    table->prdt_entry[i].dba = (uint32_t)buf;
    table->prdt_entry[i].dbc = (count << 9) - 1;
    table->prdt_entry[i].i = 1;
}

static inline void build_cmdheader(HBA_PORT *port, int slot, CMD_PARAMS *params)
{
    HBA_CMD_HEADER *cmd_list = ((HBA_CMD_HEADER *)port->clb + slot);
    mymemset((void *)cmd_list, 0, 0x400);
    cmd_list->ctba = (uint32_t)CMD_TBL_BASE;
    cmd_list->ctbau = 0;
    cmd_list->prdtl = (uint16_t)(((params->count - 1) >> 4) + 1);
    cmd_list->cfl = params->cfis_len;
    cmd_list->w = params->w;
}

static inline void notify_cmd_is_active(HBA_PORT *port, int slot)
{
    port->ci |= 1 << slot;
}

static inline void build_command(HBA_PORT *port, CMD_PARAMS *params)
{
    int slot = find_free_cmdslot(port);
    // step 1:
    // build a command FIS in system memory at location PxCLB[CH(pFreeSlot)]:CFIS with the command type.
    build_cmd_table(params, (uint64_t *)CMD_TBL_BASE);
    // step 2:
    // build a command header at PxCLB[CH(pFreeSlot)].
    build_cmdheader(port, slot, params);
    // step 3:
    // set PxCI.CI(pFreeSlot) to indicate to the HBA that a command is active.
    notify_cmd_is_active(port, slot);

    puts_serial("build command is over\r\n");
}

static inline void wait_interrupt(HBA_PORT *port)
{
    puts_serial("while waiting interrupt\r\n");
    while (port->is == 0) {
        __asm__ volatile("hlt");
    }
    puts_serial("interrupt comes\r\n");
}

static inline void clear_pxis(HBA_PORT *port)
{
    port->is |= port->is;
    puts_serial("while clear PxIS\r\n");
    while (port->is) {
        __asm__ volatile("hlt");
    }
    puts_serial("clearing PxIS is over\r\n");
}

static inline void clear_ghc_is(int portno)
{
    HBA_MEM_REG *ghc = (HBA_MEM_REG *)abar;
    ghc->is |= 1 << portno;
    puts_serial("while clear IS.IPS\r\n");
    while (ghc->is) {
        __asm__ volatile("hlt");
    }
    puts_serial("clearing IS.IPS is finished\r\n");
}

static inline void stop_cmd(HBA_PORT *port)
{
    puts_serial("stop_cmd start\r\n");
    // clear ST
    port->cmd &= ~0x01;

    // wait until FR, CR are cleared
    while (port->cmd & 0x4000 || port->cmd & 0x8000) {
        __asm__ volatile("hlt");
    }

    // clear FRE
    port->cmd &= ~0x10;
    puts_serial("stop_cmd end\r\n");
}

static inline void start_cmd(HBA_PORT *port)
{
    puts_serial("start_cmd start\r\n");
    port->cmd &= 0xfffffffe;    // PxCMD.ST = 0
    // wait until CR is cleared
    while (port->cmd & 0x8000) { __asm__ volatile("hlt"); }

    // set FRE and ST
    port->cmd |= 0x10;
    port->cmd |= 0x01;
    puts_serial("start_cmd end\r\n");
}

static inline void wait_pxci_clear(HBA_PORT *port)
{
    puts_serial("wait PxCI\r\n");
    while (port->ci) {
        __asm__ volatile("hlt");
    }
    puts_serial("wait PxCI end\r\n");
}

int ahci_read_test(HBA_PORT *port, int portno, uint64_t start, uint16_t count, uint16_t *buf)
{
    start_cmd(port);
    CMD_PARAMS params;
    params.fis_type = 0x27;
    params.cmd_type = READ_DMA_EXT;
    params.cfis_len = 5;
    params.lba = (uint64_t *)start;
    params.count = count;
    params.dba = (uint64_t *)buf;
    params.w = 0;
    build_command(port, &params);
    wait_interrupt(port);
    clear_pxis(port);
    clear_ghc_is(portno);
    wait_pxci_clear(port);
    return 1;
}

int ahci_write_test(HBA_PORT *port, int portno, uint64_t start, uint16_t count, uint16_t *buf)
{
    start_cmd(port);
    CMD_PARAMS params;
    params.fis_type = 0x27;
    params.cmd_type = WRITE_DMA_EXT;
    params.cfis_len = 5;
    params.lba = (uint64_t *)start;
    params.count = count;
    params.dba = (uint64_t *)buf;
    params.w = 1;
    build_command(port, &params);
    wait_interrupt(port);
    clear_pxis(port);
    clear_ghc_is(portno);
    wait_pxci_clear(port);
    return 1;
}

void check_ahci(void)
{
    ahci_init();    // AHCI initialization
    put_hba_memory_register();

    uint32_t pi = ((HBA_MEM_REG *)abar)->pi;
    int k = -1;
    for (int i = 0; i < 32; i++) {
        if ((pi & 1) == 1) {
            k = i;
            break;
        } else {
            pi >>= 1;
        }
    }
    if (k == -1) {
        puts_serial("implemented port not found\r\n");
        return;
    }
    HBA_PORT *port= &((HBA_MEM_REG *)abar)->ports[k];

    uint16_t buf[512];
    for (int i = 0; i < 512; i++) {
        buf[i] = 0xbeef;
    }

    ahci_read_test(port, 0, 0x0, 1, buf);
    for (int i = 0; i < 256; i++) {
        putn_serial((uint64_t)buf[i]);
    }
    puts_serial("\r\n");

    uint16_t buf1[512];
    for (int i = 0; i < 512; i++) {
        buf1[i] = 0xabcd;
        //buf1[i] = 0;
    }

    put_port_status(port);
    puts_serial("write start\r\n");
    ahci_write_test(port, 0, 0x0, 1, buf1);
    puts_serial("write end\r\n");

    ahci_read_test(port, 0, 0x0, 1, buf);
    for (int i = 0; i < 256; i++) {
        putn_serial((uint64_t)buf[i]);
    }
    puts_serial("\r\n");

    puts_serial("iroiro end\r\n");
}

