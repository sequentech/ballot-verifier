### agora-airgap

Set of Agora Voting tools for:
- Verifying a ballot from the "cast or cancel" procedure is correct
- Creating a ballot from an air gap computer (ideally on an OpenRISC open hardware arquitecture)

# How to

You need to install gmp-devel gcc-c++ git premake4 libcurl-devel. Then download and get the source code:

    git clone https://github.com/miloyip/rapidjson
    git clone https://github.com/agoravoting/agora-airgap.git
    cp -r rapidjson/include/rapidjson agora-airgap/src

And compile:

  cd agora-airgap/src
  make

and now you have your binary in x64/main

