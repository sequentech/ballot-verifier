SHELL := /bin/bash
myvar := "jiji"
INSTALLDIR := $(CURDIR)
LOGPATH := $(CURDIR)/log
SYSROOT := /opt/or1k-toolchain/or1k-linux-uclibc/sys-root

all: log or1ksim newlib uclibc linux
log:
	[ -f $(LOGPATH) ] && { echo "" > $(LOGPATH); } || { touch $(LOGPATH); }
	echo "log file created" >> $(LOGPATH)
or1ksim: 
	[ -f $(LOGPATH) ] || { touch $(LOGPATH); }
	type svn >/dev/null 2>&1 || { echo >&2 "Fail! subversion not found, now installing svn..."; sudo apt-get install subversion; }
	type git >/dev/null 2>&1 || { echo >&2 "Fail! git not found, now installing  git..."; sudo apt-get install git; }
	echo "getting the svn of or1ksim" >> $(LOGPATH)
	cd $(INSTALLDIR) && svn co http://opencores.org/ocsvn/openrisc/openrisc/trunk/or1ksim
	[ -d $(INSTALLDIR)/or1ksim/builddir_or1ksim ] || { mkdir $(INSTALLDIR)/or1ksim/builddir_or1ksim; }
	cd $(INSTALLDIR)/or1ksim/builddir_or1ksim && ../configure --target=or32-elf --prefix=/opt/or1ksim
	cd $(INSTALLDIR)/or1ksim/builddir_or1ksim && $(MAKE) all
	echo "Please enter your password to install the or1ksim OpenRISC simulator"
	echo "installing or1ksim" >> $(LOGPATH)
	cd $(INSTALLDIR)/or1ksim/builddir_or1ksim && sudo $(MAKE) install
	echo "installed!!" >> $(LOGPATH)
	grep -Fxq "export PATH=/opt/or1ksim/bin:\$$PATH" $$HOME/.bashrc || { \
		echo "export PATH=/opt/or1ksim/bin:\$$PATH" >> $$HOME/.bashrc; \
	}

newlib:
	[ -f $(LOGPATH) ] || { touch $(LOGPATH); }
	#Newlib toolchain (or1k-elf) 
	echo "Newlib toolchain (or1k-elf)" >>  $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-src.git 2>/dev/null; true; } 
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-gcc.git 2>/dev/null; true; }
	[ -d $(INSTALLDIR)/bld-or1k-src ] || { mkdir $(INSTALLDIR)/bld-or1k-src; }
	[ -d $(INSTALLDIR)/bld-or1k-gcc ] || { mkdir $(INSTALLDIR)/bld-or1k-gcc; }
	grep -Fxq "export PATH=/opt/or1k-toolchain/bin:\$$PATH" $$HOME/.bashrc || { \
		echo "export PATH=/opt/or1k-toolchain/bin:\$$PATH" >> $$HOME/.bashrc; \
	}
	[ -d /opt/or1k-toolchain ] || { sudo mkdir /opt/or1k-toolchain; }
	sudo chown $(shell whoami):$(shell id -g -n $(shell whoami)) -R /opt/or1k-toolchain
	#Build the first set of tools, binutils etc (for Newlib toolchain)
	echo "Build the first set of tools, binutils etc (for Newlib toolchain)" >> $(LOGPATH)  
	sudo apt-get install libgmp-dev libmpfr-dev libmpc-dev bison flex texinfo libncurses5-dev
	echo "Configure..." >> $(LOGPATH)
	cd $(INSTALLDIR)/bld-or1k-src && ../or1k-src/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-shared \
	--disable-itcl --disable-tk --disable-tcl --disable-winsup --disable-gdbtk --disable-libgui --disable-rda \
	--disable-sid --disable-sim --disable-gdb --with-sysroot --disable-newlib --disable-libgloss
	echo "Building..." >> $(LOGPATH) 
	cd $(INSTALLDIR)/bld-or1k-src && $(MAKE)
	echo "Installing..." >> $(LOGPATH) 
	cd $(INSTALLDIR)/bld-or1k-src && $(MAKE) install
	#Build gcc 
	echo "Build gcc" >> $(LOGPATH)
	cd $(INSTALLDIR)/bld-or1k-gcc && ../or1k-gcc/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-languages=c \
	--disable-shared --disable-libssp
	cd $(INSTALLDIR)/bld-or1k-gcc && $(MAKE)
	cd $(INSTALLDIR)/bld-or1k-gcc && $(MAKE) install
	#Build newlib and gdb (without or1ksim in this case)  (for Newlib toolchain) 
	echo "Build newlib and gdb (without or1ksim in this case)  (for Newlib toolchain) " >> $(LOGPATH)
	rm -rf $(INSTALLDIR)/bld-or1k-src
	mkdir $(INSTALLDIR)/bld-or1k-src  
	cd $(INSTALLDIR)/bld-or1k-src && ../or1k-src/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-shared --disable-itcl \
	--disable-tk --disable-tcl --disable-winsup --disable-gdbtk --disable-libgui --disable-rda --disable-sid \
	--enable-sim --disable-or1ksim \
	--enable-gdb  --with-sysroot --enable-newlib --enable-libgloss
	cd $(INSTALLDIR)/bld-or1k-src && $(MAKE)
	cd $(INSTALLDIR)/bld-or1k-src && $(MAKE) install
	#Build gcc again, this time with newlib (for Newlib toolchain) 
	echo "Build gcc again, this time with newlib (for Newlib toolchain) " >> $(LOGPATH)
	rm -rf $(INSTALLDIR)/bld-or1k-gcc
	mkdir $(INSTALLDIR)/bld-or1k-gcc
	cd $(INSTALLDIR)/bld-or1k-gcc
	cd $(INSTALLDIR)/bld-or1k-gcc && ../or1k-gcc/configure --target=or1k-elf --prefix=/opt/or1k-toolchain \
	--enable-languages=c,c++ --disable-shared --disable-libssp --with-newlib
	cd $(INSTALLDIR)/bld-or1k-gcc && $(MAKE)
	cd $(INSTALLDIR)/bld-or1k-gcc && $(MAKE) install

uclibc:
	[ -f $(LOGPATH) ] || { touch $(LOGPATH); }
	#Linux (uClibc) toolchain (or1k-linux-uclibc) 
	echo "Linux (uClibc) toolchain (or1k-linux-uclibc)" >> $(LOGPATH)
	[ -d /opt/or1k-toolchain ] || sudo mkdir /opt/or1k-toolchain
	sudo chown $(shell whoami):$(shell id -g -n $(shell whoami)) -R  /opt/or1k-toolchain
	grep -Fxq "PATH=\$$PATH:/opt/or1k-toolchain/bin" $$HOME/.bashrc || { \
		echo "PATH=\$$PATH:/opt/or1k-toolchain/bin" >> $$HOME/.bashrc; \
	}
	#unset LD_LIBRARY_PATH
	#or1k-src 
	#NOTE: on 32-bit machines --disable-werror is needed due to an enum acting as bit mask is considered signed 
	echo "or1k-src" >> $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-src.git; true; }
	[ -d $(INSTALLDIR)/build-or1k-src ] || mkdir $(INSTALLDIR)/build-or1k-src
	cd $(INSTALLDIR)/build-or1k-src && ../or1k-src/configure --target=or1k-linux-uclibc --prefix=/opt/or1k-toolchain \
	--disable-shared --disable-itcl --disable-tk --disable-tcl --disable-winsup \
	--disable-libgui --disable-rda --disable-sid --disable-sim --disable-gdb \
	--with-sysroot --disable-newlib --disable-libgloss
	cd $(INSTALLDIR)/build-or1k-src && $(MAKE)
	cd $(INSTALLDIR)/build-or1k-src && $(MAKE) install
	#Linux headers
	echo "Linux headers" >> $(LOGPATH)
	grep -Fxq "SYSROOT=/opt/or1k-toolchain/or1k-linux-uclibc/sys-root" $$HOME/.bashrc || { \
		echo "SYSROOT=/opt/or1k-toolchain/or1k-linux-uclibc/sys-root" >> $$HOME/.bashrc; \
	}
	cd $(INSTALLDIR) && { git clone git://openrisc.net/jonas/linux; true; }
	cd $(INSTALLDIR)/linux && $(MAKE) INSTALL_HDR_PATH=${SYSROOT}/usr headers_install
	#gcc stage 1
	echo "gcc stage 1" >> $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-gcc 2>/dev/null; true; }
	[ -d $(INSTALLDIR)/build-gcc ] || mkdir $(INSTALLDIR)/build-gcc
	cd $(INSTALLDIR)/build-gcc && ../or1k-gcc/configure --target=or1k-linux-uclibc --prefix=/opt/or1k-toolchain --disable-libssp \
	--srcdir=../or1k-gcc --enable-languages=c --without-headers --enable-threads=single \
	--disable-libgomp --disable-libmudflap --disable-shared --disable-libquadmath \
	--disable-libatomic
	cd $(INSTALLDIR)/build-gcc && $(MAKE)
	cd $(INSTALLDIR)/build-gcc && $(MAKE) install
	#uClibc
	echo "uClibc" >> $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/uClibc-or1k.git 2>/dev/null; true; }
	cd $(INSTALLDIR)/uClibc-or1k && $(MAKE) ARCH=or1k defconfig
	cd $(INSTALLDIR)/uClibc-or1k && $(MAKE)
	cd $(INSTALLDIR)/uClibc-or1k && $(MAKE) PREFIX=${SYSROOT} install
	#gcc stage 2
	echo "gcc stage 2" >> $(LOGPATH)
	cd $(INSTALLDIR)/build-gcc && rm -rf *
	cd $(INSTALLDIR)/build-gcc && ../or1k-gcc/configure --target=or1k-linux-uclibc --prefix=/opt/or1k-toolchain --disable-libssp \
	--srcdir=../or1k-gcc --enable-languages=c,c++ --enable-threads=posix --disable-libgomp \
	--disable-libmudflap --with-sysroot=${SYSROOT} --disable-multilib
	cd $(INSTALLDIR)/build-gcc && $(MAKE)
	cd $(INSTALLDIR)/build-gcc && $(MAKE) install

linux:
	[ -f $(LOGPATH) ] || { touch $(LOGPATH); }
	#linux
	echo "linux" >> $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-src.git; true; }
	sed -i "s/elf32-or32/elf32-or1k/g" $(INSTALLDIR)/linux/arch/openrisc/kernel/vmlinux.lds.S
	cd $(INSTALLDIR)/linux && $(MAKE) ARCH=openrisc defconfig
	cd $(INSTALLDIR)/linux && $(MAKE) menuconfig
	cd $(INSTALLDIR)/linux && $(MAKE) CROSS_COMPILER=or1k-elf-

runlinux:
	cd $(INSTALLDIR)/linux && or32-elf-sim -f arch/openrisc/or1ksim.cfg vmlinux

.PHONY: clean or1ksim log uclibc linux runlinux

clean:
	#clean 
	[ -f $(INSTALLDIR)/log ] && rm $(INSTALLDIR)/log 
	[ -d $(INSTALLDIR)/linux ] && rm -rf $(INSTALLDIR)/linux
	[ -d $(INSTALLDIR)/build-gcc ] && rm -rf $(INSTALLDIR)/build-gcc
	[ -d $(INSTALLDIR)/uClibc-or1k ] && rm -rf $(INSTALLDIR)/uClibc-or1k
	[ -d $(INSTALLDIR)/build-or1k-src ] && rm -rf $(INSTALLDIR)/build-or1k-src
	[ -d $(INSTALLDIR)/bld-or1k-gcc ] && rm -rf $(INSTALLDIR)/bld-or1k-gcc
	[ -d $(INSTALLDIR)/bld-or1k-src ] && rm -rf $(INSTALLDIR)/bld-or1k-src
	[ -d $(INSTALLDIR)/or1ksim ] && rm -rf $(INSTALLDIR)/or1ksim
	[ -d $(INSTALLDIR)/or1k-gcc ] && rm -rf $(INSTALLDIR)/or1k-gcc
	[ -d $(INSTALLDIR)/or1k-src ] && rm -rf $(INSTALLDIR)/or1k-src
