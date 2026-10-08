{
  description = "Môi trường phát triển Node.js và Python";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            nodejs_22
            pnpm

            # Python
            python3
            python3Packages.pip
            python3Packages.pyserial
            python3Packages.paho-mqtt
          ];

          shellHook = ''
            echo "Môi trường phát triển Node.js + Python đã sẵn sàng"
            echo "Node.js: $(node --version)"
            echo "npm:     $(npm --version)"
            echo "pnpm:    $(pnpm --version)"
            echo "Python:  $(python --version)"
          '';
        };
      });
}