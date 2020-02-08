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
#define API_URL			                5


class eosnft : public eosio::contract {
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

```
