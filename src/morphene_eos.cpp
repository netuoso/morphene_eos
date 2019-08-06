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

  process_auctions();

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
