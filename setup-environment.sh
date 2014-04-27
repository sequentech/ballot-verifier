#!/bin/bash
# (POSIX shell syntax)


#Setup some environment variables
INSTALLDIR=$(pwd)
type svn >/dev/null 2>&1 || { echo >&2 "Fail! subversion not found, now installing svn..."; sudo apt-get install subversion;}
type git >/dev/null 2>&1 || { echo >&2 "Fail! git not found, now installing  git..."; sudo apt-get install git;}
svn co http://opencores.org/ocsvn/openrisc/openrisc/trunk/or1ksim
echo "Finished svn co"
cd or1ksim
mkdir builddir_or1ksim
cd builddir_or1ksim
../configure --target=or32-elf --prefix=/opt/or1ksim
make all
echo "Please enter your password to install the or1ksim OpenRISC simulator"
sudo make install
export PATH=/opt/or1ksim/bin:$PATH
grep -Fxq "export PATH=/opt/or1ksim/bin:\$PATH" $HOME/.bashrc || {
echo "export PATH=/opt/or1ksim/bin:\$PATH" >> $HOME/.bashrc
source $HOME/.bashrc
}
cd $INSTALLDIR

exit

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
