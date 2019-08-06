# Morphene_EOS Smart Contract

---

### Building
- clone this repository
	- `git clone https://github.com/netuoso/morphene_eos`
- `cd build`
- `cmake ..`
- `make`

### Deploying
- `cleos set code CONTRACTUSER morphene_eos/ morphene_eos.wasm morphene_eos.abi`
- `cleos set account permission CONTRACTUSER active --add-code`
