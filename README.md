Universal Crypto Asset Token(UCAT)

For any questions, contributions or feedback join our dedicated telegram group: https://t.me/eosionft

Simple Summary
A standard interface for cross chain non-fungible tokens(NFT).

Abstract
The following standard allows the implementation of a standard API for NFTs within EOS/IOST/ETH smart contracts. This standard provides basic functionality to track and transfer NFTs.

NFTs can represent ownership over digital or physical assets:

Physical property — houses, unique artwork
Cryptocollectibles — unique collectibles or instances which are part of limited-edition collections.
"Negative value" assets — loans, burdens and other responsibilities
NFTs are distinguishable and you must track the ownership of each one separately.

Motivation
A standard interface allows wallet/broker/auction applications to work with any NFT on EOS/IOST/ETH blockchain. A simple EOS/IOST smart contract is provided.

This standard is inspired by the eosio.token standard. eosio.token is insufficient for tracking NFTs because each asset is distinct (non-fungible) whereas each of a quantity of tokens is identical (fungible).

Specification
The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

/// @title eosio.nft public interface
/// @dev See https://github.com/blockchainpower/UniversalCryptoAssetToken/blob/master/README.md

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>
#include <vector>

using namespace eosio;
using std::string;
using std::vector;
typedef uint128_t uuid;
typedef uint64_t id_type;
typedef string uri_type;

CONTRACT nft : public eosio::contract {
    public:
        using contract::contract;
        nft( name receiver, name code, datastream<const char*> ds)
		: contract(receiver, code, ds), tokens(receiver, receiver.value) {}

	/// Creates token with a symbol name for the specified issuer account.
	/// Throws if token with specified symbol already exists.
	/// @param issuer Account name of the token issuer
	/// @param symbol Symbol code of the token
        ACTION create(name issuer, std::string symbol);

	/// Issues specified number of tokens with previously created symbol to the account name "to". 
	/// Each token is generated with an unique token_id assigned to it. Requires authorization from the issuer.
	/// Any number of tokens can be issued.
	/// @param to Account name of tokens receiver
	/// @param quantity Number of tokens to issue for specified symbol (positive integer number)
	/// @param uris Vector of URIs for each issued token (size is equal to tokens number)
	/// @param name Name of issued tokens (max. 32 bytes)
	/// @param memo Action memo (max. 256 bytes)
        ACTION issue(name to,
                   asset quantity,
                   vector<string> uris,
		   string name,
                   string memo);

	/// Transfers 1 token with specified "id" from account "from" to account "to".
	/// Throws if token with specified "id" does not exist, or "from" is not the token owner.
	/// @param from Account name of token owner
	/// @param to Account name of token receiver
	/// @param id Unique ID of the token to transfer
	/// @param memo Action memo (max. 256 bytes)
        ACTION transferid(name from,
                      name to,
                      id_type id,
                      string memo);

	/// Transfers 1 token with specified symbol in asset from account "from" to account "to".
	/// Throws if amount is not 1, token with specified symbol does not exist, or "from" is not the token owner.
	/// @param from Account name of token owner
	/// @param to Account name of token receiver
	/// @param quantity Asset with 1 token 
	/// @param memo Action memo (max. 256 bytes)
	ACTION transfer(name from,
                      name to,
                      asset quantity,
                      string memo);

	/// @notice Burns 1 token with specified "id" owned by account name "owner".
	/// @param owner Account name of token owner
	/// @param id Unique ID of the token to burn
	ACTION burn(name owner,
                  id_type token_id);

	/// @notice Sets owner of the token as a ram payer for stored data.
	/// @param payer Account name of token owner
	/// @param id Unique ID of the token to burn
        ACTION setrampayer(name payer, 
			   id_type id);
    
    	/// Structure keeps information about the balance of tokens 
	/// for each symbol that is owned by an account. 
	/// This structure is stored in the multi_index table.
        TABLE account {

            asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };

	/// Structure keeps information about the total supply 
	/// of tokens for each symbol issued by "issue" account. 
	/// This structure is stored in the multi_index table.
        TABLE stats {
            asset supply;
            name issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
            uint64_t get_issuer() const { return issuer.value; }
        };

	/// Structure keeps information about each issued token.
	/// Each token is assigned a global unique ID when it is issued. 
	/// Token also keeps track of its owner, stores assigned URI and its symbol code.    
	/// This structure is stored in the multi_index table "tokens".
        TABLE token {
            id_type id;          // Unique 64 bit identifier,
            uri_type uri;        // RFC 3986
            name owner;  	 // token owner
            asset value;         // token value (1 SYS)
	    string tokenName;	 // token name

            id_type primary_key() const { return id; }
            uint64_t get_owner() const { return owner.value; }
            string get_uri() const { return uri; }
            asset get_value() const { return value; }
	    uint64_t get_symbol() const { return value.symbol.code().raw(); }
	    string get_name() const { return tokenName; }

	    // generated token global uuid based on token id and
	    // contract name, passed as argument
	    uuid get_global_id(name self) const
	    {
		uint128_t self_128 = static_cast<uint128_t>(self.value);
		uint128_t id_128 = static_cast<uint128_t>(id);
		uint128_t res = (self_128 << 64) | (id_128);
		return res;
	    }

	    string get_unique_name() const
	    {
		string unique_name = tokenName + "#" + std::to_string(id);
		return unique_name;
	    }
        };
	
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
#define API_URL			                5


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
        std::string value;

        sysparam() = default;

        EOSLIB_SERIALIZE(sysparam, (id)(value))

        uint64_t primary_key() const { return id; }

    };
    static constexpr uint64_t SYSPARAM_TABLE_NAME = N(sysparam);
    using sysparam_list = eosio::multi_index<SYSPARAM_TABLE_NAME, sysparam>;

    
    // @abi table token i64
    struct token {
        uint64_t id;
        std::string name;
	    account_name owner;
	    std::string imageUrl;
	    std::string category;
	    std::string meta;

        token() = default;
        
        EOSLIB_SERIALIZE(token, (id)(name)(owner)(imageUrl)(category)(meta))
       
        uint64_t primary_key() const { return id; }
    };
    static constexpr uint64_t TOKEN_TABLE_NAME = N(token);
    using token_list = eosio::multi_index<TOKEN_TABLE_NAME, token>;

 
    // @abi table accountinfo i64
    struct accountinfo {
        account_name id;

        accountinfo() = default;

        uint64_t tokenCount;

        std::string info;

        EOSLIB_SERIALIZE(accountinfo, (id)(tokenCount)(info))

        account_name primary_key() const { return id; }
    };
    static constexpr account_name ACCOUNT_TABLE_NAME = N(accountinfo);
    using account_list = eosio::multi_index<ACCOUNT_TABLE_NAME, accountinfo>;

    // @abi table log i64
    struct log {
        uint64_t id;
	    account_name from;
	    account_name to;
	    std::string memo;
	    uint64_t acttime;

        log() = default;

        EOSLIB_SERIALIZE(log, (id)(from)(to)(memo)(acttime))

        uint64_t primary_key() const { return id; }
    };
    static constexpr uint64_t LOG_TABLE_NAME = N(log);
    using log_list = eosio::multi_index<LOG_TABLE_NAME, log>;

 public:
 
    // @abi action
    void assign(const uint64_t id, account_name newowner);
    
    // @abi action
    void create(const uint64_t id, const std::string category, std::string name, std::string imageUrl, std::string meta);

    // @abi action
    void transfer(const uint64_t id, account_name newowner);

    // @abi action
    void update(const uint64_t id, const std::string name, const std::string category, const std::string imageUrl, const std::string meta);

};
