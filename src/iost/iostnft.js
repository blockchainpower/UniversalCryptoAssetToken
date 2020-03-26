// const xpetUser = "alexandrite";
// const xpetgame = "xpetgame";

const tokenPrefix = "token_";
const userTokenPrefix = "user_";
const MIGRATE_KEY = "migrate_info";
const INFO_KEY = "nft_info";
const LOG_EXPIRE_SECOND = 3600000;



class NFT {
    init() {}

    onlyAllowContract() {
        return blockchain.requireAuth(blockchain.contractOwner(), "active");
    }
    _requireAuth() {
        const ret = blockchain.requireAuth(blockchain.contractOwner(), "active");
        if (ret != true) {
            throw "require auth error"
            throw new Error("require auth failed. ret = " + ret + "owner = " + blockchain.contractOwner());
        }
    }

    _requireOwner(owner) {
        const ret = blockchain.requireAuth(owner, "active");
        if (ret != true) {
            throw new Error("require auth failed. ret = " + ret + "owner = " + owner);
        }
    }

    can_update(data) {
        this._requireAuth();
        return true;
    }
    _get(k) {
        const val = storage.get(k);
        if (val === "") {
            return 0;
        } else {
            return JSON.parse(val);
        }

    }
    _put(k, v, p) {
        storage.put(k, JSON.stringify(v), p);
    }
    _remove(k) {
        storage.del(k);
    }

    _msg(code, msg, obj) {
        var success;
        if (code == 200) {
            success = true;
        } else {
            success = false;
        }
        var message = {
            code: code,
            message: msg,
            success: success,
            object: obj
        }
        return message;
    }

    _clearLog() {
        var minlogNum = this._get("minlogNum");
        var maxlogNum = this._get("maxlogNum");
        var logInfo = this._get("log" + minlogNum);
        if (logInfo) {
            var nowTime = (tx.time / 1000000);
            var timeInterval = nowTime - logInfo.acttime;
            if (maxlogNum - minlogNum > 1000) {
                this._remove("log" + minlogNum);
            }
        }
        this._put("minlogNum", (minlogNum + 1));
        this._nftUpdate();
    }

    _nftUpdate() {
        var minlogNum = this._get("minlogNum");
        var maxlogNum = this._get("maxlogNum");
        var tokenTotal = this._get("tokenTotal");
        var nft = {
            "TOKEN_COUNT": 0,
            "MIN_LOG_ID": 0,
            "MAX_LOG_ID": 0,
            "API_URL": "https//nftapi.xpet.io/",
            "CONTRACT_NAME": "XPET",
            "CONTRACT_LOGO": "https://images-1251625178.cos.ap-guangzhou.myqcloud.com/xpet128.png"
        };

        if (minlogNum) {
            nft.MIN_LOG_ID = minlogNum;
        }
        if (maxlogNum) {
            nft.MAX_LOG_ID = maxlogNum;
        }
        if (tokenTotal) {
            nft.TOKEN_COUNT = tokenTotal;
        }

        this._put(INFO_KEY, nft);
    }

    _blackCheck(user) {
        var blacklist = this._get("blacklist");
        if (blacklist) {
            var existed = false;
            for (var i = 0; i < blacklist.length; i++) {
                if (blacklist[i] == user) {
                    existed = true;
                }
            }
            if (existed == true) {

                throw "Your account is on the blacklist and cannot be transferred";
            }
        }
    }

    _migrateCheck(id) {
        var migratelist = this._get(MIGRATE_KEY);
        if (migratelist) {
            var existed = false;
            for (var i = 0; i < migratelist.length; i++) {
                if (migratelist[i].id == id) {
                    existed = true;
                }
            }
            if (existed == true) {
                throw "The asset is in the process of applying for transfer";
            }
        }
    }



    addLog(log) {
        var maxlogNum = this._get("maxlogNum");
        var minlogNum = this._get("minlogNum");

        this._put("log" + (maxlogNum + 1), log);
        this._put("maxlogNum", (maxlogNum + 1));

        if (minlogNum) {
            this._clearLog();
        } else {
            this._put("minlogNum", 0);
        }
    }

    rmLog() {
        var maxlogNum = this._get("maxlogNum");
        var minlogNum = this._get("minlogNum");

        if (minlogNum && maxlogNum) {
            if (maxlogNum - minlogNum > 1000) {
                this._remove("log" + minlogNum);
                this._put("minlogNum", (minlogNum + 1));
                this._nftUpdate();
            }
        }
    }

    issue(id, newowner) {
        this._requireAuth();
        var contractOwner = blockchain.contractOwner();
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            if (tokeninfo.owner == contractOwner) {

                var userTokenNum = this._get(userTokenPrefix + newowner);
                if(tokeninfo.owner == newowner){
                    throw "New account is the same as old account";
                }
                tokeninfo.owner = newowner;
                var nowTime = (tx.time / 1000000);
                var log = {
                    from: contractOwner,
                    to: newowner,
                    memo: "ASSIGN-" + id,
                    acttime: nowTime,
                    tokenId: id
                }

                this._put(tokenPrefix + id, tokeninfo);
                this._put(userTokenPrefix + newowner, (userTokenNum + 1));

                this.addLog(log);
                this._nftUpdate();
                return this._msg(200, "success");
            } else {
                return this._msg(0, "token setup failed");
            }

        } else {
            return this._msg(0, "token does not exist");
        }
    }

    reassign(id, newowner) {
        this._requireAuth();
        var contractOwner = blockchain.contractOwner();
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            var from = tokeninfo.owner;
            var userTokenNum = this._get(userTokenPrefix + newowner);
            var oldOwnerTokenNum = this._get(userTokenPrefix + from);

            if(tokeninfo.owner == newowner){
                throw "New account is the same as old account";
            }
            tokeninfo.owner = newowner;
            var nowTime = (tx.time / 1000000);
            var log = {
                from: contractOwner,
                to: newowner,
                memo: "ASSIGN-" + id,
                acttime: nowTime,
                tokenId: id
            }

            this._put(tokenPrefix + id, tokeninfo);
            this._put(userTokenPrefix + newowner, (userTokenNum + 1));
            this._put(userTokenPrefix + from, (oldOwnerTokenNum - 1));

            this.addLog(log);
            this._nftUpdate();
            return this._msg(200, "success");

        } else {
            return this._msg(0, "token does not exist");
        }
    }


    create(id, uuid, category, name, imageUrl, meta, lock, ext) {
        this._requireAuth();

        if (this._get(tokenPrefix + id)) {
            return this._msg(0, "token existed");
        }

        var owner = blockchain.contractOwner();
        var tokenMsg = {
            id: id,
            uuid: uuid,
            category: category,
            name: name,
            imageUrl: imageUrl,
            meta: meta,
            lock: lock,
            ext: ext,
            owner: owner
        }
        var tokenTotal = this._get("tokenTotal");
        var nowTime = (tx.time / 1000000);
        var log = {
            from: owner,
            to: owner,
            memo: "CREATE-" + id,
            acttime: nowTime,
            tokenId: id
        }

        this._put(tokenPrefix + id, tokenMsg);
        this._put("tokenTotal", (tokenTotal + 1));
        this.addLog(log);
        this._nftUpdate();
        return this._msg(200, "success");

    }

    updatemeta(id, category, name, imageUrl, meta) {
        this._requireAuth();
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            var tokenMsg = {
                id: id,
                uuid: tokeninfo.uuid,
                category: category,
                name: name,
                imageUrl: imageUrl,
                meta: meta,
                lock: tokeninfo.lock,
                ext: tokeninfo.ext,
                owner: tokeninfo.owner
            }
            var nowTime = (tx.time / 1000000);
            var log = {
                from: tokeninfo.owner,
                to: tokeninfo.owner,
                memo: "UPDATE-" + id,
                acttime: nowTime,
                tokenId: id
            }

            this._put(tokenPrefix + id, tokenMsg);
            this.addLog(log);
            this._nftUpdate();
            return this._msg(200, "success");

        } else {
            return this._msg(0, "token does not exist");
        }
    }

    updateext(id, ext) {
        this._requireAuth();
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            var tokenMsg = {
                id: id,
                uuid: tokeninfo.uuid,
                category: tokeninfo.category,
                name: tokeninfo.name,
                imageUrl: tokeninfo.imageUrl,
                meta: tokeninfo.meta,
                lock: tokeninfo.lock,
                ext: ext,
                owner: tokeninfo.owner
            }
            var nowTime = (tx.time / 1000000);
            var log = {
                from: tokeninfo.owner,
                to: tokeninfo.owner,
                memo: "UPDATE-" + id,
                acttime: nowTime,
                tokenId: id
            }

            this._put(tokenPrefix + id, tokenMsg);
            this.addLog(log);
            this._nftUpdate();
            return this._msg(200, "success");

        } else {
            return this._msg(0, "token does not exist");
        }
    }

    updatelock(id, lock) {
        this._requireAuth();

        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            tokeninfo.lock = lock;

            var nowTime = (tx.time / 1000000);
            var log = {
                from: tokeninfo.owner,
                to: tokeninfo.owner,
                memo: "UPDATE-" + id,
                acttime: nowTime,
                tokenId: id
            }

            this._put(tokenPrefix + id, tokeninfo);
            this.addLog(log);
            this._nftUpdate();
            return this._msg(200, "success");

        } else {
            return this._msg(0, "token does not exist");
        }
    }

    //amount always 1
    transfer(id, from, to, amount, memo) {
        this._requireOwner(from);
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            if (tokeninfo.owner == from) {


                var tokenMsg = {
                    from: from,
                    to: to,
                    memo: memo
                }

                if (tokeninfo.lock == true) {
                    return this._msg(0, "token is locked");
                }

                this._blackCheck(from);
                this._migrateCheck(id);

                var oldOwnerTokenNum = this._get(userTokenPrefix + from);
                var newOwnerTokenNum = this._get(userTokenPrefix + to);
                tokeninfo.owner = to;
                var nowTime = (tx.time / 1000000);
                var log = {
                    from: from,
                    to: to,
                    memo: "TRANSFER-" + id + "-" + memo,
                    acttime: nowTime,
                    tokenId: id
                }

                this._put(userTokenPrefix + from, (oldOwnerTokenNum - 1));
                this._put(userTokenPrefix + to, (newOwnerTokenNum + 1));
                this._put(tokenPrefix + id, tokeninfo);


                this.addLog(log);
                this._nftUpdate();
                return this._msg(200, "success");

            } else {
                return this._msg(0, "token does not belong to you");
            }

        } else {
            return this._msg(0, "token does not exist");
        }
    }

    rmtoken(id) {
        var formUser = blockchain.publisher();
        this._requireOwner(formUser);
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {
            var userTokenNum = this._get(userTokenPrefix + tokeninfo.owner);
            var tokenTotal = this._get("tokenTotal");
            var nowTime = (tx.time / 1000000);
            var log = {
                from: formUser,
                to: formUser,
                memo: "REMOVE-" + id,
                acttime: nowTime,
                tokenId: id
            }


            this._put(userTokenPrefix + tokeninfo.owner, (userTokenNum - 1));
            this._put("tokenTotal", (tokenTotal - 1));

            this._remove(tokenPrefix + id);
            this.addLog(log);
            this._nftUpdate();
            return this._msg(200, "success");
        } else {
            return this._msg(0, "token does not exist");
        }
    }

    ownerOf(tokenId) {
        var tokeninfo = this._get(tokenPrefix + tokenId);
        if (tokeninfo) {
            return this._msg(200, "success", tokeninfo.owner);
        } else {
            return this._msg(0, "token does not exist");
        }
    }

    tokenMetadata(tokenId) {
        var tokeninfo = this._get(tokenPrefix + tokenId);
        if (tokeninfo) {
            return this._msg(200, "success", tokeninfo.meta);
        } else {
            return this._msg(0, "token does not exist");
        }
    }

    tokenInfo(tokenId) {
        var tokeninfo = this._get(tokenPrefix + tokenId);
        if (tokeninfo) {
            return this._msg(200, "success", tokeninfo);
        } else {
            return this._msg(0, "token does not exist");
        }
    }

    nftInfo() {
        var nftInfo = this._get(sysparam);
        if (nftInfo) {
            return this._msg(200, "success", nftInfo);
        } else {
            return this._msg(0, "No information");
        }
    }

    addblack(acc) {

        this._requireAuth();

        var blacklist = this._get("blacklist");
        if (blacklist) {
            var existed = false;
            for (var i = 0; i < blacklist.length; i++) {
                if (blacklist[i] == acc) {
                    existed = true;
                }
            }
            if (existed == false) {
                blacklist[blacklist.length] = acc;
                this._put("blacklist", blacklist);
                return this._msg(200, "success");
            } else {
                return this._msg(0, "Account is already blacklisted");
            }
        } else {
            blacklist = [];
            blacklist[0] = acc;
            this._put("blacklist", blacklist);
            return this._msg(200, "success");
        }

    }

    rmblack(acc) {

        this._requireAuth();

        var blacklist = this._get("blacklist");
        if (blacklist) {
            var existed = false;
            for (var i = 0; i < blacklist.length; i++) {
                if (blacklist[i] == acc) {
                    existed = true;
                    blacklist.splice(i, 1);
                }
            }
            if (existed == true) {
                this._put("blacklist", blacklist);
                return this._msg(200, "success");
            } else {
                return this._msg(0, "Account is not on the blacklist");
            }
        } else {
            return this._msg(0, "param does not exist");
        }
    }

    applymigrate(id, target, account) {
        var formUser = blockchain.publisher();
        this._requireOwner(formUser);
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            if (tokeninfo.owner == formUser) {


                if (tokeninfo.lock == true) {
                    return this._msg(0, "token is locked");
                }

                this._blackCheck(formUser);

                var userMigrate = {
                    id: id,
                    account: account,
                    target: target
                }
                var migratelist = this._get(MIGRATE_KEY);
                if (migratelist) {
                    if (migratelist.length >= 32) {
                        return this._msg(0, "migrate queue is full");
                    }
                    var existed = false;

                    for (var i = 0; i < migratelist.length; i++) {
                        if (migratelist[i].id == id) {
                            existed = true;
                        }
                    }

                    if (existed == false) {
                        migratelist[migratelist.length] = userMigrate;
                        this._put(MIGRATE_KEY, migratelist);
                        return this._msg(200, "success");
                    } else {
                        return this._msg(0, "You have submitted it, please wait for review");
                    }
                } else {
                    migratelist = [];
                    migratelist[0] = userMigrate;
                    this._put(MIGRATE_KEY, migratelist);
                    return this._msg(200, "success");
                }

            } else {
                return this._msg(0, "token does not belong to you");
            }

        } else {
            return this._msg(0, "token does not exist");
        }



    }

    apprmigrate(id, approve) {
        this._requireAuth();
        var migratelist = this._get(MIGRATE_KEY);

        if (migratelist) {
            var existed = false;

            for (var i = 0; i < migratelist.length; i++) {
                if (migratelist[i].id == id) {
                    existed = true;
                    migratelist.splice(i, 1);
                }
            }

            if (existed == true) {
                this._put(MIGRATE_KEY, migratelist);

                if (approve == "True") {
                    return this.burn(id);
                }else{
					return this._msg(200, "success reject=" + approve);
				}

            } else {
                return this._msg(0, "No such apply");
            }
        } else {
            return this._msg(0, "migratelist empty");
        }
    }


    burn(id) {
        this._requireAuth();
        var tokeninfo = this._get(tokenPrefix + id);
        if (tokeninfo) {

            var owner = tokeninfo.owner;
            var tokenTotal = this._get("tokenTotal");
            var userTokenTotal = this._get(userTokenPrefix + owner);
            var nowTime = (tx.time / 1000000);
            var log = {
                from: blockchain.contractOwner(),
                to: blockchain.contractOwner(),
                memo: "BURN-" + id,
                acttime: nowTime,
                tokenId: id
            }

            this._remove(tokenPrefix + id);
            this._put(userTokenPrefix + owner, (userTokenTotal - 1));
            this._put("tokenTotal", (tokenTotal - 1));
            this.addLog(log);
            this._nftUpdate();

            return this._msg(200, "success burn");
        } else {
            return this._msg(0, "token does not exist");
        }
    }

}

module.exports = NFT;
