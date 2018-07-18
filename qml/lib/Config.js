.pragma library
.import QtQuick.LocalStorage 2.0 as LS

var db = LS.LocalStorage.openDatabaseSync("login-flow-demo", "", "login-flow-demo", 100000);
var conf = {};
var mediator = (function(){
    var subscribe = function(channel, fn){
        if(!mediator.channels[channel]) mediator.channels[channel] = [];
        mediator.channels[channel].push({ context : this, callback : fn });
        return this;
    };
    var publish = function(channel){
        if(!mediator.channels[channel]) return false;
        var args = Array.prototype.slice.call(arguments, 1);
        for(var i = 0, l = mediator.channels[channel].length; i < l; i++){
            var subscription = mediator.channels[channel][i];
            subscription.callback.apply(subscription.context.args);
        };
        return this;
    };
    return {
        channels : {},
        publish : publish,
        subscribe : subscribe,
        installTo : function(obj){
            obj.subscribe = subscribe;
            obj.publish = publish;
        }
    };
}());
var init = function(){
    console.log("db.version: "+db.version);
    db.transaction(function(tx) {
       //tx.executeSql('DROP TABLE accounts');
    });

    if(db.version !== "1.1") {
        db.transaction(function(tx) {
            tx.executeSql('CREATE TABLE IF NOT EXISTS accounts ('
                          + ' host TEXT, '
                          + ' login TEXT '
                          + ' token TEXT '
                          + ' userId TEXT '
                          +');');
        });
        db.changeVersion('1.0', '1.1', function(tx) {});
    }
    db.transaction(function(tx) {
        var rs = tx.executeSql('SELECT * FROM accounts;');
        console.log("READING CONF FROM DB")
        for (var i = 0; i < rs.rows.length; i++) {
            console.log(rs.rows.item(i).key+" \t > \t "+rs.rows.item(i).value)
            conf[rs.rows.item(i).key] = JSON.parse(rs.rows.item(i).value)
        }
        console.log("END OF READING")
        console.log(JSON.stringify(conf));
        mediator.publish('confLoaded', { loaded: true});
    });
};

function hasValidInstance() {
    var has = false;
    db.transaction(function(tx) {
       var rs = tx.executeSql('SELECT COUNT(*) FROM accounts WHERE "host" != \'\' AND "login" != \'\' AND "token" != \'\'');
       console.log(rs.rows);
       has = parseInt(rs.rows.item(0).value, 10) > 0;
    });
    // TODO: should also test the login state
    return has;
}
