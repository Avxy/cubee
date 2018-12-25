/**
 * Created by adson on 05/09/17.
 */
var cubeeRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var cubeeController = require('../controllers/cubeeController.js');
var index = require('../index');

cubeeRouter.get('/', function (req, res) {
    var idCubee = req.header("idCubee");

    featureController.getCubeeById(idCubee, function (error, resp) {
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
 * @api {post} /cubee/register
 * @apiName registerCUBEE
 * @apiGroup CUBEE
 *
 * @apiDescription This route register defineJobs CUBEE for defineJobs User
 *
 * @apiParam {String} cubeeName   Mandatory  CUBEE name
 * @apiParam {String} userId      Mandatory  id of the CUBEE owner
 * @apiParam {String} idCubee     Mandatory  CUBEE id
 * @apiParam {String} oldIdCubee  Optional   Old id of the CUBEE if it's alredy registraded
 *
 * @apiSuccess {json} cubee The CUBEE registred
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "162652817",
 *          "name" : "Computer",
 *          "state" : "off",
 *          "idOwner" : "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU2MmVhYmEwZTA2ZmUxNDYzNzQ2YzY5ZWZhOTc0YzExNTIyMzQ2MzQifQ.eyJpc3MiOiJod",
 *          "__v" : 0
 *      }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid token"
 *  }
 */
cubeeRouter.post('/register', function (req, res) {
    var cubeeName = req.body.cubeeName;
    var userId = req.body.userId;
    var idCubee = req.body.idCubee;


    //this is defineJobs CUBEE old id
    var oldIdCubee = req.header("idCubee");

    featureController.registerCubee(cubeeName, idCubee, oldIdCubee, userId, function (error, cubee) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).json({error: error.message});
        } else {
            index.winston.info(cubee);
            res.json({status_register: cubee});
        }
    });
});

/**
 * @api {get} /cubee/byUser
 * @apiName getCubeesByUser
 * @apiGroup CUBEE
 *
 * @apiDescription This route returns the CUBEE's of a specific User
 *
 * @apiHeader (CUBEE) {String} idToken FirebaseToken for the User
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} cubee List of CUBEE's of a specific user.
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "162652817",
 *          "name" : "Computer",
 *          "state" : "off",
 *          "idOwner" : "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU2MmVhYmEwZTA2ZmUxNDYzNzQ2YzY5ZWZhOTc0YzExNTIyMzQ2MzQifQ.eyJpc3MiOiJod",
 *          "__v" : 0
 *      },
 *      {
 *          "_id" : "162652820",
 *          "name" : "Freezer",
 *          "state" : "on",
 *          "idOwner" : "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU2MmVhYmEwZTA2ZmUxNDYzNzQ2YzY5ZWZhOTc0YzExNTIyMzQ2MzQifQ.eyJpc3MiOiJod",
 *          "__v" : 0
 *      }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid token"
 *  }
 */
cubeeRouter.get('/byUser', function (req, res) {
    var idToken = req.header("idToken");

    featureController.getCubeesByOwner(idToken, function (error, resp) {
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
 * @api {get} /cubee/bySector
 * @apiName getCubeesBySector
 * @apiGroup CUBEE
 *
 * @apiDescription This route returns the CUBEE's of a specific Sector
 *
 * @apiHeader (CUBEE) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiHeader (CUBEE) {String} idSector Id of the Sector requested
 * @apiHeaderExample {json} Request-Example:
 *      {"idSector": "59b0294bd59f911bb0be4ec5"}
 *
 * @apiSuccess {json} cubee List of CUBEE's of a specific sector.
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "162652817",
 *          "name" : "Computer",
 *          "state" : "off",
 *          "idOwner" : "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU2MmVhYmEwZTA2ZmUxNDYzNzQ2YzY5ZWZhOTc0YzExNTIyMzQ2MzQifQ.eyJpc3MiOiJod",
 *          "__v" : 0
 *      },
 *      {
 *          "_id" : "162652820",
 *          "name" : "Freezer",
 *          "state" : "on",
 *          "idOwner" : "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU2MmVhYmEwZTA2ZmUxNDYzNzQ2YzY5ZWZhOTc0YzExNTIyMzQ2MzQifQ.eyJpc3MiOiJod",
 *          "__v" : 0
 *      }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid token"
 *  }
 */
cubeeRouter.get('/bySector', function (req, res) {
    var userToken = req.header("idToken");
    var idSector = req.header("idSector");

    featureController.getCubeesBySector(userToken, idSector, function (error, resp) {
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
 * @api {post} /cubee/delete
 * @apiName deleteCUBEE
 * @apiGroup CUBEE
 *
 * @apiDescription This route deletes a specific CUBEE by the id
 *
 * @apiParam {String} idToken   Mandatory  Token of the CUBEE's owner
 * @apiParam {String} idCubee   Mandatory  Id of the CUBEE to be deleted
 *
 * @apiSuccess {json} CUBEE Deleted message
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {"message": "Sucessfull Deleted"}
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid token"
 *  }
 */
cubeeRouter.post('/delete', function (req, res) {

    var idToken = req.body.idToken;
    var idCubee = req.body.idCubee;

    featureController.deleteEventByCubee(idToken, idCubee, function (error) {
        if (error) {
            index.winston.error(error);
            res.send(error);
        } else {
            index.winston.info("delete cubee OK, ID: " + idCubee);
            res.send({"status": "ok"});
        }
    });
});

/**
 * @api {get} /cubee/command
 * @apiName getCubeeCommand
 * @apiGroup CUBEE
 *
 * @apiDescription This route returns the appcommand of a specific CUBEE. It's only used by the CUBEE(embedded)
 *
 * @apiHeader (CUBEE) {String} idCubee Id of the CUBEE in question.
 * @apiHeaderExample {json} Request-Example:
 *      {"idCubee": "59b0294bd59f911bb0be4ec5"}
 *
 * @apiHeader (CUBEE) {String} cubeeState  Actual state of the cubee
 * @apiHeaderExample {json} Request-Example:
 *      {"cubeeState": "1"}
 *
 * @apiHeader (CUBEE) {String} cubeeSignal  Actual signal of the cubee
 * @apiHeaderExample {json} Request-Example:
 *      {"cubeeState": "0"}
 *
 * @apiSuccess {json} appCommand Actual appcommand of the CUBEE. Indicates if the EmbeddedCubee have to do something. 1 to turn on, 2 to turn off and 3 for LED.
 *
 * @apiSuccessExample {json} Success-Response:
 *      {"appCommand": 2}
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not "
 *  }
 */
cubeeRouter.get('/command', function (req, res) {

    var idCubee = req.header("idCubee");
    var state = req.header("cubeeState");
    var signal = req.header("cubeeSignal");
    //GET A COMMAND TO RETURN
    featureController.getCommandForCubee(idCubee, function (error, resp) {
        if (error) {
            index.winston.error(error);
            error.status = 400;
            res.status(error.status).json({error_command: error.message});
        } else {
            //BEFORE RETURN COMMAND, SET INFORMATION WE GOT FROM CUBEE (SIGNAL AND STATE) AT DB MODEL
            featureController.setInformationsCubee(idCubee, state, signal);
            index.winston.info(resp.toString());
            res.json(resp);
        }
    });
});

/**
 * @api {post} /cubee/command
 * @apiName changeCUBEECommand
 * @apiGroup CUBEE
 *
 * @apiDescription This route change the CUBEE command flag
 *
 * @apiParam {String} command   Mandatory  flag representing the actual CUBEE command
 * @apiParam {String} idCubee   Mandatory  Id of the CUBEE to be changed
 *
 * @apiSuccess {json} CUBEE with his actual format
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *          {
 *              name: 'Cubee de Teste',
 *              idOwner: 'L3v7mGW39rPC2L4juzdwhk1fL1g2',
 *              cubeeState: true,
 *              appCommand: true,
 *              validated: true
 *              _id: 59417c5def563123e2e90eff
 *          }
 *
 * @apiError CUBEENoted
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "CUBEE not ed"
 *  }
 */
cubeeRouter.post('/command', function (req, res) {
    var idCubee = req.body.idCubee;
    var command = req.body.command;
    var idToken = req.body.idToken;

    console.log("POSTMAN");
    console.log(req.body);

    if(req.body.listOfIdEvents){
        var listOfIdEvents = JSON.parse(req.body.listOfIdEvents);
        featureController.deleteEvents(idToken, listOfIdEvents, function (error) {
            if (error) {
                index.winston.error(error);
                res.send(error);
            } else {
                cubeeController.setCubeeAppCommand(idCubee, command, function (error, resp) {
                    if (error) {
                        index.winston.error(error);
                        res.status(error.status).send(error.message);
                    } else {
                        index.winston.info(resp.toString());
                        res.send(resp);
                    }
                });
            }
        });
    }else {
        cubeeController.setCubeeAppCommand(idCubee, command, function (error, resp) {
            if (error) {
                index.winston.error("Erro post command" + error);
                res.status(error.status).send(error.message);
            } else {
                index.winston.info("Sucess post command" + resp.toString());
                res.send(resp);
            }
        });
    }
    index.winston.info("ListOfIdEvents" + req.body.listOfIdEvents);
});

/**
 * @api {post} /cubee/measurement
 * @apiName postMeasurement
 * @apiGroup CUBEE
 *
 * @apiDescription This route save defineJobs measurement of a specific CUBEE
 *
 * @apiParam {String} measure   Mandatory  number who represents the measure collected by the CUBEE(Embedded)
 * @apiParam {String} idCubee   Mandatory  Id of the CUBEE to be deleted
 *
 * @apiSuccess {json} Sucess message.
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {"message": "status_ok"}
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not ed"
 *  }
 */
cubeeRouter.post('/measurement', function (req, res) {

    var measure = req.body.measurement;
    var idCubee = req.header('idCubee');

    featureController.saveMeasurement(idCubee, measure, function (error, resp) {
        if (error) {
            res.status(error.status).json({error_measurement :error.message});
        } else {
            res.json({measurement_status: resp});
        }
    });
});

/**
 * @api {get} /cubee/get/measurement
 * @apiName getCubeeMeasurement
 * @apiGroup CUBEE
 *
 * @apiDescription This route returns the measurements of a specific CUBEE.
 *
 * @apiHeader (CUBEE) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiHeader (CUBEE) {String} idCubee Id of the CUBEE in question.
 * @apiHeaderExample {json} Request-Example:
 *      {"idCubee": "59b0294bd59f911bb0be4ec5"}
 *
 * @apiHeader (CUBEE) {String} initDate  The inicial date to start to collect measument
 * @apiHeaderExample {json} Request-Example:
 *      {"initDate": "DATEFORMAT"} Todo:Check with MAtheus
 *
 * @apiHeader (CUBEE) {String} initHour  The inicial hour from the day to start to collect measument
 * @apiHeaderExample {json} Request-Example:
 *      {"initHour": "DATEFORMAT"} Todo:Check with MAtheus
 *
 * @apiHeader (CUBEE) {String} finishDate  The finish date to start to collect measument
 * @apiHeaderExample {json} Request-Example:
 *      {"finishDate": "DATEFORMAT"} Todo:Check with MAtheus
 *
 * @apiHeader (CUBEE) {String} finishHour  The finish hour from the day to start to collect measument
 * @apiHeaderExample {json} Request-Example:
 *      {"finishHour": "DATEFORMAT"} Todo:Check with MAtheus
 *
 * @apiSuccess {json} mesurements according to the initial and finish dates. In hours, days or months, accordin to the calculete interval
 *
 * @apiSuccessExample {json} Success-Response:
 *      todo: CheckFormat
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not ed"
 *  }
 */
cubeeRouter.get('/get/measurement', function (req, res) {
    var idCubee = req.header('idCubee');
    var initDate = req.header('initDate');
    var initHour = req.header('initHour');

    var finishDate = req.header('finishDate');
    var finishHour = req.header('finishHour');

    featureController.getMeasurements(idCubee, initDate, parseInt(initHour),  finishDate, parseInt(finishHour), function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp);
            res.send(resp);
        }
    });
});

/**
 * @api {post} /cubee/alarm
 * @apiName saveCubeeAlarm
 * @apiGroup CUBEE
 *
 * @apiDescription This route save an alarm of a specific CUBEE
 *
 * @apiParam {String} alarms   Mandatory  alarms to save
 * @apiParam {String} idCubee  Mandatory  Id of the CUBEE in case
 *
 * @apiSuccess {json} Sucess message.
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {"alarmeEnviado" : "Alarme Enviado"}
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not ed"
 *  }
 */
cubeeRouter.post('/alarm', function (req, res) {
    var alarms = req.body.alarms;
    var idCubee = req.header("idCubee");
    console.log(alarms);
    featureController.sendAlarmsFromCubee(idCubee, alarms, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).json({error: error.message});
        }else{
            index.winston.info('Alarme Enviado.');
            res.json({alarm_status: resp});
        }
    });

});

/**
 * @api {post} /cubee/threshold
 * @apiName saveCubeeAlarm
 * @apiGroup CUBEE
 *
 * @apiDescription This route save an alarm of a specific CUBEE
 *
 * @apiParam {String} idCubee          Mandatory  Id of the CUBEE in case
 * @apiParam {String} idToken          Mandatory  FirebaseToken for the User
 * @apiParam {Number} upperThreshold   Mandatory  Upper Threshold for measurements alarms
 * @apiParam {Number} lowerThreshold   Mandatory  Lower Threshold for measurements alarms
 *
 * @apiSuccess {json} CUBEE model.
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: Cubee not ed"
 *  }
 */
cubeeRouter.post('/threshold', function (req, res) {
    var idCubee =  req.body.idCubee;
    var idToken = req.body.idToken;
    var upperThreshold = req.body.upperThreshold;
    var lowerThreshold = req.body.lowerThreshold;

    featureController.setThreshold(idToken, idCubee, upperThreshold, lowerThreshold, function (error, resp) {
        if (error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

/**
 * @api {get} /cubee/newId
 * @apiName getCubeeNewId
 * @apiGroup CUBEE
 *
 * @apiDescription This route returns defineJobs new Id used to register defineJobs new CUBEE
 *
 * @apiSuccess {String} new Id
 *
 * @apiSuccessExample {json} Success-Response:
 *      {"appCommand": 2}
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "MongoDBError"
 *  }
 */
cubeeRouter.get('/newId', function (req, res) {
    featureController.getNewId(function (resp) {
        res.send(resp);
    });
});

cubeeRouter.get('/byDB9', function (req, res) {
    var ruleId = req.header('ruleId');
    var idToken = req.header('idToken');
    featureController.getCubeeByDb9(idToken, ruleId, function (error, resp) {
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
 * @api {post} /cubee/output
 * @apiName setCubeeOutput
 * @apiGroup CUBEE
 *
 * @apiDescription This route set an output to a cubee
 *
 * @apiSuccess {json} Cubee updated
 *
 * @apiSuccessExample {json} Success-Response:
 *         {
 *          "_id" : "59cceb70ec419e0f52b40229",
 *          "idOwner" : "GtwQxXo8sIf8vIIHOYAb5j6kmLh2",
 *          "db9RuleHadSetFlag" : true,
 *          "db9RuleId" : "59d4e9c74d982c351886bf23",
 *          "name" : "CUBEE-Einstein",
 *          "validated" : false,
 *          "cubeeBtn" : true,
 *          "appCommand" : 2,
 *          "cubeeState" : false,
 *          "idSector" : null,
 *           "__v" : 0,
 *          "currentOutput" : 0,
 *          "outputHadSetFlag" : false
 *          }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "MongoDBError"
 *  }
 */
cubeeRouter.post('/output', function (req, res) {
    var output = req.body.output;
    var cubeeId = req.body.cubeeId;
    var idToken = req.body.idToken;
    featureController.setOutputToCubee(idToken, output, cubeeId, function (error, resp) {
        if(error){
            index.winston.error(error);
            res.status(error.status).send(error.message);
        }else{
            index.winston.info(resp);
            res.send(resp);
        }
    });
});


module.exports = cubeeRouter;