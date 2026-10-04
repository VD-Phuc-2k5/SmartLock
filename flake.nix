{
  description = "Môi trường phát triển Node.js với npm và pnpm";

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
            # Node.js runtime (npm comes bundled with nodejs)
            nodejs_22
            # pnpm được cung cấp riêng
            pnpm
          ];

          shellHook = ''
            echo "Môi trường phát triển Node.js đã sẵn sàng"
            echo "Node.js: $(node --version)"
            echo "npm:     $(npm --version)"
            echo "pnpm:    $(pnpm --version)"
          '';
        };
      });
}