{
  description = "C Development Environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # Compilers and Debuggers
            gcc
            gdb

            # Build Tools
            gnumake
            cmake

            # Language Server / Tooling (LSPs)
            clang-tools # Provides clangd for linting and code navigation
          ];
        };
      }
    );
}
