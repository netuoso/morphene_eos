#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

CONTRACT morphene_eos : public contract {
   public:

      using contract::contract;

      // structs
      struct st_transfer {
          name          from;
          name          to;
          asset         quantity;
          std::string   memo;
      };

      // tables
      TABLE users {
        name username;
        asset balance = asset(0, symbol("EOS", 4));

        uint64_t total_bids = 0;
        uint64_t total_auctions = 0;

        uint64_t updated_at;
        uint64_t created_at;

        uint64_t primary_key() const {return username.value;}
      };

      TABLE auctions {
        uint64_t id;

        name creator;
        name last_bidder;

        std::string status;

        uint64_t start_time;
        uint64_t end_time;
        uint64_t bids_count;
        asset total_value;

        uint64_t created_at;
        uint64_t updated_at;

        uint64_t primary_key() const {return id;}
        uint64_t by_creator() const {return creator.value;}
        uint64_t by_bidder() const {return last_bidder.value;}
      };

      // indexes
      typedef eosio::multi_index<"users"_n, users> userstable;

      typedef eosio::multi_index<"auctions"_n, auctions,
          indexed_by<"creator"_n, const_mem_fun<auctions, uint64_t, &auctions::by_creator>>, 
          indexed_by<"bidder"_n, const_mem_fun<auctions, uint64_t, &auctions::by_bidder>>> auctionstable;

      // action definitions

      ACTION reguser( name username );
      ACTION withdraw( name username, asset amount );
      ACTION newauction( name username );
      // ACTION placebid( uint64_t auction_id );
      // ACTION claim( name username );

      using reguser_action = action_wrapper<"reguser"_n, &morphene_eos::reguser>;
      using withdraw_action = action_wrapper<"withdraw"_n, &morphene_eos::withdraw>;
      using newauction_action = action_wrapper<"newauction"_n, &morphene_eos::newauction>;

      [[eosio::on_notify("eosio.token::transfer")]]
      void transfer(uint64_t sender, uint64_t receiver) {

        userstable users(_self, _self.value);

        auto transfer_data = unpack_action_data<morphene_eos::st_transfer>();
        if (transfer_data.from != _self && transfer_data.to == _self){
          if( !user_exists(transfer_data.from) ) create_user(transfer_data.from);

          auto itr = users.find(transfer_data.from.value);
          users.modify(itr, _self, [&](auto& o) {
            o.balance += transfer_data.quantity;
            o.updated_at = current_time_point().sec_since_epoch();
          });
        }

      }

      using transfer_action = action_wrapper<"transfer"_n, &morphene_eos::transfer>;

  private:

    bool user_exists(name username){
      userstable users(_self, _self.value);

      auto itr = users.find(username.value);
      if (itr != users.end()) return true;
      else return false;
    }

    void create_user(name username){
      userstable users(_self, _self.value);

      users.emplace(_self, [&](auto& o){
        o.username = username;
        o.updated_at = current_time_point().sec_since_epoch();
        o.created_at = current_time_point().sec_since_epoch();
      });
    }

};
