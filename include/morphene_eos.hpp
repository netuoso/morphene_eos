#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
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

      struct newauction_params {
          name          creator;
          uint64_t      start_time;
          uint64_t      end_time;
          asset         fee;
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
        uint64_t by_balance() const {return balance.amount;}
      };

      TABLE auctions {
        uint64_t id;

        name creator;
        name last_bidder = ""_n;

        name status = "pending"_n;

        uint64_t start_time;
        uint64_t end_time;

        uint64_t bids_count = 0;
        asset total_value = asset(0, symbol("EOS", 4));

        uint64_t created_at;
        uint64_t updated_at;

        uint64_t primary_key() const {return id;}
        uint64_t by_creator() const {return creator.value;}
        uint64_t by_bidder() const {return last_bidder.value;}
        uint64_t by_status() const {return status.value;}
        uint64_t by_start_time() const {return start_time;}
        uint64_t by_end_time() const {return end_time;}
      };

      // indexes
      typedef eosio::multi_index<"users"_n, users,
          indexed_by<"balance"_n, const_mem_fun<users, uint64_t, &users::by_balance>>> userstable;

      typedef eosio::multi_index<"auctions"_n, auctions,
          indexed_by<"creator"_n, const_mem_fun<auctions, uint64_t, &auctions::by_creator>>,
          indexed_by<"bidder"_n, const_mem_fun<auctions, uint64_t, &auctions::by_bidder>>,
          indexed_by<"status"_n, const_mem_fun<auctions, uint64_t, &auctions::by_status>>,
          indexed_by<"start"_n, const_mem_fun<auctions, uint64_t, &auctions::by_start_time>>,
          indexed_by<"end"_n, const_mem_fun<auctions, uint64_t, &auctions::by_end_time>>> auctionstable;

      // action definitions
      ACTION reguser( name username );
      ACTION withdraw( name username, asset amount );
      ACTION newauction( name username, newauction_params params );
      ACTION placebid( name username, uint64_t auction_id );
      ACTION startsystem( );
      ACTION stopsystem( );
      // ACTION placebid( uint64_t auction_id );
      // ACTION claim( name username );

      // handlers
      [[eosio::on_notify("eosio::onerror")]]
      void onError(const onerror &error) {
        // reschedule failed deferred transaction
        transaction deferred = error.unpack_sent_trx();
        deferred.delay_sec = 1;
        deferred.send(_self.value, _self, true);
      }

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

      // action wrappers
      using reguser_action = action_wrapper<"reguser"_n, &morphene_eos::reguser>;
      using withdraw_action = action_wrapper<"withdraw"_n, &morphene_eos::withdraw>;
      using newauction_action = action_wrapper<"newauction"_n, &morphene_eos::newauction>;
      using placebid_action = action_wrapper<"placebid"_n, &morphene_eos::placebid>;
      using startsystem_action = action_wrapper<"startsystem"_n, &morphene_eos::startsystem>;
      using stopsystem_action = action_wrapper<"stopsystem"_n, &morphene_eos::stopsystem>;
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

    void process_auctions(){
      auctionstable auctions(_self, _self.value);
      userstable users(_self, _self.value);

      auto ctime = current_time_point().sec_since_epoch();

      auto idx = auctions.get_index<"end"_n>();
      auto itr = idx.rbegin();

      while(itr != idx.rend()) {
        if(itr->end_time < ctime && itr->status != "ended"_n){
          auctions.modify(*itr, _self, [&](auto& o) {
            o.status = "ended"_n;
            o.updated_at = ctime;

            auto creator = users.find(itr->creator.value);
            auto bidder = users.find(itr->last_bidder.value);
            auto payout_amount = itr->total_value.amount;

            uint64_t creator_payout = 0;
            uint64_t bidder_payout = 0;

            if(itr->last_bidder != ""_n){
              creator_payout = payout_amount * 0.15;
              bidder_payout = payout_amount * 0.80;
            } else {
              creator_payout = payout_amount * 0.95;
            }
            if(creator_payout > 0) {
              users.modify(creator, _self, [&](auto& c) {
                c.balance += asset(creator_payout, symbol("EOS", 4));
              });
            }
            if(bidder_payout > 0) {
              users.modify(bidder, _self, [&](auto& b) {
                b.balance += asset(bidder_payout, symbol("EOS", 4));
              });
            }
          });
        } else if ( itr->start_time < ctime && itr->end_time > ctime && itr->status == "pending"_n ) {
          auctions.modify(*itr, _self, [&](auto& o) {
            o.status = "active"_n;
            o.updated_at = ctime;
          });
        } else if (itr->status == "ended"_n) { break; }

        ++itr;
      }
    }

};
