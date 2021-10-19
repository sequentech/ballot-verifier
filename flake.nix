/**
 * This file is part of agora-airgap.
 * Copyright (C) 2014-2021  Agora Voting SL <agora@agoravoting.com>

 * agora-airgap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License.

 * agora-airgap  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with agora-airgap.  If not, see <http://www.gnu.org/licenses/>.
**/
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
            nativeBuildInputs = [ pkgs.pkg-config ];
            buildInputs = [
                pkgs.gnumake
                pkgs.gnum4
                pkgs.rapidjson
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
            buildPhase = ''
              cd src && make gui
            '';
            installPhase = ''
              mkdir -p $out/bin
              [ -f x64/agora-audit ] && mv x64/agora-audit $out/bin
              echo $?
            '';
          };
          # agora-airgap is the default package
          defaultPackage = packages.agora-airgap;

          # configure the dev shell
          devShell = pkgs.mkShell { 
            buildInputs = packages.agora-airgap.buildInputs; 
          };
        }
    );
}
