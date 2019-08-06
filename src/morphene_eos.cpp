#include <morphene_eos.hpp>

ACTION morphene_eos::reguser( name username ) {
	require_auth(username);

	if( !user_exists(username) ) create_user(username);
}

ACTION morphene_eos::withdraw( name username, asset amount ) {
	require_auth(username);

	check(user_exists(username), "user does not exist. call reguser action or transfer some EOS to morphene_eos.");

	userstable users(_self, _self.value);

	auto itr = users.find(username.value);

	check(itr->balance.amount > 0, "user has no balance.");
	check(amount.amount > 0, "user must withdraw something.");
	check(itr->balance >= amount, "user attempting to withdraw more than balance.");

  action act(
    permission_level{_self, "active"_n},
    "eosio.token"_n, "transfer"_n,
    std::make_tuple(_self, username, amount, std::string(""))
  );

  act.send();

  users.modify(itr, _self, [&](auto& o){
  	o.balance -= amount;
  	o.updated_at = current_time_point().sec_since_epoch();
  });

}

ACTION morphene_eos::newauction( name username, newauction_params params ) {
  require_auth(username);

  check(user_exists(username), "user does not exist. call reguser action or transfer some EOS to morphene_eos.");

  userstable users(_self, _self.value);
  auctionstable auctions(_self, _self.value);

  auto itr = users.find(username.value);

  auto ctime = current_time_point().sec_since_epoch();

  check(itr->balance.amount > 0, "user has no balance; transfer EOS to contract.");
  check(itr->balance >= params.fee, "user does not have enough EOS balance.");
  check(params.start_time > ctime, "start_time must be in the future.");
  check(params.end_time > params.start_time, "end_time must be greater than start_time.");

  auctions.emplace(_self, [&](auto& o) {
    o.id = auctions.available_primary_key();
    o.creator = params.creator;
    o.total_value = params.fee;
    o.start_time = params.start_time;
    o.end_time = params.end_time;
    o.created_at = ctime;
    o.updated_at = ctime;

    users.modify(itr, _self, [&](auto& u) {
      u.balance -= asset(params.fee.amount, symbol("EOS", 4));
      u.updated_at = ctime;
    });
  });
}

ACTION morphene_eos::startsystem() {
  require_auth(_self);

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

  transaction deferred;

  deferred.actions.emplace_back(
    permission_level(_self, "active"_n),
    _self, "startsystem"_n, ""
  );

  deferred.delay_sec = 3;
  deferred.send(_self.value, _self, true);
}

ACTION morphene_eos::stopsystem() {
  require_auth(_self);

  cancel_deferred(_self.value);
}
