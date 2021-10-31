<!--
SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>

SPDX-License-Identifier: AGPL-3.0-only
-->

# agora-airgap

agora-airgap is nVotes cast-as-intended verifier. It allows a voter to audit a
spoiled ballot.

This software implements the 'cast or cancel' procedure described on the paper
[Ballot Casting Assurance via Voter-Initiated Poll Station Auditing] by Josh
Benaloh.

## Install

agora-airgap works in Linux and Mac OS X 64 bits systems. You can download the
latest compiled version here:

- [Linux binary]
- [Mac OS X binary]

## How to use

The agora-airgap tool has a textbox on the upper left side where you should copy
the ballot. Before you cast your vote in nVotes voting booth, you are allowed to
audit the ballot. Note that this also discards the ballot for security reasons.
The upper right side of agora-airgap shows you a screen capture of the audit
ballot screen and marks the place where you will find the auditable ballot
enclosed with a red box.

Once you have copied and pasted the auditable ballot to agora-airgap, you should
click the **Verify Ballot** button. If the ballot is verified, the state
indicator below should change to State: VERIFIED. There is also a console below
the Details label that shows more information.

## Compiling

As an alternative to just downloading the pre-compiled tool, you can compile it
yourself from the source code. 

agora-airgap uses [Nix Package Manager] as its package builder. To build 
agora-airgap, **first [install Nix]** correctly in your system.

After you have installed Nix, you can build agora-airgap executing the following
command in the agora-airgap main directory:

```bash
nix build -L
```

After a succesful build, you can find the built `agora-airgap` binary in the
`result/bin/agora-airgap` output directory.

```bash
$ ls -lah result/bin/  
total 1176
dr-xr-xr-x  4 root  wheel   128B Jan  1  1970 .
dr-xr-xr-x  3 root  wheel    96B Jan  1  1970 ..
-r-xr-xr-x  1 root  wheel   227K Jan  1  1970 agora-airgap
-r-xr-xr-x  1 root  wheel   356K Jan  1  1970 agora-airgap-gui
```

You can just execute this binary from the command line with:

```bash
./result/bin/agora-airgap
```

# Contributing

Contributions are welcome! We'd love to include your improvements to our
project. Please make sure you:
- Sign the [Contributor License Agreement].
- All the tests in the [Contin uous Integration] github Actions pipeline are 
green.

## Code structure

The following is a brief explanation of the tree structure of the repository:

```
agora-airgap/
├── CMakeLists.txt                          << Main Configuration file for CMake
├── Format.cmake/                           << CMake config for clang-format
│   └── ...
├── LICENSE                                 << Main License (AGPL-v3.0)
├── LICENSES/                               << License files used by reuse
│   └── ...
├── README.md                               << This file
├── apps/                                   << The code of the app is here
│   ├── CMakeLists.txt                      << CMake configuration for the app
│   ├── interface.cpp                       << GUI source code
│   ├── interface.h                         << GUI source code header
│   └── main.cpp                            << CLI (Currently Unused)
├── cmake/                                  << CMake files to find some deps
│   ├── FindGMP.cmake
│   └── FindGMPXX.cmake
├── flake.nix                               << Build configuration file for Nix
├── include/                                << Headers for our libraries
│   └── agora-airgap/                       << Header for agora-airgap lib
│       ├── ElGamal.h
│       ├── ...
│       └── sha256.h
├── python/                                 << some currently unused python code
├── src/                                    << source code of the library
│   ├── CMakeLists.txt                      << CMake configuration for the app
│   ├── ElGamal.cpp
│   ├── Random.cpp
│   ├── encrypt.cpp
│   ├── screen.png                          << image used in the GUI
│   ├── screen.png.license                  << reuse license header for screen.png file
│   ├── sha256.cpp
└── tests/                                  << Unit tests
    ├── CMakeLists.txt
    ├── fixtures/                           << Fixture used in some unit tests 
    │   ├── example_1/                      << Each fixture has its own dir
    │   │   ├── ballot.json                 << JSON Ballot of the fixture
    │   │   ├── ballot.json.license         << reuse license header for ballots.json file
    │   │   ├── config                      << election configuration
    │   │   ├── config.license              << reuse license header for config
    │   │   ├── expectations.json           << fixture expectations for unit tests
    │   │   ├── expectations.json.license   
    │   │   ├── pk_1                        << public keys of election 1
    │   │   ├── pk_1.license
    │   │   ├── pk_1110                     << public keys for election 1110
    │   │   ├── pk_1110.license
    │   │   ├── votes.json                  << currently unused
    │   │   └── votes.json.license
    |   └─── ...                            << more fixtures
    ├── tests.cpp                           << unit tests
    └── update_fixtures.sh                  <<| updates the fixtures, as some  
                                              | are copies of example_1/ unit 
                                              | test.
```

## Dependencies

`agora-airgap` is developed in C++11. It uses the following tools:
- [git] and [GitHub] for source code control.
- [Nix Package Manager] for building a Nix [flake].
- [cmake] as the C++ code building tool.
- [ninja] as a building tool and target for cmake.
- [reuse] to checks that all files have a correct copyright headers (for code
  quality).
- [clang-format] to check code styling (for code quality).
- [cppcheck] to run C++ static code analysis (for code quality).

`agora-airgap` uses the following libraries:
- [rapidjson] as a library for processing JSON data.
- [Crypto++] as a library to execute cryptographic operations.
- [gmplib] as a library for modular exponentiation.
- [googletest] as a library to create unit tests.
- [wxWidgets] as the GUI library.

## Configuring the Development

agora-airgap uses [Nix Package Manager] as its package builder. To build 
agora-airgap, **first [install Nix]** correctly in your system.

After you have installed Nix, you can build agora-airgap executing the following
command in the agora-airgap main directory:

## Continuous Integration

A Continuous Integration (CI) pipeline has been setup to be executed everytime
someone pushes into master or a stable branch or into a branch that should be
merged into those.

The CI pipeline includes the following tasks:
- Builds the Nix Flake for Linux and Mac OS X.
- Compiles the code with -Wall and any warning will fail.
- Executes all unit tests in Linux and Mac OS X.
- Archives the generated artifacts (the flake).
- Checks that all files have a correct copyright header with [reuse].
- Checks code styling with [clang-format].
- Executes C++ static code analysis with [cppcheck].

Refer to the `.github/workflows/build.yml` for more information on each of the
steps.

## The screen.h file

The file `include/agora-airgap/screen.h` includes the PNG found in
`src/screen.png` file. The `screen.h` has been generated with the [bin2c tool].

---
[Ballot Casting Assurance via Voter-Initiated Poll Station Auditing](https://www.usenix.org/legacy/event/evt07/tech/full_papers/benaloh/benaloh.pdf)
[Linux binary](https://github.com/agoravoting/agora-airgap/releases/download/0.50/agora-airgap-windows.zip)
[Mac OS X binary](https://github.com/agoravoting/agora-airgap/releases/download/3.4.0/agora-airgap-linux-3.4.0.tar.gz)
[Nix Package Manager](https://nixos.org/)
[install Nix](https://nixos.org/)
[bin2c tool](https://github.com/gwilymk/bin2c)
[Contributor License Agreement](https://example.com)
[whatrever](#continuous-integration)
[reuse](https://reuse.software/)
[clang-format](https://releases.llvm.org/7.1.0/tools/clang/docs/ClangFormatStyleOptions.html)
[cppcheck](https://cppcheck.sourceforge.io)
[flake](https://nixos.wiki/wiki/Flakes)
[rapidjson](https://rapidjson.org/)
[Crypto++](https://cryptopp.com/)
[ninja](TODO)
[gmplib](https://gmplib.org/)
[googletest](https://github.com/google/googletest)
[wxWidgets](TODO)
[git](https://github.com/agoravoting/agora-airgap/)
[GitHub](https://github.com/agoravoting/agora-airgap/)
