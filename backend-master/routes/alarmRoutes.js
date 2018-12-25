/**
 * Created by adson on 14/09/17.
 */
var alarmRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /alarm
 * @apiName getAlarms
 * @apiGroup Alarm
 *
 * @apiDescription This route returns all the alarms of a specific User
 *
 * @apiHeader (CUBEE) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} Alarms  an array of AlarmModels
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
alarmRouter.get('/', function (req, res) {
    var idToken = req.header("idToken");
    featureController.getNotificationByUser(idToken, function (error, notifications) {
        if(error){
            index.winston.error(error);
            res.send(error);
        }else if (notifications){
            index.winston.info(notifications);
            res.send(notifications);
        }
    });
});

alarmRouter.get('/filter', function (req, res) {
    var idToken = req.header("idToken");
    var filter = req.header("filter");
    filter = JSON.parse(filter);
    featureController.getNotificationByUserWithFilter(idToken, filter, function (error, notifications) {
        if(error){
            index.winston.error(error);
            res.send(error);
        }else if (notifications){
            index.winston.info(notifications);
            res.send(notifications);
        }
    });
});


/**
 * @api {post} /alarm/delete
 * @apiName deleteAlarm
 * @apiGroup Alarm
 *
 * @apiDescription This route deletes a specific alarm from a specific user.
 *
 * @apiParam {String} idAlarm          Mandatory  Id of the alarm in case
 * @apiParam {String} idToken          Mandatory  FirebaseToken for the User

 * @apiSuccess {json} Alarm model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not found"
 *  }
 */
alarmRouter.post('/delete', function(req, res){
    var idAlarm = req.body.idAlarm;
    var idToken = req.body.idToken;
    featureController.deleteAlarm(idAlarm, idToken, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.send(error);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});


/**
 * @api {post} /alarm/delete/all
 * @apiName deleteAlarm
 * @apiGroup Alarm
 *
 * @apiDescription This route deletes all alarms from an especific user.
 *
 * @apiParam {String} idToken          Mandatory  FirebaseToken for the User

 * @apiSuccess {json} Alarm model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
alarmRouter.post('/delete/all', function(req, res){
    var idToken = req.body.idToken;

    featureController.deleteAllAlarms(idToken, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.send(error);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

module.exports = alarmRouter;