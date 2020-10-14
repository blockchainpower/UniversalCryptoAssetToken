#include "eosnft.hpp"


void eosnft::create(const uint64_t id, const std::string uuid, const std::string category, std::string name, std::string imageUrl, std::string meta, const bool lock, const std::string ext){
	require_auth_admin();
	
	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
	eosio_assert(iter == tokens.end(), "asset exisit");

	tokens.emplace(_self, [&](auto& p) {
		p.id = id;
		p.owner = get_admin();
		p.category = category;
		p.name = name;
		p.imageUrl = imageUrl;
		p.meta = meta;
		p.uuid = uuid;
		p.lock = lock;
		p.ext = ext;
		});
	addtokencount();

	logoperator(id, get_admin(), get_admin(), "CREATE", "");
}

void eosnft::assign(const uint64_t id, account_name newowner){
	require_auth_admin();

	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);

    eosio_assert(iter != tokens.end(), "asset not exisit");

	eosio::require_auth(iter->owner);

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	addaccounttoken(newowner);

	logoperator(id, get_admin(), newowner, "ASSIGN", "");
}

void eosnft::reassign(const uint64_t id, account_name newowner){
	require_auth_admin();

	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);

    eosio_assert(iter != tokens.end(), "asset not exisit");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	addaccounttoken(newowner);

	logoperator(id, get_admin(), newowner, "ASSIGN", "");
}

void eosnft::logoperator(const uint64_t& id, const account_name& oldowner, const account_name& newowner, const std::string& opcode, const std::string& ext) {
	log_index logs(_self, _self.value);

	uint64_t maxlogId = 1;

	logs.emplace(_self, [&](auto& p) {
		p.id = max(1, logs.available_primary_key());
		p.from = oldowner;
		p.to = newowner;
		p.acttime = now();
		p.memo = opcode + std::string("-") + std::to_string(id);
		if(!ext.empty()){
			p.memo = p.memo + std::string("-") + ext;
		}
		p.tokenId=id;

		maxlogId = p.id;
	});

	clearlog(maxlogId);
}

void eosnft::clearlog(uint64_t maxlogId) {
	log_index logs(_self, _self.value);
	auto iter = logs.begin();
	if (iter == logs.end()) {
		return;
	}

	if(maxlogId - iter->id > 1000){
		logs.erase(iter);
	}
}

void eosnft::updatemeta(const uint64_t id, const std::string name, const std::string category, const std::string imageUrl, const std::string meta){
	require_auth_admin();
	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.name = name;
		p.imageUrl = imageUrl;
		p.category = category;
		p.meta = meta;
    });
	logoperator(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::updatelock(const uint64_t id, const bool lock){
	require_auth_admin();
	
	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.lock = lock;
    });
	logoperator(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::updateext(const uint64_t id, const std::string ext){
	require_auth_admin();
	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
    check(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.ext = ext;
    });
	logoperator(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::transmk(const uint64_t id, const account_name newowner, const std::string memo){
	transfer(id,newowner,memo);
}

void eosnft::transfer(const uint64_t id, name newowner, const std::string memo){

	check(is_account(newowner), "new owner should be an account");

	token_index tokens(_self, _self.value);

    auto iter = tokens.find(id);

    check(iter != tokens.end(), "unknow asset");
	check(!iter->lock, "asset is locked");

	migratecheck(id);

	blackcheck(iter->owner);

    eosio::require_auth(iter->owner);
	account_name oldowner = iter->owner;

	check(oldowner != newowner, "asset aleady yours");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	subaccounttoken(oldowner);
	addaccounttoken(newowner);

	logoperator(id, oldowner, newowner, "TRANSFER", memo);
}
