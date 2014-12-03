### agora-airgap

A version of agora voting to create encrypted votes on an air-gap computer off the grid. This project is aimed to a run on an openrisc architecture.

# How to

You need to install gmp-devel gcc-c++ git premake4 libcurl-devel. Then download and get the source code:

    git clone https://github.com/miloyip/rapidjson
    git clone https://github.com/Findeton/agora-airgap.git
    cp -r rapidjson/include/rapidjson agora-airgap/src

And compile:

  cd agora-airgap/src
  make

and now you have your binary in x64/main

