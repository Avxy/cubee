/**
 * Created by matheus on 20/09/17.
 */
var DB9Rule = require('../models/DB9Rule').DB9Rule;
var Cubee = require('../models/Cubee');

/**
 * Callback all db9 rules of a user or error
 * @param uid      id of user to search
 * @param callback function to return result or error
 */
exports.getDB9RulesByUser = function (uid, callback) {
    DB9Rule.find({idOwner: uid}, function (error, db9RulesByUser) {
        if(error){
            error.status = 400;
            callback(error);
        }else if(db9RulesByUser){
            callback(null, db9RulesByUser);
        }else{
            var noDb9Rules = new Error("noDb9Rules");
            noDb9Rules.status = 400;
            callback(noDb9Rules);
        }
    });
};

/**
 * Save a DB9 Rule in DB.
 * @param jsonDB9Rule Rule to save
 * @param callback    Function to return error or sucess
 */
exports.registerDB9Rule = function (jsonDB9Rule, callback) {
    new DB9Rule(jsonDB9Rule).save(function (error, response){
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, response);
        }
    });
    
};

/**
 * Check if a cubee has a DB9 Rule to return to cubeee
 * Delete the rule after, so CUBEE only receive it once
 * @param cubee    Cubee to search for DB9 Rule
 * @param callback Function to return error or sucess
 */
exports.checkForDB9Rule = function (cubee, callback) {
    if(!cubee.db9RuleHadSetFlag){
        var db9RuleId = cubee.db9RuleId;
        cubee.db9RuleHadSetFlag = true;
        cubee.save(function (error) {
            if(error){
                callback(error);
            }else{
                DB9Rule.findById(db9RuleId, function (error, db9Rule) {
                    if(error){
                        callback(error);
                    }else if (db9Rule){
                        callback(null, db9Rule);
                    }else{
                        var cantFindDB9 = new Error("Cant find DB9 Rule");
                        cantFindDB9.status = 400;
                        callback(cantFindDB9);
                    }
                });
            }
        });
    }else{
        var unknownError = new Error("DB9RULE already sanded ");
        unknownError.status = 400;
        callback(unknownError);
    }
};

/**
 * Search in DB for a rule and return it with callback
 * @param ruleId   id of rule to search
 * @param callback Function to return Rule or Error
 */
exports.getRuleById = function (ruleId, callback) {
    if(ruleId){
        DB9Rule.findById(ruleId, function (error, rule) {
            if(error){
                callback(error);
            }else{
                callback(null, rule);
            }
        });
    }
};

/**
 * Delete a DB9Rule with callback
 * @param db9RuleId  Id of Rule to delete
 * @param callback   callback used to return error or sucess
 */
exports.deleteDB9Rule = function (db9RuleId, callback) {
    DB9Rule.remove({_id: db9RuleId}, function (error, rule) {
        if(error){
            callback(error);
        }else{
            callback(null, rule);
        }
    });

};
