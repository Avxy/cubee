/**
 * Created by adson_silva on 17/09/17.
 */
var eventRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /event
 * @apiName getEvents
 * @apiGroup Event
 *
 * @apiDescription This route returns all the events of a specific User
 *
 * @apiHeader (Event) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} Events  An array of EventModels
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
eventRouter.get('/', function (req, res) {
    var idToken = req.header('idToken');
    featureController.getEventsByUser(idToken, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });

});

/**
 * @api {post} /event/register
 * @apiName registerEvent
 * @apiGroup Event
 *
 * @apiDescription This route register an event on the database.
 *
 * @apiParam {String} idToken   Mandatory  FirebaseToken for the User
 * @apiParam {json}   event     Mandatory  Event to register

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
eventRouter.post('/register', function (req, res) {
    var idToken = req.body.idToken;
    var event = JSON.parse(req.body.event);
    featureController.registerEvent(idToken, event, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.send(error);
        } else if (resp) {
            index.winston.info('success - router post');
            res.send({'status': 'success'});
        } else {
            var unknowError = new Error("Unknow error");
            unknowError.status = 400;
            index.winston.error(error);
            resp.send(unknowError);
        }
    });
});

/**
 * @api {get} /event/byCubee
 * @apiName getEventByCubee
 * @apiGroup Event
 *
 * @apiDescription This route returns all the events of a specific CUBEE
 *
 * @apiHeader (Event) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiHeader (Event) {String} idCubee Id of the CUBEE requested
 * @apiHeaderExample {json} Request-Example:
 *      {"idCubee": "59b0294bd59f911bb0be4ec5"}
 *
 * @apiSuccess {json} Events  an array of EventModels
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: CUBEE not found"
 *  }
 */
eventRouter.get('/byCubee', function (req, res) {
    var idToken = req.header("idToken");
    var idCubee = req.header("idCubee");
    featureController.getEventsByCubeeAndCurrentTime(idToken, idCubee, function (error, resp) {
        if (error) {
            console.log("error");
            index.winston.error("Events by cubee error" + error.message);
            res.status(error.status).send(error.message);
        } else {
            console.log(resp);
            index.winston.info("Events by cubee succes" + resp.toString());
            res.send(resp);
        }
    });
});

/**
 * @api {post} /event/delete
 * @apiName deleteEvent
 * @apiGroup Event
 *
 * @apiDescription This route deletes a specific event from a specific user.
 *
 * @apiParam {String} idEvent          Mandatory  Id of the event in case
 * @apiParam {String} idToken          Mandatory  FirebaseToken for the User

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
eventRouter.post('/delete', function (req, res) {
    var idEvent = req.body.idEvent;
    var idToken = req.body.idToken;

    featureController.deleteEvent(idToken, idEvent, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});
module.exports = eventRouter;