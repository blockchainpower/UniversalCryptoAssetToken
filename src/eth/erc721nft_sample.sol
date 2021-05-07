// SPDX-License-Identifier: MIT
pragma solidity 0.8.0;

import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC721/extensions/ERC721Enumerable.sol";
import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/access/Ownable.sol";
import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/utils/Counters.sol";
import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/IERC20.sol";



contract ERC721NFT is ERC721Enumerable, Ownable{
    
    using Counters for Counters.Counter;

    Counters.Counter private _nftIds;
    
    
    string private baseUri = "https://ipfs.xloot.io/ipfs/";
    
    mapping (uint256 => string) private tokenHashs;


    constructor() ERC721("NFT SAMPLE", "SAMP") {
        lootDexAddr = owner();
    }
    
    function toAsciiString(address x)  private pure returns (string memory) {
        bytes memory s = new bytes(40);
        for (uint i = 0; i < 20; i++) {
            bytes1 b = bytes1(uint8(uint(uint160(x)) / (2**(8*(19 - i)))));
            bytes1 hi = bytes1(uint8(b) / 16);
            bytes1 lo = bytes1(uint8(b) - 16 * uint8(hi));
            s[2*i] = char(hi);
            s[2*i+1] = char(lo);            
        }
        return string(s);
    }

    function char(bytes1 b) private pure returns (bytes1 c) {
        if (uint8(b) < 10) return bytes1(uint8(b) + 0x30);
        else return bytes1(uint8(b) + 0x57);
    }
    
    
    function concat(string memory str1, string memory str2) private pure returns (string memory){
         bytes memory bs1 = bytes(str1);
         bytes memory bs2 = bytes(str2);

         uint len1 = bs1.length;
         uint len2 = bs2.length;

         string memory temp = new string(len1 + len2);
         bytes memory result = bytes(temp);

         uint index = 0;
         for(uint256 i = 0; i < len1; i++){
             result [index ++] = bs1[i];

         }
         for(uint256 i = 0; i < len2; i ++){
             result [index ++] = bs2 [i];
         }
         return string(result);
     }

    function mint(uint256 tokenId, address receiver, string memory uriHash) external onlyOwner returns (uint256) {
        tokenHashs[tokenId] = uriHash;
        _mint(receiver, tokenId);
        return tokenId;
    }
    
    function mintMyNFT(address receiver, string memory uriHash) public returns (uint256){
        uint256 baseId = 1000000;
        _nftIds.increment();
        uint256 tokenId = baseId + _nftIds.current();
        require(!_exists(tokenId), "token exisit");
        tokenHashs[tokenId] = uriHash;
        _mint(receiver, tokenId);
        return tokenId;
    }
    
    function tokenURI(uint256 tokenId) public view virtual override returns (string memory) {
        require(_exists(tokenId), "ERC721Metadata: URI query for nonexistent token");

        string memory baseURI = _baseURI();
        return bytes(baseURI).length > 0 ? concat(baseURI, tokenHashs[tokenId]) : '';
    }
    
    function _baseURI() override internal view virtual returns (string memory) {
        return baseUri;
    }
    
    function reBaseUri(string memory str) external onlyOwner returns (string memory){
        baseUri = str;
        return str;
    }

    function transferFrom(address from, address to, uint256 tokenId) public virtual override {
        //solhint-disable-next-line max-line-length
        require(_isApprovedOrOwner(_msgSender(), tokenId), concat("ERC721: transfer caller is not owner nor approved : ", toAsciiString(_msgSender())));

        _transfer(from, to, tokenId);
    }
    
    
}
