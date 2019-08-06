# Morphene_EOS Smart Contract

---

### Building
- clone this repository
	- `git clone https://github.com/netuoso/morphene_eos`
- `cd build`
- `cmake ..`
- `make`

### Deploying
- `cleos set code CONTRACTACCOUNT morphene_eos/ morphene_eos.wasm morphene_eos.abi`
- `cleos set account permission CONTRACTACCOUNT active --add-code`

### Usage Guide
- `cleos push action CONTRACTACCOUNT startsystem "{}" -p CONTRACTACCOUNT`
- `cleos push action CONTRACTACCOUNT reguser "{\"username\":\"USERACCOUNT\"}" -p USERACCOUNT`
- `cleos transfer USERACCOUNT CONTRACTACCOUNT "10.0000 EOS" "balance deposit"`
- ```cleos push action CONTRACTACCOUNT newauction "{\"username\":\"USERACCOUNT\",\"params\":{\"creator\":\"USERACCOUNT\",\"fee\":\"1.0000 EOS\",\"start_time\":`date -v +30S +%s`,\"end_time\":`date -v +60S +%s`}}" -p USERACCOUNT```
- `cleos push action CONTRACTACCOUNT placebid "{\"username\":\"USERACCOUNT\",\"auction_id\":1}" -p USERACCOUNT`
- `cleos get table CONTRACTACCOUNT CONTRACTACCOUNT auctions`
- `cleos get table CONTRACTACCOUNT CONTRACTACCOUNT users`
