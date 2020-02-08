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

