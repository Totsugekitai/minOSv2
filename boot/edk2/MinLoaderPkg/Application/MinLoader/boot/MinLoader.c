#include "MinLoader.h"

static uint64_t *update_start_addr(uint64_t *start_addr)
{
    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)start_addr;
    Elf64_Shdr *section_header = (Elf64_Shdr *)((char *)elf_header + elf_header->e_shoff);
    return (uint64_t *)((char *)start_addr + (int)section_header[1].sh_offset);
}

static inline void relocate_kernel(uint64_t *start_addr, uint64_t *updated_start_addr, UINTN file_size)
{
    char *saddr = (char *)start_addr;
    char *usaddr = (char *)updated_start_addr;
    char *diff = usaddr - saddr;
    char *correct = saddr - diff;
    gBS->CopyMem(correct, start_addr, file_size);
}

EFI_STATUS
EFIAPI
UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    bootinfo_t binfo;
    EFI_STATUS status;

    // GraphicsOutputProtocolを取得
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    do {
        status = gBS->LocateProtocol(&gop_guid, NULL, (void **)&gop);
    } while (EFI_ERROR(status));

    // カーネルに渡す引数を設定
    // video_infoを詰める
    binfo.vinfo.fb = (uint64_t *)gop->Mode->FrameBufferBase;
    binfo.vinfo.fb_size = (uint64_t)gop->Mode->FrameBufferSize;
    binfo.vinfo.x_axis = (uint32_t)gop->Mode->Info->HorizontalResolution;
    binfo.vinfo.y_axis = (uint32_t)gop->Mode->Info->VerticalResolution;
    binfo.vinfo.ppsl = (uint32_t)gop->Mode->Info->PixelsPerScanLine;

    // SympleFileSystemProtocolを取得
    EFI_GUID sfsp_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp;
    do {
    status = gBS->LocateProtocol(&sfsp_guid, NULL, (void **)&sfsp);
    } while(EFI_ERROR(status));

    // rootディレクトリを開く
    EFI_FILE_PROTOCOL *root;
    do {
        sfsp->OpenVolume(sfsp, &root);
    } while(EFI_ERROR(status));

    // カーネルのファイルを開く
    EFI_FILE_PROTOCOL *kernel_file;
    CHAR16 *file_name = (CHAR16 *)KERNEL_FILE_NAME;
    UINT64 file_mode = (UINT64)EFI_FILE_READ_ONLY;
    do {
        status = root->Open(root, &kernel_file, file_name, file_mode, 0);
    } while(EFI_ERROR(status));

    // カーネルのファイルサイズを取得
    EFI_FILE_INFO file_info;
    EFI_GUID fi_guid = EFI_FILE_INFO_ID;
    UINTN buf_size = BUF_256B;
    do {
        status = kernel_file->GetInfo(kernel_file, &fi_guid, &buf_size, &file_info);
    } while(EFI_ERROR(status));
    UINTN file_size = file_info.FileSize;

    // カーネルファイルをメモリに読み込む
    uint64_t *kernel_program = NULL;
    do {
        status = kernel_file->Read(kernel_file, &file_size, kernel_program);
    } while(EFI_ERROR(status));

    // bodyをメモリに書き込む
    uint64_t *start_addr = KERNEL_START_QEMU;
    gBS->CopyMem(start_addr, kernel_program, file_size);

    // update start address
    uint64_t *updated_start_addr = update_start_addr(start_addr);

    // relocate kernel
    relocate_kernel(start_addr, updated_start_addr, file_size);

    // メモリマップ取得のための変数
    UINTN mmapsize = 0, mapkey, descsize;
    UINT32 descver;
    EFI_MEMORY_DESCRIPTOR *mmap = NULL;
    // ExitBootService()の処理を開始
    do {
        status = gBS->GetMemoryMap(&mmapsize, mmap, &mapkey, &descsize, &descver);
        while (status == EFI_BUFFER_TOO_SMALL) {
            if (mmap) {
                gBS->FreePool(mmap);
            }
            mmapsize += 0x1000;
            // メモリマップの領域を確保
            status = gBS->AllocatePool(EfiLoaderData, mmapsize, (void **)&mmap);
            // メモリマップを取得
            status = gBS->GetMemoryMap(&mmapsize, mmap, &mapkey, &descsize, &descver);
        }
        // ExitBootServices
        status = gBS->ExitBootServices(ImageHandle, mapkey);
    } while (EFI_ERROR(status));

    // カーネルに渡す情報をレジスタに格納
    // スタックポインタを設定しカーネルへジャンプ
    jump_to_kernel(&binfo, start_addr);
    //jump_to_kernel(&binfo, updated_start_addr);

    return EFI_SUCCESS;
}

