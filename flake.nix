# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

{
  description = "Flake for agora-airgap, a cast-as-intended verifier for nVotes platform";

  # input
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.05";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  # output function of this flake
  outputs = { self, nixpkgs, flake-utils }: 
    flake-utils.lib.eachDefaultSystem (
      system:
        let 
          # pkgs is just the nix packages
          pkgs = nixpkgs.legacyPackages.${system};

          # Static GMP with C++ support
          gmpCustom = 
            pkgs
              .gmp
              .override {
                withStatic = true;
                cxx = true;
              };

          # static wxmac/wxGTK30
          wxCustom = 
            (if system == "x86_64-darwin" then pkgs.wxmac else pkgs.wxGTK30);

          # static curl with idn support
          curlCustom = 
            (pkgs.curl
              .override {
                idnSupport = true;
                sslSupport = true;
              });

        # resulting packages of the flake
        in rec {
          # Derivation for agora-airgap
          packages.agora-airgap = pkgs.stdenv.mkDerivation {
            name = "agora-airgap";
            version = "4.0.2";
            src = self;
            type = "git"; 
            submodules = "true";
            nativeBuildInputs = [
                pkgs.pkg-config
                pkgs.cmake
                pkgs.cmake-format
                pkgs.rapidjson
                pkgs.cryptopp
                pkgs.ninja
                pkgs.git
                pkgs.cppcheck
                pkgs.reuse
            ];
            buildInputs = [
                # the overlayed libraries
                gmpCustom
                wxCustom
                curlCustom
              ]
              ++ nixpkgs.lib.optionals (system == "x86_64-darwin") [
                pkgs.pkgs.darwin.apple_sdk.frameworks.IOKit
                pkgs.pkgs.darwin.apple_sdk.frameworks.Carbon
                pkgs.pkgs.darwin.apple_sdk.frameworks.Cocoa
              ];
          };
          # agora-airgap is the default package
          defaultPackage = packages.agora-airgap;

          # configure the dev shell
          devShell = pkgs.mkShell { 
            buildInputs = packages.agora-airgap.nativeBuildInputs 
              ++ packages.agora-airgap.buildInputs; 
          };
        }
    );
}
