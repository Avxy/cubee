/**
 * Created by adson on 14/09/17.
 */
var ruleRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /rule
 * @apiName getRules
 * @apiGroup Rule
 *
 * @apiDescription This route returns all the rules of a specific User
 *
 * @apiHeader (Rule) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} Rules  an array of AlarmModels
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
ruleRouter.get('/', function (req, res) {
    var idToken = req.header('idToken');
    featureController.getRulesByUser(idToken, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

/**
 * @api {post} /rule/register
 * @apiName registerRule
 * @apiGroup Rule
 *
 * @apiDescription This route register an rule on the database.
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiParam {json}   jsonRule  Mandatory  Rule to register

 * @apiSuccess {json} Event model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
ruleRouter.post('/register', function (req, res) {
    var jsonRule = JSON.parse(req.body.jsonRule);
    var idToken = req.body.idToken;

    featureController.registerRule(idToken, jsonRule, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

/**
 * @api {post} /rule/delete
 * @apiName deleteRule
 * @apiGroup Rule
 *
 * @apiDescription This route deletes a specific rule created by a specific user.
 *
 * @apiParam {String} idRule   Mandatory  Id of the rule in case
 * @apiParam {String} idToken  Mandatory  FirebaseToken for the User

 * @apiSuccess {json} Rule model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
ruleRouter.post('/delete', function (req, res) {
    var idRule = req.body.idRule;
    var idToken = req.body.idToken;

    featureController.deleteRule(idToken, idRule, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

module.exports = ruleRouter;