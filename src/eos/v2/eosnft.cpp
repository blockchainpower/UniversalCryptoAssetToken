#include "eosnft.hpp"

void eosnft::create(const uint64_t id, const std::string uuid, const std::string category, std::string title, std::string imageUrl, std::string meta, const bool lock, const std::string ext){
	require_auth_admin();

	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
	check(iter == tokens.end(), "asset exisit");

	tokens.emplace(_self, [&](auto& p) {
		p.id = id;
		p.owner = get_admin();
		p.category = category;
		p.title = title;
		p.imageUrl = imageUrl;
		p.meta = meta;
		p.uuid = uuid;
		p.lock = lock;
		p.ext = ext;
		});
	addtokencount();

	log(id, get_admin(), get_admin(), "CREATE", "");
}

void eosnft::assign(const uint64_t id, name newowner){
	require_auth_admin();

	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);

    check(iter != tokens.end(), "asset not exisit");

	eosio::require_auth(iter->owner);

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	addaccounttoken(newowner);

	log(id, get_admin(), newowner, "ASSIGN", "");
}

void eosnft::reassign(const uint64_t id, name newowner){
	require_auth_admin();

	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);

    check(iter != tokens.end(), "asset not exisit");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	addaccounttoken(newowner);
	log(id, get_admin(), newowner, "ASSIGN", "");
}

void eosnft::log(const uint64_t& id, const name& oldowner, const name& newowner, const std::string& opcode, const std::string& ext) {
	auto maxId = 0;

	log_index logs(_self, _self.value);

	logs.emplace(_self, [&](auto& p) {
		p.id = max(1, logs.available_primary_key());
		p.from = oldowner;
		p.to = newowner;
		p.acttime = current_time_point().sec_since_epoch() ;
		p.memo = opcode + std::string("-") + std::to_string(id);
		if(!ext.empty()){
			p.memo = p.memo + std::string("-") + ext;
		}
		p.tokenId=id;
		maxId = p.id;
		});
	
	std::string memo = opcode + std::string("-") + std::to_string(id) + std::string("-") + ext;

	clearlog();
}

void eosnft::clearlog() {
	log_index logs(_self, _self.value);

	auto iter = logs.begin();
	if (iter == logs.end()) {
		return;
	}
	
	if(logs.rend() == logs.rbegin()){
		return ;
	}

	if ((logs.rbegin()->id - iter->id) >= 1000) {
		logs.erase(iter);
	}
	iter = logs.begin();
}

void eosnft::updatemeta(const uint64_t id, const std::string title, const std::string category, const std::string imageUrl, const std::string meta){
	require_auth_admin();
	
	token_index tokens(_self, _self.value);
	auto iter = tokens.find(id);

    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.title = title;
		p.imageUrl = imageUrl;
		p.category = category;
		p.meta = meta;
    });
	log(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::updatelock(const uint64_t id, const bool lock){
	require_auth_admin();
	token_index tokens(_self, _self.value);
	auto iter = tokens.find(id);
    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.lock = lock;
    });
	log(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::updateext(const uint64_t id, const std::string ext){
	require_auth_admin();
	token_index tokens(_self, _self.value);
	auto iter = tokens.find(id);
    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.ext = ext;
    });
	log(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::transmk(const uint64_t id, const name newowner, const std::string memo){
	transfer(id,newowner,memo);
}

void eosnft::transfer(const uint64_t id, name newowner, const std::string memo){
	check(is_account(newowner), "new owner should be an account");

	token_index tokens(_self, _self.value);
    auto iter = tokens.find(id);

    check(iter != tokens.end(), "unknow asset");
	check(!iter->lock, "asset is locked");

	blackcheck(iter->owner);

    eosio::require_auth(iter->owner);
	name oldowner = iter->owner;

	check(oldowner != newowner, "asset aleady yours");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	subaccounttoken(oldowner);
	addaccounttoken(newowner);

	log(id, oldowner, newowner, "TRANSFER", memo);

	notifyall();
}
