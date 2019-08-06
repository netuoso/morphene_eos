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

ACTION morphene_eos::newauction( name username ) {
  require_auth(username);

  // todo: create auction
}
