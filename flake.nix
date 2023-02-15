# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

{
  description = "Flake for ballot-verifier, a cast-as-intended verifier for Sequent platform";

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
            
          buildTools = [
                pkgs.pkg-config
                pkgs.cmake
                pkgs.cmake-format
                pkgs.rapidjson
                pkgs.cryptopp
                pkgs.ninja
                pkgs.git
                pkgs.cppcheck
                pkgs.ack
                pkgs.reuse
            ];

        # resulting packages of the flake
        in rec {
          # Derivation for ballot-verifier
          packages.ballot-verifier = pkgs.clangStdenv.mkDerivation {
            name = "ballot-verifier";
            version = "4.0.2";
            src = self;
            type = "git"; 
            submodules = "true";
            nativeBuildInputs = buildTools;
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
          # ballot-verifier is the default package
          defaultPackage = packages.ballot-verifier;

          # configure the dev shell
          devShell = (
            pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }
          ) { 
            packages = buildTools;
            buildInputs = packages.ballot-verifier.buildInputs
                ++ [ pkgs.bashInteractive pkgs.cmake ];
          };
        }
    );
}
