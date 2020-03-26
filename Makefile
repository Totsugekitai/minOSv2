.PHONY: default boot kernel run debug-log all clean

QEMU	= /home/totsugekitai/workspace/mywork/qemu/build/x86_64-softmmu/qemu-system-x86_64

default:

boot:
	make -C boot/
	cp boot/edk2/Build/MinLoaderPkgX64/RELEASE_GCC5/X64/MinLoaderPkg/Application/MinLoader/MinLoader/OUTPUT/MinLoader.efi ./fs/EFI/BOOT/BOOTX64.EFI

kernel:
	make -C kernel/
	cp kernel/kernel.elf ./fs

run:
	$(QEMU) \
	-drive if=pflash,format=raw,readonly,file=tool/OVMF_CODE.fd \
	-drive if=pflash,format=raw,file=tool/OVMF_VARS.fd \
	fat:rw:fs/ -m 8G \
	-chardev stdio,mux=on,id=com1 \
	-serial chardev:com1 \
	-device ich9-ahci,id=ahci \
	-device ide-drive,drive=sata,bus=ahci.0 \
	-drive if=none,id=sata,file=tool/hdd.img \
	-monitor telnet::1234,server,nowait \
	-gdb tcp::1235 \

debug-log:
	$(QEMU) \
	-drive if=pflash,format=raw,readonly,file=tool/OVMF_CODE.fd \
	-drive if=pflash,format=raw,file=tool/OVMF_VARS.fd \
	fat:rw:fs/ -m 4G \
	-chardev stdio,mux=on,id=com1,logfile=serial_output.log \
	-serial chardev:com1 \
	-device ich9-ahci,id=ahci \
	-device ide-drive,drive=sata,bus=ahci.0 \
	-drive if=none,id=sata,file=tool/hdd.img \
	-monitor telnet::1234,server,nowait \
	-gdb tcp::1235 \
	--trace events=trace.event \

all:
	make boot
	make kernel
	make run

clean:
	make -C boot/ clean
	make -C kernel/ clean
	rm -rf ./edk2/MinLoaderPkg/Application/MinLoader/boot
