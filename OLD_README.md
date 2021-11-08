<!--
SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>

SPDX-License-Identifier: AGPL-3.0-only
-->

## agora-airgap

agora-airgap is a security tool used in nVotes. With it you can:

- Audit a ballot from the "cast or cancel" procedure
- Create a ballot inside an air gap computer

## Introduction

agora-airgap includes a set of commands:
 
- download-audit: This command needs a file with an auditable ballot as an argument. It will download the public keys and the election data for that voting and then it will audit the ballot.

- download: This command needs a file with an auditable ballot as an argument, as well as the paths where the public keys and election data files will be stored. It will download the public keys and the election data for that voting.

- audit: This command needs a file with an auditable ballot, the public keys file and the election data file as arguments. It will audit the ballot offline, without the need of an internet connection.

- encrypt: This command needs a file with a plaintext ballot, the public keys file and the election data file as arguments. 

Executing the download and audit commands achieve the same result as only executing the single command download-audit, but the strength of separating the steps is that the audit command can be executed in a separate, secure computer that doesn't need to have an internet connection. 

The audit command is useful to audit a ballot from the 'cast or cancel' paradigm. That auditable ballot is obtained from a cancelled vote from that procedure and you are not auditing the votes that are being casted, but the software that creates them: the 'cast or cancel' is a statistical verification procedure.

On the other hand, the encrypt command lets you encrypt a ballot that you can cast afterwards and it lets you encrypt that ballot in a computer without internet connection, on your own trustworthy airgap computer. While the security scheme of nVotes is quite strong, in the end the election authorities cannot control the end-user computer. By encrypting the ballot on an airgap computer the risks are minimized, and the aim here is to ease the task of encrypting the vote in a safe environment, but it is your responsability to ensure that the computer you are using to encrypt your vote is not compromised.

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

In order to audit a ballot we first need an auditable ballot and in this case that is the file agora-airgap/tests/example_1/ballot.json . That file stores the urls to the public key file and to the election data file. So firt we need to have an http server to serve the files when someone requests those urls. To start a very basic http server, execute this command from the agora-airgap/tests/example_1/ folder:

    python3 -m http.server
    
Now we can audit the ballot. On another console, execute the following command from the agora-airgap/ folder:

    ./out/agora-airgap download-audit tests/example_1/ballot.json
    
As we mentioned earlier, you can also do the audit procedure in two steps. First to download the public key files, execute the following from the agora-airgap/ directory.

    ./out/agora-airgap download tests/example_1/ballot.json election_config.json
    
Note that for that command to work you need to still be running the python http server and you also need the files pk.file and election_data.file not to exist.

Afterwards, you can run the following command without the need of having the http server running:

    ./out/agora-airgap audit tests/example_1/ballot.json election_config.json

## Encrypt example

In this case we will encrypt a plaintext ballot without the need of an internet connection. For that we need the public keys and the plaintext ballot files. The following command will encrypt the ballot and save it into the file encrypted_ballot.json. Execute it from the agora-airgap/src/x64 folder:

    ./agora-airgap encrypt ../example/votes.json ../example/pk_1 encrypted_ballot.json

## Graphic interface

There is a graphic interface for the audit procedure, available for linux 64 bits. Compile it executing the following command from the agora-airgap/src folder:

    make gui
    
The audit graphic interface will be available in agora-airgap/src/x64/interface . If you want to move the program somewhere else don't forget to also copy/move the picture agora-airgap/src/x64/screen.png to the same folder.


