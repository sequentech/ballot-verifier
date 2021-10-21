## agora-airgap

agora-audit is a tool that allows you to audit a spoiled ballot from Agora Voting. This software implements the 'cast or cancel' procedure described on the paper "Ballot Casting Assurance via Voter-Initiated Poll Station Auditing" by Josh Benaloh. The paper can be found on:

https://www.usenix.org/legacy/event/evt07/tech/full_papers/benaloh/benaloh.pdf

This software has been tested both on Windows 64 bits and Linux 64 bits systems

The Windows 64 bits version can be downloaded here (note, outdated!):

https://github.com/agoravoting/agora-airgap/releases/download/0.50/agora-audit-windows.zip

The Linux 64 bits version can be downloaded here:

https://github.com/agoravoting/agora-airgap/releases/download/3.4.0/agora-audit-linux-3.4.0.tar.gz

Download and extract the files and execute agora-audit.

The agora-audit tool has a textbox on the upper left side where you should copy the ballot. Before you cast your vote in Agora Voting, you are allowed to audit the ballot (this also discards the ballot for security reasons). The upper right side of agora-audit shows you a screen capture of the audit ballot screen and marks the place where you will find the auditable ballot enclosed with a red box.

Once you have copied and pasted the auditable ballot to agora-audit, you should click the "Verify Ballot" button. If the ballot is verified, the state indicator below should change to State: VERIFIED. There is also a console below the Details label that shows more information.

## Compilation

As an alternative to just downloading the pre-compiled tool, you can compile it yourself from the source code. 

First you'll need to install some libraries:

    sudo apt-get install m4 libgtk-3-dev libcurl4-openssl-dev libcurl3-gnutls libssl-dev

Now we will need the static version of the GMP library.

    wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.bz2
    tar xjf gmp-6.1.2.tar.bz2
    cd gmp-6.1.2
    ./configure --enable-static --enable-cxx
    make && make check
    sudo make install


To install the static libraries of the wxWidgets library, download the latest stable release from the following url and then execute the commands.

http://sourceforge.net/projects/wxwindows/files/3.0.2/wxWidgets-3.0.2.tar.bz2/download

    tar xjf wxWidgets-3.0.2.tar.bz2
    cd wxWidgets-3.0.2
    ./configure --disable-shared
    make
    sudo make install

Download the idn library:

    wget http://ftp.gnu.org/gnu/libidn/libidn-1.33.tar.gz
    tar xzf libidn-1.33.tar.gz
    cd libidn-1.33
    ./configure
    make
    sudo make install
    
Download and install the static CUrl library:

    wget https://curl.haxx.se/download/curl-7.52.1.tar.gz
    tar xzf curl-7.52.1.tar.gz
    cd curl-7.52.1
    ./configure  --disable-ldap --without-librtmp --with-ssl
    make
    sudo make install

Download the source code by executing the following command:

    git clone https://github.com/agoravoting/agora-airgap.git
    
Download the rapidjson library and copy the includes to the agora-airgap/src folder:

    git clone https://github.com/miloyip/rapidjson.git
    cp -Rf rapidjson/include/rapidjson/ agora-airgap/src

Go to the agora-airgap/src folder and execute:

    cd agora-airgap/src
    make gui
    
If the build is successful, you will find the agora-audit tool on agora-airgap/src/x64/agora-audit

As a side note, the file screen.h includes the PNG agora-airgap/src/screen.png file. The file screen.h has been generated with the bin2c tool available at https://github.com/gwilymk/bin2c

## Windows compilation

Now we will need the static version of the GMP library.

    wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.bz2
    tar xjf gmp-6.1.2.tar.bz2
    cd gmp-6.1.2
    ./configure --host=x86_64-w64-mingw64 --enable-static --enable-cxx
    make && make check
    sudo make install

To install the static libraries of the wxWidgets library, download the latest stable release from the following url and then execute the commands.

http://sourceforge.net/projects/wxwindows/files/3.0.2/wxWidgets-3.0.2.tar.bz2/download

    tar xjf wxWidgets-3.0.2.tar.bz2
    cd wxWidgets-3.0.2
    ./configure --host=x86_64-w64-mingw32 --disable-shared
    make
    sudo make install

Download the idn library:

    wget https://ftp.gnu.org/gnu/libidn/libidn-1.33.tar.gz
    tar xzf libidn-1.33.tar.gz
    cd libidn-1.33
    ./configure --host=x86_64-w64-mingw64 --enable-static --disable-shared
    make
    sudo make install

Download and install OpenSSL.

    wget https://www.openssl.org/source/openssl-1.0.2k.tar.gz
    tar xzf openssl-1.0.1j.tar.gz && cd openssl-1.0.2k
    export CROSS_COMPILE="x86_64-w64-mingw32-"
    ./Configure mingw64 no-asm no-shared
    make depend
    make
    sudo make install

Download and install the static CUrl library:

    wget https://curl.haxx.se/download/curl-7.52.1.tar.gz
    tar xzf curl-7.52.1.tar.gz
    cd curl-7.52.1
    export LDFLAGS="-L/usr/local/ssl"
    export CPPFLAGS="-I/usr/local/ssl/include"
    ./configure  --host=x86_64-w64-mingw64 --disable-shared --disable-ldap --without-librtmp --with-ssl=/usr/local/ssl
    make
    sudo make install

Download the agora-airgap source code by executing the following command:

    git clone https://github.com/agoravoting/agora-airgap.git

Download the rapidjson library and copy the includes to the agora-airgap/src folder:

    git clone https://github.com/miloyip/rapidjson.git
    cp -Rf rapidjson/include/rapidjson/ agora-airgap/src

Go to the agora-airgap/src folder and execute:

    cd agora-airgap/src
    make xcompile

If the build is successful, you will find the agora-audit tool on agora-airgap/src/w64/agora-audit.exe

As a side note, the file screen.h includes the PNG agora-airgap/src/screen.png file. The file screen.h has been generated with the bin2c tool available at https://github.com/gwilymk/bin2c



