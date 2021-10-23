<!--
SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>

SPDX-License-Identifier: AGPL-3.0-only
-->

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


## agora-airgap

agora-airgap is a security tool used in Agora Voting. With it you can:

- Audit a ballot from the "cast or cancel" procedure
- Create a ballot inside an air gap computer (ideally on an OpenRISC open hardware arquitecture)

## Introduction

agora-airgap includes a set of commands:
 
- download-audit: This command needs a file with an auditable ballot as an argument. It will download the public keys and the election data for that voting and then it will audit the ballot.

- download: This command needs a file with an auditable ballot as an argument, as well as the paths where the public keys and election data files will be stored. It will download the public keys and the election data for that voting.

- audit: This command needs a file with an auditable ballot, the public keys file and the election data file as arguments. It will audit the ballot offline, without the need of an internet connection.

- encrypt: This command needs a file with a plaintext ballot, the public keys file and the election data file as arguments. 

Executing the download and audit commands achieve the same result as only executing the single command download-audit, but the strength of separating the steps is that the audit command can be executed in a separate, secure computer that doesn't need to have an internet connection. 

The audit command is useful to audit a ballot from the 'cast or cancel' paradigm. That auditable ballot is obtained from a cancelled vote from that procedure and you are not auditing the votes that are being casted, but the software that creates them: the 'cast or cancel' is a statistical verification procedure.

On the other hand, the encrypt command lets you encrypt a ballot that you can cast afterwards and it lets you encrypt that ballot in a computer without internet connection, on your own trustworthy airgap computer. While the security scheme of Agora Voting is quite strong, in the end the election authorities cannot control the end-user computer. By encrypting the ballot on an airgap computer the risks are minimized, and the aim here is to ease the task of encrypting the vote in a safe environment, but it is your responsability to ensure that the computer you are using to encrypt your vote is not compromised.

In the future we will help in that task by providing an easy guide to audit or encrypt ballots on an airgap OpenRSIC computer, with both a free hardware and free software architecture.

## Compilation

You need to install gmp-devel gcc-c++ git premake4 libcurl-devel. Then download and get the source code:

    git clone https://github.com/miloyip/rapidjson
    git clone https://github.com/agoravoting/agora-airgap.git
    cp -r rapidjson/include/rapidjson agora-airgap/src

And compile:

    cd agora-airgap/src
    make

and now you have your binary in x64/agora-airgap.

## Audit example

Once you have compiled agora-airgap, the fastest way to do a test is simply executing this command from the agora-airgap/src folder:

    make run
    
This will audit an example ballot that is stored in the agora-airgap/src/example folder, now we'll analyze thoroughly what is going on and achieve the same thing using the underlying commands.

In order to audit a ballot we first need an auditable ballot and in this case that is the file agora-airgap/src/example/ballot.json . That file stores the urls to the public key file and to the election data file. So firt we need to have an http server to serve the files when someone requests those urls. To start a very basic http server, execute this command from the agora-airgap/src/example/ folder:

    python3 -m http.server
    
Now we can audit the ballot. On another console, execute the following command from the agora-airgap/src/x64 folder:

    ./agora-airgap download-audit ../example/ballot.json
    
As we mentioned earlier, you can also do the audit procedure in two steps. First to download the public key files, execute the following from the agora-airgap/src/x64.

    ./agora-airgap download ../example/ballot.json election_data.file
    
Note that for that command to work you need to still be running the python http server and you also need the files pk.file and election_data.file not to exist.

Afterwards, you can run the following command without the need of having the http server running:

    ./agora-airgap audit ../example/ballot.json election_data.file

## Encrypt example

In this case we will encrypt a plaintext ballot without the need of an internet connection. For that we need the public keys and the plaintext ballot files. The following command will encrypt the ballot and save it into the file encrypted_ballot.json. Execute it from the agora-airgap/src/x64 folder:

    ./agora-airgap encrypt ../example/votes.json ../example/pk_1 encrypted_ballot.json

## Graphic interface

There is a graphic interface for the audit procedure, available for linux 64 bits. Compile it executing the following command from the agora-airgap/src folder:

    make gui
    
The audit graphic interface will be available in agora-airgap/src/x64/interface . If you want to move the program somewhere else don't forget to also copy/move the picture agora-airgap/src/x64/screen.png to the same folder.

