#include "eosnft.hpp"

void eosnft::applymigrate(const uint64_t id, const std::string target){
	auto iter = tokens.find(id);
    eosio_assert(iter != tokens.end(), "unknow asset");
	eosio_assert(!iter->lock, "asset is locked");
	blackcheck(iter->owner);
	migratecheck(id);
    eosio::require_auth(iter->owner);
	migrates.emplace(_self, [&](auto& p) {
		p.id = id;
		p.target = target;
	});
}

void eosnft::setparam(const uint64_t id, const std::string tag, const std::string val){
	setsysparam(id, tag, val);
}

// @abi action
void eosnft::burn(const uint64_t id){
	require_auth_admin();

	migratecheck(id);

    auto iter = tokens.find(id);

    eosio_assert(iter != tokens.end(), "unknow asset");

	tokens.erase(tokens.find(id));

	subtokencount();

	subaccounttoken(iter->owner);

	logoperator(id, get_admin(), get_admin(), "BURN", "");
}


void eosnft::create(const uint64_t id, const std::string uuid, const std::string category, std::string name, std::string imageUrl, std::string meta, const bool lock, const std::string ext){
	require_auth_admin();
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

	auto iter = tokens.find(id);

    eosio_assert(iter != tokens.end(), "asset not exisit");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	addaccounttoken(newowner);
	logoperator(id, get_admin(), newowner, "ASSIGN", "");
}

void eosnft::logoperator(const uint64_t& id, const account_name& oldowner, const account_name& newowner, const std::string& opcode, const std::string& ext) {
	auto maxId = 0;

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
		maxId = p.id;
		});
	
	std::string memo = opcode + std::string("-") + std::to_string(id) + std::string("-") + ext;

	clearlog();

	setmaxlogid(maxId);
}

void eosnft::clearlog() {
	auto iter = logs.begin();
	if (iter == logs.end()) {
		return;
	}
	
	if(logs.rend() == logs.rbegin()){
		return ;
	}

	if ((now() - iter->acttime) > LOG_EXPIRE_SECOND) {
		logs.erase(iter);
	}
	iter = logs.begin();
	setminlogid(iter->id);
}

void eosnft::updatemeta(const uint64_t id, const std::string name, const std::string category, const std::string imageUrl, const std::string meta){
	require_auth_admin();
	auto iter = tokens.find(id);
    eosio_assert(iter != tokens.end(), "asset not exisit");
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
	auto iter = tokens.find(id);
    eosio_assert(iter != tokens.end(), "asset not exisit");
	tokens.modify(iter, _self, [&](auto& p) {
		p.lock = lock;
    });
	logoperator(id, iter->owner, iter->owner, "UPDATE", "");
}

void eosnft::transfer(const uint64_t id, account_name newowner, const std::string memo){
	eosio_assert(is_account(newowner), "new owner should be an account");
    auto iter = tokens.find(id);

    eosio_assert(iter != tokens.end(), "unknow asset");
	eosio_assert(!iter->lock, "asset is locked");

	migratecheck(id);

	blackcheck(iter->owner);

    eosio::require_auth(iter->owner);
	account_name oldowner = iter->owner;

	eosio_assert(oldowner != newowner, "asset aleady yours");

	tokens.modify(iter, _self, [&](auto& p) {
        p.owner = newowner;
    });

	subaccounttoken(oldowner);
	addaccounttoken(newowner);

	logoperator(id, oldowner, newowner, "TRANSFER", memo);
}


void eosnft::
    apply(account_name contract, uint64_t action)
{
    if (contract == _self) {
        auto& thiscontract = *this;
        switch (action) {
            EOSIO_API(eosnft, (create)(assign)(reassign)(updatemeta)(updatelock)(transfer)(init)(burn)(applymigrate)(apprmigrate)(addblack)(rmblack)(setparam)(rmlog))
        }
    }
}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    eosnft ps(receiver);
    ps.apply(code, action);
    eosio_exit(0);
}
}
