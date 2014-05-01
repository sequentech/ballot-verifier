#!/bin/bash
# (POSIX shell syntax)

#Create log
[ -f log ] && { echo "" > log; } || { touch log; }

#Setup some environment variables
INSTALLDIR=$(pwd)
echo "Setup some environment variables" >> $INSTALLDIR/log

#Check if some needed programs are installed and install them if needed (will ask for password)
type svn >/dev/null 2>&1 || { echo >&2 "Fail! subversion not found, now installing svn..."; sudo apt-get install subversion; }
type git >/dev/null 2>&1 || { echo >&2 "Fail! git not found, now installing  git..."; sudo apt-get install git; }
echo "" >> $INSTALLDIR/log

#or1ksim
echo "getting the svn of or1ksim" >> $INSTALLDIR/log
svn co http://opencores.org/ocsvn/openrisc/openrisc/trunk/or1ksim
cd or1ksim
mkdir builddir_or1ksim
cd builddir_or1ksim
../configure --target=or32-elf --prefix=/opt/or1ksim
make all
echo "Please enter your password to install the or1ksim OpenRISC simulator"
echo "installing or1ksim" >> $INSTALLDIR/log
sudo make install
export PATH=/opt/or1ksim/bin:$PATH
grep -Fxq "export PATH=/opt/or1ksim/bin:\$PATH" $HOME/.bashrc || {
echo "export PATH=/opt/or1ksim/bin:\$PATH" >> $HOME/.bashrc
source $HOME/.bashrc
}
cd $INSTALLDIR

#Newlib toolchain (or1k-elf) 
echo "Newlib toolchain (or1k-elf)" >> $INSTALLDIR/log
git clone git://github.com/openrisc/or1k-src.git
git clone git://github.com/openrisc/or1k-gcc.git
[ -d $INSTALLDIR/bld-or1k-src ] || { mkdir bld-or1k-src; }
[ -d $INSTALLDIR/bld-or1k-gcc ] || { mkdir bld-or1k-gcc; }
grep -Fxq "export PATH=/opt/or1k-toolchain/bin:\$PATH" $HOME/.bashrc || {
echo "export PATH=/opt/or1k-toolchain/bin:\$PATH" >> $HOME/.bashrc
source $HOME/.bashrc
}
[ -d /opt/or1k-toolchain ] || { sudo mkdir /opt/or1k-toolchain; }
sudo chown $(whoami):$(id -g -n $(whoami)) -R /opt/or1k-toolchain

#Build the first set of tools, binutils etc (for Newlib toolchain)
echo "Build the first set of tools, binutils etc (for Newlib toolchain)" >> $INSTALLDIR/log 
sudo apt-get install libgmp-dev libmpfr-dev libmpc-dev bison flex texinfo libncurses5-dev
cd bld-or1k-src
echo "Configure..." >> $INSTALLDIR/log 
../or1k-src/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-shared \
--disable-itcl --disable-tk --disable-tcl --disable-winsup --disable-gdbtk --disable-libgui --disable-rda \
--disable-sid --disable-sim --disable-gdb --with-sysroot --disable-newlib --disable-libgloss
echo "Building..." >> $INSTALLDIR/log 
make
echo "Installing..." >> $INSTALLDIR/log 
make install

#Build gcc 
echo "Build gcc" >> $INSTALLDIR/log
cd ../bld-or1k-gcc
../or1k-gcc/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-languages=c \
--disable-shared --disable-libssp
make
make install

#Build newlib and gdb (without or1ksim in this case)  (for Newlib toolchain) 
echo "Build newlib and gdb (without or1ksim in this case)  (for Newlib toolchain) " >> $INSTALLDIR/log
cd ..
rm -rf bld-or1k-src
mkdir bld-or1k-src  
cd bld-or1k-src
../or1k-src/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-shared --disable-itcl \
--disable-tk --disable-tcl --disable-winsup --disable-gdbtk --disable-libgui --disable-rda --disable-sid \
--enable-sim --disable-or1ksim \
--enable-gdb  --with-sysroot --enable-newlib --enable-libgloss
make
make install

#Build gcc again, this time with newlib (for Newlib toolchain) 
echo "Build gcc again, this time with newlib (for Newlib toolchain) " >> $INSTALLDIR/log
cd ..
rm -rf bld-or1k-gcc
mkdir bld-or1k-gcc
cd bld-or1k-gcc

../or1k-gcc/configure --target=or1k-elf --prefix=/opt/or1k-toolchain --enable-languages=c,c++ --disable-shared \
--disable-libssp --with-newlib
make
make install


echo "SUCCESSS! " >> $INSTALLDIR/log

#Linux (uClibc) toolchain (or1k-linux-uclibc) 

	

exit


#this is the skeleton of what will probably used in the future to control options in this script, but for now it's not used at all.
# Usage info
show_help() {
echo "Usage: ${0##*/} [-hv] [-f OUTFILE] [FILE]...
Do stuff with FILE and write the result to standard output. With no FILE
or when FILE is -, read standard input.
     
     -h          display this help and exit
     -f OUTFILE  write the result to OUTFILE instead of standard output.
     -v
          verbose mode. Can be used multiple times for increased
                 verbosity."
 }                
 
# Initialize our own variables:
 output_file=""
 verbose=0
 
 OPTIND=1 # Reset is necessary if getopts was used previously in the script.  It is a good idea to make this local in a function.
 while getopts "hvf:" opt; do
    case "$opt" in
         h)
             show_help
             exit 0
             ;;
         v)  verbose=1
             ;;
          f)  output_file=$OPTARG
             ;;
         '?')
             show_help >&2
             exit 1
             ;;
     esac
 done
 shift "$((OPTIND-1))" # Shift off the options and optional --.
 
 printf 'verbose=<%s>\noutput_file=<%s>\nLeftovers:\n' "$verbose" "$output_file"
 printf '<%s>\n' "$@"
 
 # End of file
