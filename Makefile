.PHONY: default boot kernel run all clean

default:

boot:
	make -C boot/
	cp boot/edk2/Build/MinLoaderPkgX64/RELEASE_GCC5/X64/MinLoaderPkg/Application/MinLoader/MinLoader/OUTPUT/MinLoader.efi ./fs/EFI/BOOT/BOOTX64.EFI

kernel:
	make -C kernel/
	cp kernel/kernel.elf ./fs

run:
	qemu-system-x86_64 \
	-drive if=pflash,format=raw,readonly,file=tool/OVMF_CODE.fd \
	-drive if=pflash,format=raw,file=tool/OVMF_VARS.fd \
	fat:rw:fs/ -m 4G

all:
	make boot
	make kernel
	make run

clean:
	make -C boot/ clean
	make -C kernel/ clean
	rm -rf ./edk2/MinLoaderPkg/Application/MinLoader/boot
