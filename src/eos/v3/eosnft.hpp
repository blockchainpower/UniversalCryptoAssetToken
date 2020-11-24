#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <math.h>

#define max(a,b) (a>b?a:b)


namespace eosiosystem {
   class system_contract;
}

using namespace eosio;

#define SYSPARAM_TOKEN_COUNT		1
#define SYSPARAM_CONTRACT_ENABLE	2

#define SYSPARAM_ADMIN_ACCOUNT		4
#define CONTRACT_NAME			    6
#define CONTRACT_LOGO			    7


#define HTML_TEMPLATE			10
#define SYSPARAM_VERSION		11



class [[eosio::contract("eosnft")]] eosnft : public contract
{   

public:

   using contract::contract;

   TABLE tokensummary{
	uint64_t  id;
	name	  owner;

	uint64_t primary_key() const { return id; }
   };
   typedef multi_index<"tokensummary"_n, tokensummary> tokensummary_index;

   TABLE sysparam{
        uint64_t	id;
        std::string	tag;
        std::string	val;

        uint64_t primary_key() const { return id; }
   };
   typedef multi_index<"sysparams"_n, sysparam> sysparam_index;
    
 
    TABLE accounts {
        name id;

        uint64_t count;

        uint64_t primary_key() const { return id.value; }
    };
    typedef multi_index<"accounts"_n, accounts> account_index;

    TABLE token {
        uint64_t id;
	std::string uuid;
        std::string title;
        name        owner;
        std::string imageUrl;
        std::string category;
        std::string ext;
        std::string meta;
        bool	    lock;

        //--ext-prop for this contract
        uint64_t	level;
        uint64_t	quality;
        asset		parvalue;
        time_point_sec	stackexpire;  

        uint64_t minepower(){
            return 0;
        }      
	    

        uint64_t primary_key() const { return id; }
	uint64_t get_secondary_1() const { return owner.value;}
        uint64_t get_secondary_2() const { return level;}
	uint64_t get_secondary_3() const { return quality;}
	uint64_t get_secondary_4() const { return parvalue.amount;}
    };
   typedef multi_index<"tokens"_n, token, 
        indexed_by<"byowner"_n, const_mem_fun<token, uint64_t, &token::get_secondary_1>>,
        indexed_by<"bylevel"_n, const_mem_fun<token, uint64_t, &token::get_secondary_2>>,
	indexed_by<"byquality"_n, const_mem_fun<token, uint64_t, &token::get_secondary_3>>,
	indexed_by<"byparval"_n, const_mem_fun<token, uint64_t, &token::get_secondary_4>>
	> token_index;


    TABLE logs {
        uint64_t id;
        name from;
        name to;
        std::string memo;
        uint64_t acttime;
        uint64_t tokenId;
        uint64_t primary_key() const { return id; }
    };
    typedef multi_index<"logs"_n, logs> log_index;


 private:

     double stringtodouble(std::string str) {
        double dTmp = 0.0;
        int iLen = str.length();
        int iPos = str.find(".");
        std::string strIntege = str.substr(0,iPos);
        std::string strDecimal = str.substr(iPos + 1,iLen - iPos - 1 );
        for (int i = 0; i < iPos;i++)
        {
        if (strIntege[i] >= '0' && strIntege[i] <= '9')
        {
        dTmp = dTmp * 10 + strIntege[i] - '0';
        }
        }
        for (int j = 0; j < strDecimal.length(); j++)
        {
        if (strDecimal[j] >= '0' && strDecimal[j] <= '9')
        {
        dTmp += (strDecimal[j] - '0') * pow(10.0,(0 - j - 1));
        }
        }
        return dTmp;
     }


     name get_admin() const {
         const std::string adminAccount = getsysparam(SYSPARAM_ADMIN_ACCOUNT);
         if (adminAccount.empty()) {
             return _self;
         }
         else {
             return name(adminAccount.c_str());
         }
     }

     void require_auth_admin() const { require_auth(get_admin()); }

     void require_auth_contract() const { require_auth( _self );}
     
     inline std::string getsysparam(const uint64_t& key) const {
        sysparam_index sysparams(_self, _self.value);
        auto iter = sysparams.find(key);
        if(iter == sysparams.end()){
                return std::string("");
        }else{
                return iter->val;
        }
    }

    
    inline void setsysparam(const uint64_t& id, const std::string& tag, const std::string& val){
	    sysparam_index sysparams(_self, _self.value);
	    auto iter = sysparams.find(id);
	    if(iter == sysparams.end()){
		    sysparams.emplace(_self, [&](auto& p) {
		        p.id  = id;
		        p.val = val;
			p.tag = tag;
		    });
	    }else{
		    sysparams.modify(iter, _self, [&](auto& p) {
			    p.val = val;
			    p.tag = tag;
		    });
	    }
    }

    inline void addaccounttoken(const name user) {
	    account_index accounts(_self, _self.value);
        auto iter = accounts.find(user.value);
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

    inline void subaccounttoken(const name user) {
        account_index accounts(_self, _self.value);
            auto iter = accounts.find(user.value);
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
        if (str.empty() || str == "") {
            return 0;
        }
        else {
            std::string::size_type sz = 0;
            return std::stoull(str, &sz, 0);
        }
    }
    
    void log(const uint64_t& id, const name& oldowner, const name& newowner, const std::string& opcode, const std::string& ext);

    
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

     [[eosio::action]]
    void init(const std::string adminacc, const std::string title, const std::string image){
        require_auth_contract();
        setsysparam(SYSPARAM_ADMIN_ACCOUNT, "SYSPARAM_ADMIN_ACCOUNT", adminacc);
        setsysparam(CONTRACT_NAME, "CONTRACT_NAME", title);	
        setsysparam(CONTRACT_LOGO, "CONTRACT_LOGO", image);	
    }
 
    [[eosio::action]]
    void assign(const uint64_t id, const name owner, const name newowner);

    [[eosio::action]]
    void reassign(const uint64_t id, const name owner, const name newowner);
       
    [[eosio::action]]
    void create(const uint64_t id, const name owner, std::string title, std::string cate, std::string imageUrl, const bool lock, const uint64_t level, const uint64_t quality , const asset parvalue);

    [[eosio::action]]
    void updatemeta(const uint64_t id, const name owner, const std::string title, const std::string category, const std::string imageUrl, const std::string meta);

    [[eosio::action]]
    void updatelock(const uint64_t id, const name owner, const bool lock);

    [[eosio::action]]
    void updateext(const uint64_t id, const name owner, const std::string ext);

    [[eosio::action]]
    void transfer(const uint64_t id, const name from, const name to, const std::string memo);

    [[eosio::action]]
    void transmk(const uint64_t id, const name from, const name to, const std::string memo);

    [[eosio::action]]
    void setparam(const uint64_t id, const std::string tag, const std::string val){
	    require_auth_contract();
	    setsysparam(id,tag,val);
    }

    [[eosio::action]]
    void burn(const uint64_t id, const name owner){
        require_auth_admin();
        rmtoken_(id, owner);
    }


    [[eosio::action]]
    void rmparam(const uint64_t id){
        require_auth_contract();
        sysparam_index sysparams(_self, _self.value);
        sysparams.erase(sysparams.find(id));
    }

private:
	void rmtoken_(const uint64_t id, const name& owner){	
		token_index tokens(_self, owner.value);
		auto iter = tokens.find(id);
		check(iter != tokens.end(), "token not found");

	        tokens.erase(iter);

		subtokencount();
		subaccounttoken(iter->owner);

		tokensummary_index tokensum(_self, _self.value);
		auto siter = tokensum.find(id);
		if(siter != tokensum.end()){
			tokensum.erase(siter);
		}

		log(id, _self, _self, "BURN", "");
	}

    void notify(const name& to){
	    require_recipient(to);
    }
};
