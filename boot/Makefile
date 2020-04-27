.RECIPEPREFIX = >
.PHONY: default clean

ROOTDIR = $(CURDIR)/..
EDK_WORKSPACE = $(WORKSPACE)

default:
> echo $(EDK_WORKSPACE)
ifeq ($(EDK_WORKSPACE),)
> echo "$(error EDK PATH not set)"
else
> cp $(ROOTDIR)/common/* $(ROOTDIR)/boot/MinLoaderPkg/Application/MinLoader/boot
> cp $(ROOTDIR)/boot/src/* $(ROOTDIR)/boot/MinLoaderPkg/Application/MinLoader/boot
> ln -sf $(ROOTDIR)/boot/MinLoaderPkg $(EDK_WORKSPACE)
> cd $(EDK_WORKSPACE); build
endif
