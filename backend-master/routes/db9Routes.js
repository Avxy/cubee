/**
 * Created by matheus on 20/09/17.
 */
var db9Router  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /db9/byUser
 * @apiName getDB9RulesByUser
 * @apiGroup DB9Rules
 *
 * @apiDescription This route get all db9 rules from defineJobs user.
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiSuccess {json} List of DB9Rules model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
db9Router.get('/byUser', function (req, res) {
    var idToken = req.header('idToken');
    featureController.getDB9RulesByUser(idToken, function (error, resp) {
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
 * @api {get} /db9/register
 * @apiName registerDB9Rule
 * @apiGroup DB9Rules
 *
 * @apiDescription Use this route to register defineJobs new DB9 Rule
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiParam {json} jsonDBRule  Mandatory  Db9Rule model to register
 * @apiSuccess {json} DB9Rule model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
db9Router.post('/register', function (req, res) {
    var idToken = req.body.idToken;
    var jsonDB9Rule = JSON.parse(req.body.jsonDB9Rule);
    featureController.registerDB9Rule(idToken, jsonDB9Rule, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString().toString());
            res.send(resp);
        }
    });
});

/**
 * @api {get} /db9/set
 * @apiName setDB9RuleToCubee
 * @apiGroup DB9Rules
 *
 * @apiDescription Use this route to send defineJobs Rule to defineJobs Cubee
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiParam {String} ruleId   Mandatory  Id of rule to set
 * @apiParam {String} cubeeId   Mandatory  Id of cubee to set
 * @apiSuccess {json} Cubee model with db9 id.
 *
 * @apiError AuthenticationError
 * @apiError Cubee not found
 * @apiError Rule not found
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
db9Router.post('/set', function (req, res) {
    var db9RuleId = req.body.ruleId;
    var cubeeId = req.body.cubeeId;
    var idToken = req.body.idToken;
    console.log(req);
    featureController.setDB9RuleToCubee(idToken, db9RuleId, cubeeId, function (error, resp) {
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
 * @api {get} /db9/delete
 * @apiName deleteDB9Rule
 * @apiGroup DB9Rules
 *
 * @apiDescription Use this route delete defineJobs DB9 Rule
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiParam {String} ruleId   Mandatory  Id of rule to delete
 * @apiSuccess {json} Cubee model with db9 id.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
db9Router.post('/delete', function (req, res) {
    var db9RuleId = req.body.ruleId;
    var idToken = req.body.idToken;
    featureController.deleteDB9Rule(idToken, db9RuleId, function (error, resp) {
        if(error){
            index.winston.error(error);
            error.status = 400;
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

module.exports = db9Router;