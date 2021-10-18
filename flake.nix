{
  description = "Flake for agora-verifier, a cast-as-intended verifier for nVotes platform";

  inputs.nixpkgs.linux.url = github:NixOS/nixpkgs/nixos-21.05;
  inputs.nixpkgs.darwin.url = github:NixOS/nixpkgs/nixpkgs-21.05-darwin;

  outputs = { self, nixpkgs }: {

    packages.x86_64-darwin.agora-airgap = 
      with nixpkgs.darwin { system = "x86_64-darwin"; };
      stdenv.mkDerivation {
        name = "hello";
        src = self;
        buildPhase = "gcc -o hello ./hello.c";
        installPhase = "mkdir -p $out/bin; install -t $out/bin hello";
      };

    defaultPackage.x86_64-darwin = self.packages.x86_64-darwin.hello;

  };
}
