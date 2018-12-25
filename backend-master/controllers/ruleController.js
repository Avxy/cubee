/**
 * Created by adson on 14/08/17.
 */
var Rule = require('../models/Rule').Rule;
var cubeeController = require('./cubeeController');
var index = require('../index');

/**
 * This function registers a new rule in the database
 * @param jsonRule  Json representing the Rule model
 * @param callback  Callback used to send a response
 */
exports.registerRule = function (jsonRule, callback) {
    new Rule(jsonRule).save(function (error, response){
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, response);
        }
    });
};

/**
 * This function searches and, if any rule was founded, execute all the rules of a specific CUBEE
 * @param idOwner
 * @param previousIdCubee
 * @param typeAlert
 */
exports.searchRulesForCubee = function (idOwner, previousIdCubee, typeAlert) {
    Rule.aggregate([
        {$match: {idOwner: idOwner}},
        {$unwind: '$ruleTasks'},
        {$match: {'$and' : [{'ruleTasks.previousIdCubee':  previousIdCubee},  {'ruleTasks.typeAlert': typeAlert}]}}])
        .exec(function (error, result) {
        if(error){
            index.winston.error(error);
        }else if(result.length > 0){
            executeRule(result);
        }
    });
};

/**
 * This function executes all the rules passed in a list of rule
 * @param rules List of rules to be executed
 */
var executeRule = function (rules){
    rules.forEach(function (rule) {
        cubeeController.setCubeeAppCommand(rule.ruleTasks.idCubee, rule.ruleTasks.taskCommand, function (error, success) {
            if(error){
                index.winston.error(error);
            }else{
                index.winston.info(success);
            }
        });
    });
};

/**
 * This function returns all rules of a specific user
 * @param idOwner    Id of the user in question
 * @param callback   Callback used to send a response
 */
exports.getRulesByUser = function (idOwner, callback) {
    Rule.find({idOwner: idOwner}, function (error, rulesByUser) {
        if(error){
            error.status = 400;
            callback(error);
        }else{
            callback(null, rulesByUser);
        }
    });
};

/**
 * This function deletes a specific rule
 * @param idUser    Id of the owner (user) of the rule
 * @param idRule    Id of the rule to be deleted
 * @param callback  Callback used to send a response
 */
exports.deleteRule = function (idUser, idRule, callback) {
    Rule.remove({_id: idRule, idOwner: idUser}, function (error, rule) {
       if(error){
           callback(error);
       }else{
           callback(null, rule);
       }
    });
};
