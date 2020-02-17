# Universal Crypto Asset Token(UCAT)

# Simple Summary
  A standard interface for cross chain non-fungible tokens(NFT).

# Abstract
  The following standard allows the implementation of a standard API for NFTs within EOS/IOST/ETH smart contracts. This standard provides basic functionality to track and transfer NFTs.

  NFTs can represent ownership over digital assets.

# Motivation
  A standard interface allows wallet/broker/auction applications to work with any NFT on EOS/IOST/ETH blockchain. A simple EOS/IOST smart contract is provided.

# Specification
The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

```
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/types.hpp>
#include <string>

#define max(a,b) (a>b?a:b)

#define ASSET_EOS_SYMBOL S(4, EOS)
#define EOS_ACCOUNT N(eosio.token)
#define ASSET_XPC_SYMBOL S(4, XPC)
#define XPC_ACCOUNT N(xpctokencore)
#define LOG_EXPIRE_SECOND 3600

#define SYSPARAM_TOKEN_COUNT        1
#define SYSPARAM_MIN_LOG_ID         2
#define SYSPARAM_MAX_LOG_ID         3
#define SYSPARAM_ADMIN_ACCOUNT      4
#define API_URL			    5



class eosnft : public eosio::contract {
public:
    eosnft(account_name self)
        : contract(self),
        sysparams(_self, _self),
        logs(_self, _self),
        tokens(_self, _self),
        accounts(_self, _self)
    {
    }

    /**
     **/
    void apply(account_name contract, uint64_t action);

private:

    // @abi table sysparam i64
    struct sysparam {
        uint64_t id;
	std::string tag;
        std::string value;


        sysparam() = default;

        EOSLIB_SERIALIZE(sysparam, (id)(tag)(value))

        uint64_t primary_key() const { return id; }

    };
    static constexpr uint64_t SYSPARAM_TABLE_NAME = N(sysparam);
    using sysparam_list = eosio::multi_index<SYSPARAM_TABLE_NAME, sysparam>;

    
    // @abi table token i64
    struct token {
        uint64_t id;
	std::string uuid;
        std::string name;
	account_name owner;
	std::string imageUrl;
	std::string category;
	std::string ext;
	std::string meta;
	bool lock;

        token() = default;
        
        EOSLIB_SERIALIZE(token, (id)(uuid)(name)(owner)(imageUrl)(category)(ext)(meta)(lock))
       
        uint64_t primary_key() const { return id; }
    };
    static constexpr uint64_t TOKEN_TABLE_NAME = N(token);
    using token_list = eosio::multi_index<TOKEN_TABLE_NAME, token>;

 
     // @abi table accounts i64
    struct accounts {
        account_name id;

        uint64_t count;

        EOSLIB_SERIALIZE(accounts, (id)(count))

        account_name primary_key() const { return id; }
    };
    static constexpr account_name LOOTPAY_TABLE_NAME = N(accounts);
    using account_list = eosio::multi_index<LOOTPAY_TABLE_NAME, accounts>;


    // @abi table logs i64
    struct logs {
        uint64_t id;
	account_name from;
	account_name to;
	std::string memo;
	uint64_t acttime;
	uint64_t tokenId;

        EOSLIB_SERIALIZE(logs, (id)(from)(to)(memo)(acttime)(tokenId))

        uint64_t primary_key() const { return id; }
    };
    static constexpr uint64_t LOG_TABLE_NAME = N(logs);
    using log_list = eosio::multi_index<LOG_TABLE_NAME, logs>;

 private:
    account_list accounts;
    token_list tokens;
    log_list logs;
    sysparam_list sysparams;
    
 private:
     account_name get_admin() const {
         const std::string adminAccount = getsysparam(SYSPARAM_ADMIN_ACCOUNT);
         if (adminAccount.empty()) {
             return _self;
         }
         else {
             return eosio::string_to_name(adminAccount.c_str());
         }
     }

     void require_auth_admin() const { eosio::require_auth(get_admin()); }

     void require_auth_contract() const { eosio::require_auth(_self); }
     
     inline std::string getsysparam(const uint64_t& key) const {
       auto iter = sysparams.find(key);
       if(iter == sysparams.end()){
	        return std::string("");
       }else{
	        return iter->value;
       }
    }

    
    inline void setsysparam(const uint64_t& id, const std::string& tag, const std::string& val){
	    auto iter = sysparams.find(id);
	    if(iter == sysparams.end()){
		    sysparams.emplace(_self, [&](auto& p) {
		        p.id = id;
		        p.value=val;
			p.tag = tag;
		    });
	    }else{
		    sysparams.modify(iter, _self, [&](auto& p) {
			    p.value = val;
		    });
	    }
    }

    inline void addaccounttoken(const account_name& user) {
        auto iter = accounts.find(user);
        if (iter == accounts.end()) {
            accounts.emplace(_self, [&](auto& p) {
                p.id = user;
                p.count = 1;
                });
        }
        else {
            accounts.modify(iter, _self, [&](auto& p) {
                p.count += 1;
            });
        }
    }

    inline void subaccounttoken(const account_name& user) {
        auto iter = accounts.find(user);
        if (iter == accounts.end()) {
            return ;
        }
        
	if(iter->count > 1){
            accounts.modify(iter, _self, [&](auto& p) {
                if (p.count > 1){
                    p.count -= 1;
                }
           });
        }else{
	   accounts.erase(iter);
	}
    }
    
    inline uint64_t toInt(const std::string& str) {
        if (str.empty()) {
            return 0;
        }
        else {
            std::string::size_type sz = 0;
            return std::stoull(str, &sz, 0);
        }
    }
    
    inline uint64_t getminlogid() {
        return toInt(getsysparam(SYSPARAM_MIN_LOG_ID));
    }
    
    inline uint64_t getmaxlogid() {
        return toInt(getsysparam(SYSPARAM_MAX_LOG_ID));
    }

    inline void setminlogid(const uint64_t id) {
        setsysparam(SYSPARAM_MIN_LOG_ID, "SYSPARAM_MIN_LOG_ID", std::to_string(id));
    }

    inline void setmaxlogid(const uint64_t id) {
        setsysparam(SYSPARAM_MAX_LOG_ID, "SYSPARAM_MIN_LOG_ID", std::to_string(id));
    }

    void logoperator(const uint64_t& id, const account_name& oldowner, const account_name& newowner, const std::string& opcode, const std::string& ext);

    void clearlog();

    inline uint64_t gettokencount(){
	return toInt(getsysparam(SYSPARAM_TOKEN_COUNT));
    }

    inline void addtokencount(){
	setsysparam(SYSPARAM_TOKEN_COUNT, "SYSPARAM_TOKEN_COUNT", std::to_string(gettokencount() + 1));
    }

    inline void subtokencount(){
        uint64_t tokencount = gettokencount();
	if(tokencount > 0){
	    setsysparam(SYSPARAM_TOKEN_COUNT, "SYSPARAM_TOKEN_COUNT", std::to_string(tokencount - 1));
	}
    }


 public:

     // @abi action
    void init(const std::string adminacc, const std::string apiUrl){
	require_auth_contract();
	setsysparam(SYSPARAM_ADMIN_ACCOUNT, "SYSPARAM_ADMIN_ACCOUNT", adminacc);
	setsysparam(API_URL, "API_URL", apiUrl);
    }
 
    // @abi action
    void assign(const uint64_t id, const account_name newowner);
    
    // @abi action
    void create(const uint64_t id, const std::string uuid, const std::string category, const std::string name, const std::string imageUrl, const std::string meta, const bool lock, const std::string ext);

    // @abi action
    void updatemeta(const uint64_t id, const std::string name, const std::string category, const std::string imageUrl, const std::string meta);

    // @abi action
    void updatelock(const uint64_t id, const bool lock);

    // @abi action
    void transfer(const uint64_t id, const account_name newowner, const std::string memo);

    // @abi action
    void setparam(const uint64_t id, const std::string tag, const std::string val);

    //---- debug 
    
    // @abi action
    void rmtoken(const uint64_t id);

    // @abi action
    void rmaccount(const account_name acc);

    // @abi action
    void rmparam(const uint64_t id){
	require_auth_contract();
	sysparams.erase(sysparams.find(id));
    }

    // @abi action
    void rmlog(const uint64_t id){
	require_auth_contract();
	logs.erase(logs.begin());
    }

    //---

};

```
