SHELL := /bin/bash
INSTALLDIR := $(CURDIR)
LOGPATH := $(CURDIR)/log
SYSROOT := /opt/or1k-toolchain/or1k-linux-uclibc/sys-root
LINUX_COMPILER := or1k-linux-uclibc-g++
METAL_COMPILER := or1k-elf-g++
X86_COMPILER := g++

all: log or1ksim newlib uclibc agora-airgap linux
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

agora-airgap:
	@echo 'Building target: $@'
	[ -d $(INSTALLDIR)/Debug ] || mkdir $(INSTALLDIR)/Debug 
	cd $(INSTALLDIR)/Debug && $(LINUX_COMPILER) -c ../src/sha256.cpp ../src/Random.cpp ../src/ElGamal.cpp \
		../src/Agora.cpp ../src/agora-airgap.cpp
	@echo 'Invoking: GCC C++ Linker'
	#g++  -o "has.cpp" $(OBJS) $(USER_OBJS) $(LIBS)
  	#This won't work yet because you have to use the cross-compiled version of gmp!!
	cd $(INSTALLDIR)/Debug && $(LINUX_COMPILER) -static -o "agora-airgap" sha256.o Random.o ElGamal.o Agora.o agora-airgap.o -lgmp
	@echo 'Finished building target: $@'
	[ -d $(INSTALLDIR)/linux/arch/openrisc/support/initramfs/usr/local ] || \
		mkdir $(INSTALLDIR)/linux/arch/openrisc/support/initramfs/usr/local 
	cp $(INSTALLDIR)/Debug/agora-airgap $(INSTALLDIR)/linux/arch/openrisc/support/initramfs/usr/local/

agora-airgap-x86:
	@echo 'Building target: $@'
	[ -d $(INSTALLDIR)/Debug ] || mkdir $(INSTALLDIR)/Debug 
	cd $(INSTALLDIR)/Debug && $(X86_COMPILER) -c ../src/sha256.cpp ../src/Random.cpp ../src/ElGamal.cpp \
		../src/Agora.cpp ../src/agora-airgap.cpp
	@echo 'Invoking: GCC C++ Linker'
	#g++  -o "has.cpp" $(OBJS) $(USER_OBJS) $(LIBS)
	cd $(INSTALLDIR)/Debug && $(X86_COMPILER) -o "agora-airgap" sha256.o Random.o ElGamal.o Agora.o agora-airgap.o -lgmp
	@echo 'Finished building target: $@'
	 
linux:
	[ -f $(LOGPATH) ] || { touch $(LOGPATH); }
	#linux
	echo "linux" >> $(LOGPATH)
	cd $(INSTALLDIR) && { git clone git://github.com/openrisc/or1k-src.git; true; }
	sed -i "s/elf32-or32/elf32-or1k/g" $(INSTALLDIR)/linux/arch/openrisc/kernel/vmlinux.lds.S
	cd $(INSTALLDIR)/linux && $(MAKE) ARCH=openrisc defconfig
	#cd $(INSTALLDIR)/linux && $(MAKE) menuconfig
	grep -Fq "CONFIG_CROSS_COMPILE=\"or32-linux-\"" $(INSTALLDIR)/linux/.config &&  \
		sed -i "s/CONFIG_CROSS_COMPILE=\"or32-linux-\"/CONFIG_CROSS_COMPILE=\"or1k-elf-\"/g" $(INSTALLDIR)/linux/.config || true;
	grep -Fq "ifconfig eth0 192.168.1.100 &" $(INSTALLDIR)/linux/arch/openrisc/support/initramfs/etc/init.d/rcS &&  \
		sed -i "s/ifconfig eth0 192.168.1.100 \&/ifconfig eth0 10.0.2.16 netmask 255.255.255.0 \&/g" \
			$(INSTALLDIR)/linux/arch/openrisc/support/initramfs/etc/init.d/rcS || true;
	cd $(INSTALLDIR)/linux && $(MAKE) CROSS_COMPILER=or1k-elf-

runlinux:
	type openvpn >/dev/null 2>&1 || { echo >&2 "Fail! openvpn not found, now installing  openvpn..."; sudo apt-get install openvpn; }
	type brctl >/dev/null 2>&1 || { echo >&2 "Fail! bridge-utils not found, now installing  bridge-utils..."; \
		sudo apt-get install bridge-utils; }
	ifconfig | grep -Fq "br0" || { \
		cd $(INSTALLDIR)/or1ksim && sudo ./brstart.sh openrisc openrisc br0 eth0 tap0; \
	}
	cd $(INSTALLDIR)/linux && or32-elf-sim -f arch/openrisc/or1ksim.cfg vmlinux 
	#{ sleep 7 && echo "Now connect to openrisc using the following command (user: root) telnet 10.0.2.16"; }

tapdown:
	ifconfig | grep -Fq "br0" && { cd $(INSTALLDIR)/or1ksim && sudo ./brend.sh br0 eth0 tap0; } || true;
	sudo ifconfig eth0 down
	sleep 6	
	sudo ifconfig eth0 up 
	

pruebas:
	false || { echo >&2 "Fail! bridge-utils not found, now installing  bridge-utils..."; \
		sudo apt-get install bridge-utils; }


.PHONY: clean or1ksim log uclibc agora-airgap linux runlinux pruebas tapdown

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
