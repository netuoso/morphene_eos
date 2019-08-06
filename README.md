# Morphene_EOS Smart Contract

---

### User Actions
- reguser
	- allows user registration into the system
	- could be used to motivate early adoption
	- params:
		- `username` - (type: name)
- withdraw
	- transfers the users balance from the contract
	- deposits + earnings
	- params:
		- `username` - (type: name)
- newauction
	- allows a user to create an auction
	- params: `username, params`
		- `creator` (type: name)
		- `fee` (type: uint64_t)
		- `start_time` (type: uint64_t)
		- `end_time` (type: uint64_t)
- placebid
	- allows user to place a bid on auction
	- minimum bid is 0.500 EOS
	- balance must be deposited to smart contract account prior to bidding
	- params:
		- `username` - (type: name)
		- `auction_id` - (type: uint64_t)

### System Actions
- startsystem
	- allows smart contract owner to begin processing of auctions and payouts
- stopsystem
	- allows smart contract owner to cease processing of auctions and payouts

### Handlers
- `eosio.token` handler for deposits made to CONTRACTACCOUNT
- `eosio::onerror` handler for failures in deferred action processing

### System Processing of Auctions and Payouts
- `startsystem` executes a deferred action loop that fires every 60 seconds
- in case of deferred action failure, the action will be retried every 1 second by the onerror handler

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

### Authors
- Andrew Chaney - @netuoso - EOS Titan
