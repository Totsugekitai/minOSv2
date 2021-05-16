.RECIPEPREFIX = >
.PHONY: default clean

SHELL=/bin/bash
ROOTDIR = $(CURDIR)/..
EDK_WORKSPACE = $(ROOTDIR)/../edk2

default:
> echo $(EDK_WORKSPACE)
> cp $(ROOTDIR)/common/* $(ROOTDIR)/boot/MinLoaderPkg/Application/MinLoader/boot
> cp $(ROOTDIR)/boot/src/* $(ROOTDIR)/boot/MinLoaderPkg/Application/MinLoader/boot
> ln -sf $(ROOTDIR)/boot/MinLoaderPkg $(EDK_WORKSPACE)
> cd $(EDK_WORKSPACE); source edksetup.sh --reconfig; \
> build -p MinLoaderPkg/MinLoaderPkg.dsc -b DEBUG -a X64 -t GCC5
