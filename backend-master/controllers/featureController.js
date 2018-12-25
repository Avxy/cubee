/**
 * Created by adson on 17/05/17.
 */
var userController = require('./userController.js');
var cubeeController = require('./cubeeController.js');
var sectorController = require('./sectorController');
var eventController = require('./eventController');
var notificationController = require('./notificationController');
var ruleController = require('./ruleController');
var db9RuleController = require('./db9RuleController');
var measurementController = require('./measurementController.js');
var mqttClientController = require('./mqttClientController');


global.RESTART_ALERT_TYPE = "RESTART";
global.RFID_ALERT_TYPE = "RFID";
global.ACTIVATION_ALERT_TYPE = "ACTIVATION";
global.DEACTIVATION_ALERT_TYPE = "DEACTIVATION";
global.UPPER_THRESHOLD_ALERT_TYPE = "UPPER_THRESHOLD";
global.LOWER_THRESHOLD_ALERT_TYPE = "LOWER_THRESHOLD";
global.SIGNAL_ALERT_TYPE = "SIGNAL";
global.UNKNOWN_ALERT_TYPE = "UNKNOWN";

const CHANGE_CUBEE_SIGNAL = "1";

var index = require('../index');
/*----------------------------------------------
---------------USER RELATED METHODS-------------
----------------------------------------------*/
/**
 * This function delegates to the userController for user registration by Facebook
 *
 * @param idToken   User token
 * @param callback  callback used to send defineJobs response
 */
exports.registerFbUser = function (idToken, callback) {
    userController.registerFbUser(idToken, callback);
};
/**
 * This functions delegates to the userController for user registration by email and password
 * @param name       User name
 * @param email      User email
 * @param telephone  User telephone
 * @param password   User password
 * @param callback   callback used to send a response
 */
exports.registerEPUser = function (name, email, telephone, password, callback) {
    userController.registerEPUser(name, email, telephone, password, callback);
};

/**
 * This function returns a user, based on his FirebaseToken, besides performing the error handling
 *
 * @param token     User token
 * @param callback  Callback used to send a response
 */
exports.getUser = function (token, callback) {
    userController.getUid(token, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            userController.getUser(uid, callback);
        }
    });
};

/**
 * This function delegates to the userController to edit user's information.
 *
 * @param idToken    User token
 * @param name       User name
 * @param email      User email
 * @param telephone  User telephone
 * @param password   User password
 * @param callback   Callback used to send a repsonse.
 */
exports.editUser = function (idToken, name, email, telephone, password, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            userController.editUser(uid, name, email, telephone, password, callback);
        }
    });
};

/*-----------------------------------------------------
---------------MESUREMENTS RELATED METHODS-------------
-----------------------------------------------------*/

/**
 * This function delegates do merasurementController to save measurements
 *
 * @param idCubee  Id of the CUBEE to save the measurement
 * @param measure  Measurement to be saved in the CUBEE
 * @param callback Callback used to send a response.
 */
exports.saveMeasurement = function (idCubee, measure, callback) {
    measurementController.saveMeasurement(idCubee, measure, callback);
};

/**
 * This function delegates to measurementController to reset measurements
 *
 * @param callback Callback used to send a response.
 */
exports.resetMeasurement = function (callback) {
    measurementController.resetMeasurement(callback);
};

/**
 * This function calculates the interval of initial end finish date and returns the measurements of a specific CUBEE in the correct interval;
 * In minutes, hours, days or months
 *
 * @param idCubee    The id of the CUBEE to get the measurements
 * @param initDate   The init date of the interval
 * @param initHour   The init hour of the interval
 * @param finishDate The finish date of the interval
 * @param finishHour The finish hour of the interval
 * @param callback   Callback used to send a response
 */
exports.getMeasurements = function (idCubee, initDate, initHour, finishDate, finishHour, callback) {
    var startDay = new Date(initDate);
    var endDay = new Date(finishDate);
    startDay.setHours(initHour);
    endDay.setHours(finishHour);

    var diffMilisseconds = endDay - startDay;
    var diffSeconds = diffMilisseconds / 1000;
    var diffMinutes = diffSeconds / 60;
    var diffHours = diffMinutes / 60;
    var diffDays = diffHours / 24;


    startDay.setHours(0);
    startDay.setMinutes(0);
    startDay.setSeconds(0);
    startDay.setMilliseconds(0);

    endDay.setHours(0);
    endDay.setMinutes(0);
    endDay.setSeconds(0);
    endDay.setMilliseconds(0);

    // var endDay = new Date(finishDate);

    if (diffHours <= 0) {
        measurementController.getCubeeMeasurementsInMinutes(idCubee, startDay, initHour, finishHour, callback);
    }
    else if (diffHours >= 0 && diffHours <= 48) {
        measurementController.getCubeeMeasurementsInHours(idCubee, startDay, initHour, endDay, finishHour, callback);
    } else if (diffDays > 2 && diffDays <= 62) {
        measurementController.getCubeeMeasurementsInDays(idCubee, startDay, endDay, callback);
    } else {
        measurementController.getCubeeMeasurementsInMonths(idCubee, startDay, endDay, callback);
    }
};

/*-----------------------------------------------
---------------CUBEE RELATED METHODS-------------
-----------------------------------------------*/
/**
 * This function delegates to the cubeeController for CUBEE registration
 *
 * @param cubeeName  CUBEE name
 * @param cubeeId    CUBEE id
 * @param idToken    Token of the CUBEE's owner
 * @param callback   Callback used to send a response
 */
exports.registerCubee = function (cubeeName, cubeeId, oldIdCubee, idOwner, callback) {
    cubeeController.registerCubee(cubeeName, cubeeId, oldIdCubee, idOwner, function (error, cubee) {
        if (error) {
            callback(error);
        } else {
            var command = {};
            command.appCommand = 7;
            mqttClientController.sendCommandJson(oldIdCubee, command);
            callback(cubee);
        }
    });
};

/**
 * This function delegate to the CubeeController to get a CUBEE list of the one specific user by his idToken
 * @param token    Token of the CUBEE owner
 * @param callback Callback used to send a response
 */
exports.getCubeesByOwner = function (token, callback) {
    userController.getUid(token, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            cubeeController.getCubeesByOwner(uid, callback);
        }
    });
};

/**
 * This function delegate to the userController to get all the CUBEEs in a specific sector
 * @param tokenUser  User token of the owner of the cubees
 * @param idSector   Id of the sector
 * @param callback   Callback used to send a response.
 */
exports.getCubeesBySector = function (tokenUser, idSector, callback) {
    userController.getUid(tokenUser, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            if (idSector) {
                cubeeController.getCubeesBySector(uid, idSector, callback);
            } else {
                cubeeController.getCubeesWithNoSector(uid, callback);
            }
        }
    });
};

/**
 * This function delegates to the cubeeController for get a CUBEE by id
 *
 * @param idCubee   CUBEE id
 * @param callback  Callback used to send a response
 */
exports.getCubeeById = function (idCubee, callback) {

    cubeeController.getCubeeById(idCubee, callback);

};

/**
 * This function delegates to cubeeController to get the id of the owner(User) of the CUBEE
 * @param idCubee   Id of the CUBEE in question
 * @param callback  Callback used to send a response
 */
exports.getOwnerID = function (idCubee, callback) {
    cubeeController.getOwnerId(idCubee, callback);
};

/**
 * This function delegates to the cubeeControler for delete a CUBEE
 * @param token     Token of the CUBEE owner
 * @param idCubee   CUBEE id
 * @param callback  Callback used to send a response
 */
exports.deleteCubee = function (token, idCubee, callback) {
    userController.getUid(token, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            cubeeController.deleteCubee(idCubee, uid, callback);
        }
    });

};

/**
 * This function choose to delegate to sets the signal and state of the CUBEE
 * @param idCubee  Id of the CUBEE to be setted
 * @param state    State to be setted on the CUBEE
 * @param signal   Signal to be setted on the CUBEE
 */
exports.setInformationsCubee = function(idCubee, state, signal){
    if (signal && (signal === CHANGE_CUBEE_SIGNAL)) {
        setCubeeSignal(idCubee, signal, function (error, setSignalResponse) {
            if (error) {
                index.winston.error("Erro: " + error.toString());
            } else {
                index.winston.info(setSignalResponse.toString());
            }
        });
    }
    if (state) {
        setCubeeState(idCubee, state, function (error, setStateResponse) {
            if (error) {
                index.winston.error("Erro: " + error.message);
            } else {
                index.winston.info(setStateResponse.toString());
            }
        });
    }
};

/**
 * This function verifies if the change is necessary and delegates to a specific function to set the CUBEE signal
 * @param idCubee  Id of the CUBEE to be setted
 * @param signal   Signal to be setted in the CUBEE
 * @param callback Callback used to send a response
 */
exports.setCubeeSignal = function (idCubee, signal, callback) {
    if (signal && (signal == CHANGE_CUBEE_SIGNAL)) {
        setCubeeSignal(idCubee, signal, callback);
    }
};

/**
 * This function verifies if the change is necessary and delegates to a specific function to set the CUBEE state
 * @param idCubee   Id of the CUBEE to be setted
 * @param state     State to be setted in the CUBEE
 * @param callback  Callback used to send a response
 */
exports.setCubeeState = function (idCubee, state, callback) {
    if (state !== undefined) {
        setCubeeState(idCubee, state, callback);
    }else{
        var invalidMessage = new Error("No State");
        invalidMessage.status = 400;
        callback(invalidMessage);
        callback(invalidMessage)
    }
};

/**
 * This function is used for the CUBEE(Embedded) for get any command of then. From task, DB9, Model or Event
 *
 * @param idCubee   id of CUBEE requested
 * @param callback  Callback used to send a response
 */
exports.getCommandForCubee = function (idCubee, callback) {
    //TRIES TO GET COMMAND FROM TASK
    getCommandTask(idCubee, function (error, taskResponse) {
        if (error) {
            //TRIES TO GET COMMAND FROM DB9
            getDB9Rule(idCubee, function (error, db9Response) {
                if (error) {
                    //GET ANY COMMAND FROM MODEL
                    getCubeeModel(idCubee, function (error, cubeeResponse) {
                        if(error){
                            index.winston.error(error);
                            callback(error);
                        }else{
                            getCommandFromModule(cubeeResponse, callback);
                        }
                    });
                }else{
                    callback(null, db9Response);
                }
            });
        } else {
            callback(null, taskResponse);
        }
    });
};

/**
 * This function returs the appCommand and the db9Command of a specific db9CUBEE
 * @param cubeeToGetOutput CUBEE to get the information
 * @param callback         Callback used to send a response
 */
exports.getCommandFromModule = function (cubeeToGetOutput, callback) {
    if(!cubeeToGetOutput.outputHadSetFlag){
        cubeeToGetOutput.outputHadSetFlag = true;
        cubeeToGetOutput.save(function (error, cubee) {
            if(error){
                callback(error);
            }else{
                callback(null,{"appCommand": 6,
                    "db9Command": cubee.currentOutput});
            }
        });
    }else{
        callback(null,{"appCommand": cubeeToGetOutput.appCommand});
    }
};
// exports.getCommandForCubee = function (idCubee, callback) {
//     //TRIES TO GET COMMAND FROM TASK
//     getCommandTask(idCubee, function (error, taskResponse) {
//         if (error) {
//             //TRIES TO GET COMMAND FROM DB9
//             getDB9Rule(idCubee, function (error, db9Response) {
//                 if (error) {
//                     //GET ANY COMMAND FROM MODEL
//                     getCubeeModel(idCubee, function (error, cubeeResponse) {
//                         if(error){
//                             index.winston.error(error);
//                             callback(error);
//                         }else{
//                             getCommandFromModule(cubeeResponse, callback);
//                         }
//                     });
//                 }else{
//                     callback(null, db9Response);
//                 }
//             });
//         } else {
//             callback(null, taskResponse);
//         }
//     });
// };
//
//
// exports.getCommandFromModule = function (cubeeToGetOutput, callback) {
//     if(!cubeeToGetOutput.outputHadSetFlag){
//         cubeeToGetOutput.outputHadSetFlag = true;
//         cubeeToGetOutput.save(function (error, cubee) {
//             if(error){
//                 callback(error);
//             }else{
//                 callback(null,{"appCommand": 6,
//                     "db9Command": cubee.currentOutput});
//             }
//         });
//     }else{
//         callback(null,{"appCommand": cubeeToGetOutput.appCommand});
//     }
// };

/**
 * This function returns a CUBEE model, by his id
 * @param idCubee   Id of the CUBEE to get the model
 * @param callback  Callback used to send a response
 */
var getCubeeModel = function (idCubee, callback) {
    cubeeController.getCubeeById(idCubee, function (error, cubee) {
        if(error){
            callback(error);
        }else if(cubee){
            callback(null, cubee);
        }
        else{
            var cubeeNotFindedError = new Error("Cant Find Cubee");
            cubeeNotFindedError.status = 400;
            callback(cubeeNotFindedError);
        }
    });
};

/**
 * This function execute the update of the CubeeSignal
 *
 * @param idCubee  Mandatory  Id of the CUBEE to set the signal
 * @param signal   Mandatory  Signal to set
 * @param callback Mandatory  callback used to send a response
 */
var setCubeeSignal = function (idCubee, signal, callback) {
    cubeeController.setCubeeSignal(idCubee, signal, function (error, cubee) {
        if (error) {
            callback(error);
        } else {
            notificationController.sendNotification(cubee.idOwner, cubee._id, SIGNAL_ALERT_TYPE, "Sinal recebido de CUBEE", "O " + cubee.name + " esta chamando.", "LOW");
            callback(null, cubee);

        }
    });
};

/**
 * This function delegates to eventController to get the command task of a specific CUBEE
 * @param idCubee  if of the CUBEE to get the information
 * @param callback Callback used to send a response
 */
exports.getCommandTask = function (idCubee, callback) {
    eventController.getCommandTask(idCubee, function (error, task) {
        if(error){
            callback(error);
        }
        else if (task) {
            callback(null, {"appCommand": task.appCommand});
        } else {
            var unknowError = Error("Unknown error");
            unknowError.status = 400;
            callback(unknowError);
        }
    });
};
/**
 * This function execute the update of the CubeeState
 *
 * @param idCubee  Mandatory  Id of the CUBEE to set the state
 * @param state    Mandatory  State to set
 * @param callback Mandatory  callback used to send a response
 */
var setCubeeState = function (idCubee, state, callback) {
    cubeeController.getCubeeById(idCubee, function (error, cubeeFinded) {
        if(error){
            callback(error);
        }else if(cubeeFinded){
            if(cubeeFinded.cubeeState !== state.toString()){
                var typeAlert = "";
                var stateDescription = "";

                if(state === 0){
                    stateDescription = "Desativado";
                    typeAlert = DEACTIVATION_ALERT_TYPE;
                }else{
                    stateDescription = "Ativado";
                    typeAlert = ACTIVATION_ALERT_TYPE;
                }
                notificationController.sendNotification(cubeeFinded.idOwner, cubeeFinded._id, typeAlert, "Cubee "+ stateDescription, "O " + cubeeFinded.name +" foi " +
                    stateDescription + ".", "LOW");
            }
            cubeeController.setCubeeState(cubeeFinded, state, function (error) {
                if (error) {
                    callback(error);
                } else {
                    callback(null, "Sucess");
                    // cubeeController.setCubeeAppCommand(idCubee, 0, function (error, response) {
                    //     if (error) {
                    //         callback(error);
                    //     } else {
                    //         callback(null, response);
                    //     }
                    // });
                }
            });
        } else{
            var cubeeNotFindedError = new Error("Cant Find Cubee");
            cubeeNotFindedError.status = 400;
            callback(cubeeNotFindedError);
        }
    });

};

/**
 * This function verifies the authenticity if the user and delegates to cubeeController to set the upper and lower thresholds of this CUBEE.
 * @param idToken          Token of the owner(user) to verify this authenticity.
 * @param idCubee          Id of the CUBEE to get the information
 * @param upperThreshold   Upper threshold to be setted in the CUBEE
 * @param lowerThreshold   Lower threshold to be setted in the CUBEE
 * @param callback         Callback used to send a response
 */
exports.setThreshold = function (idToken, idCubee, upperThreshold, lowerThreshold, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            cubeeController.setThreshold(uid, idCubee, upperThreshold, lowerThreshold, callback);
        }
    });
};

/*------------------------------------------------
---------------EVENTS RELATED METHODS-------------
------------------------------------------------*/
/**
 * This function verifies the authenticity of the user and delegates to the eventController to register a new event
 * @param idToken   User token to verifies the authenticity
 * @param event     Json representing the event to be registered
 * @param callback  Callback used to send a response
 */
exports.registerEvent = function (idToken, event, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            eventController.registerEvent(uid, event, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to the eventController to set
 * the events to "available", representing with a flag
 *
 * @param idToken  Flag who represents the availability of an Event. True for "available" or False for "unavailable"
 * @param listOfIdEvents   List with Ids of events to be setted
 * @param callback         Callback used to send a response
 */
exports.setEventsAvailable = function (idToken, listOfIdEvents, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else if (uid) {
            eventController.setAvailableEvents(false, listOfIdEvents, callback);

        } else {
            var errorUnknow = new Error("Erro desconhecido");
            errorUnknow.status = 400;
            callback(errorUnknow);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to the eventController to delete a specific event
 * @param idToken          user token to verify the authenticity of the user
 * @param listOfIdEvents   list of id events to be deleted
 * @param callback         callback used to send a response
 */
exports.deleteEvents = function (idToken, listOfIdEvents, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else if (uid) {
            eventController.deleteEvents(listOfIdEvents, callback);
        } else {
            var errorUnknow = new Error("Erro desconhecido");
            errorUnknow.status = 400;
            callback(errorUnknow);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to eventController to get events by current time of a specific CUBEE
 *
 * @param idToken   User token to verify the authenticity of the user
 * @param idCubee   Id of the CUBEE to get the events
 * @param callback  Callback used to send a response
 */
exports.getEventsByCubeeAndCurrentTime = function (idToken, idCubee, callback) {
    var currentDateAndTime = new Date();
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else{
            eventController.getEventsByCubee(uid, idCubee, function (error, events) {
                if (error) {
                    callback(error);
                } else {
                    eventController.getTasksByManyEvents(events, function (error2, listEventWithTasks) {
                        if (error2) {
                            callback(error2);
                        } else {
                            var booleanInit = false;
                            var booleanFinish = false;
                            var response = [];
                            var add = 0;
                            for (var eachEvent in listEventWithTasks) {
                                booleanInit = false;
                                booleanFinish = false;
                                for (var eachTask in listEventWithTasks[eachEvent].tasks) {
                                    if (listEventWithTasks[eachEvent].tasks[eachTask].dateTask >= currentDateAndTime) {
                                        booleanInit = true;
                                    }
                                    else if (listEventWithTasks[eachEvent].tasks[eachTask].dateTask <= currentDateAndTime) {
                                        booleanFinish = true;
                                    }
                                }
                                if (booleanInit && booleanFinish) {
                                    response[add++] = listEventWithTasks[eachEvent]._id;
                                }
                            }
                            callback(null, {"listOfIdEvents": response});
                        }
                    });
                }
            });
        }
    });
};

/**
 * This function  verifies the authenticity of the user and delegates to eventController to get a event by the user id
 * @param idToken   User token to verify the authenticity of the user
 * @param callback  Calback used to send a response.
 */
exports.getEventsByUser = function (idToken, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            eventController.getEventsByUser(uid, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to eventController to return tasks by
 * the id of the Event
 *
 * @param idToken  User token to verify the authenticity of the user
 * @param idEvent  Id of the event
 * @param callback Callback used to send a response
 */
exports.getTasksByEvent = function (idToken, idEvent, callback) {
    userController.getUid(idToken, function (error) {
        if (error) {
            callback(error);
        } else {
            eventController.getTasksByEvent(idEvent, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to eventController to deletes
 * an event and so his tasks
 *
 * @param idToken   User token used to verify the authenticity of the user
 * @param idEvent   Id of the event to be deleted
 * @param callback  Callback used to send a response
 */
exports.deleteEvent = function (idToken, idEvent, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            eventController.deleteEvent(uid, idEvent, function (error) {
                if (error) {
                    callback(error);
                } else {
                    eventController.deleteTasksByEvent(idEvent, function (error) {
                        if (error) {
                            callback(error);
                        } else {
                            callback(null, {'status': "status_ok"});
                        }
                    });
                }
            });
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to eventController to delete an event by the CUBEE id.
 * It's called when a CUBEE was deleted
 * @param idToken   User token used to verify the authenticity of the user
 * @param idCubee   Id of the CUBEE deleted
 * @param callback  Callback used to send a response
 */
exports.deleteEventByCubee = function (idToken, idCubee, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            eventController.deleteEventByCubee(uid, idCubee, function (error) {
                if (error) {
                    callback(error);
                } else {
                    cubeeController.deleteCubee(idCubee, uid, function (error, successCubee) {
                        if (error) {
                            callback(error);
                        } else {
                            callback(null, successCubee);

                        }
                    });

                }
            });
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to eventController to return all the tasks of
 * a specific CUBEE
 * @param idToken   User token used to verify the authenticity of the user
 * @param idCubee   Id of the CUBEE
 * @param callback  Callback used to send a response
 */
exports.getTasksByCubee = function (idToken, idCubee, callback) {
    userController.getUid(idToken, function (error) {
        if (error) {
            callback(error);
        } else {
            eventController.getTaskByCubee(idCubee, function (error, tasks) {
                if (error) {
                    callback(error);
                } else {
                    callback(null, tasks);
                }
            });
        }
    });
};

/*-----------------------------------------------
---------------RULES RELATED METHODS-------------
-----------------------------------------------*/
/**
 * This function verifies the authenticity of the user and delegates to notificationController to deletes a notification
 * @param idAlarm  Id of the notification to be deleted
 * @param idToken  User token used to verify the authenticity of the user
 * @param callback Callback used to send a response
 */
exports.deleteAlarm = function (idAlarm, idToken, callback) {
    userController.getUid(idToken, function (error) {
        if (error) {
            callback(error);
        } else {
            notificationController.deleteNotification(idAlarm, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user, receives all the alarms form CUBEE(Embedded), organize them and
 * delegates to notificationController to send them to the User.
 *
 * @param idCubee  Id of the CUBEE who's sending the alarms
 * @param alarms   List of alarms to be sended
 * @param callback Callback used to send a response
 */
exports.sendAlarmsFromCubee = function (idCubee, alarms, callback) {
    var alarmsToSend = {};
    cubeeController.getCubeeById(idCubee, function (error, cubee) {
        if (error) {
            callback(error);
        } else {
            alarms.forEach(function (alarm) {
                switch (alarm.code) {
                    case 0:
                        if ("restart" in alarmsToSend) {
                            alarmsToSend.restart.counter += 1;
                        } else {
                            alarmsToSend.restart = {
                                title: "CUBEE reiniciado inesperadamente",
                                body: "O " + cubee.name + " foi reiniciado inesperadamente ",
                                level: "HIGH", counter: 1,
                                typeAlert: RESTART_ALERT_TYPE
                            };
                        }
                        break;
                    case 1:
                        if ("rfid" in alarmsToSend) {
                            alarmsToSend.rfid.counter += 1;
                        } else {
                            alarmsToSend.rfid = {
                                title: "RFID Detectado",
                                body: "O RFID do CUBEE " + cubee.name + " foi detectado",
                                level: "MEDIUM", counter: 1,
                                typeAlert: RFID_ALERT_TYPE
                            };
                        }
                        break;
                    default:
                        var alarmError = {
                            title: "Alarme - erro inesperado",
                            body: "Ocorreu um erro inesperado no " + cubee.name + ".",
                            typeAlert: UNKNOWN_ALERT_TYPE
                        };
                        alarmsToSend.default = alarmError;
                }
            });
            for (var key in alarmsToSend) {
                if (alarmsToSend.hasOwnProperty(key)) {
                    var idCubee = cubee._id;
                    var typeAlert = alarmsToSend[key].typeAlert;
                    var title = alarmsToSend[key].title;
                    var cubeeIdOwner = cubee.idOwner;
                    var body = alarmsToSend[key].body ;
                    var level = alarmsToSend[key].level;
                    if (alarmsToSend[key].counter > 1) {
                        body = body + " " + alarmsToSend[key].counter + " vezes.";
                    } else {
                        body = body + " uma vez.";
                    }
                    notificationController.sendNotification(cubeeIdOwner, idCubee, typeAlert, title, body, level);
                }
            }
            callback(null, "sucess");
        }
    });
};


exports.deleteAllAlarms = function (idToken, callback){
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else if(uid){
            notificationController.deleteAllNotifications(uid, callback);
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to ruleController to register a new rule
 * @param idToken   User token used to verify the authenticity of the user
 * @param jsonRule  Json representing a Rule Model
 * @param callback  Callback used to send a response
 */
exports.registerRule = function (idToken, jsonRule, callback) {
    userController.getUid(idToken, function (error, uid) {
       if(error){
           callback(error);
       }else if(uid){
           ruleController.registerRule(jsonRule, callback);
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to ruleController to delete a rule
 * @param idToken   User token used to verify the authenticity of the user
 * @param idRule    Id of the rule to be deleted
 * @param callback  Callback used to send a response
 */
exports.deleteRule = function (idToken, idRule, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else{
            ruleController.deleteRule(uid, idRule, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to ruleController to return all the rules of
 * a specific user
 *
 * @param idToken   User token used to verify the authenticity of the user and get his id.
 * @param callback  Callback used to send a response
 */
exports.getRulesByUser = function (idToken, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else{
            ruleController.getRulesByUser(uid, callback);
        }
    });
};

/*---------------------------------------------------
---------------DB9 RULES RELATED METHODS-------------
---------------------------------------------------*/
/**
 * This function delegates to the CubeeController to return the DB9Rule of a specific CUBEE
 * @param idCubee  id of the CUBEE in question
 * @param callback Callback used to send a response
 */
var getDB9Rule = function (idCubee, callback) {
    cubeeController.getCubeeById(idCubee, function (error, cubee) {
        if(error){
            callback(error);
        }else if(cubee){
            db9RuleController.checkForDB9Rule(cubee, function (error, db9Rule) {
                if(error){
                    callback(error);
                }else if(db9Rule){
                    var command = {};
                    command.appCommand = 5;
                    command.db9rule = db9Rule;
                    callback(null, command);
                }else{
                    var unknownError = new Error("Unknown Error");
                    unknownError.status = 400;
                    callback(unknownError);
                }
            });
        }else{
            var err = new Error("Unknown error!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This functionverifies the authenticity of the user and delegates to db9RuleController to return all DB9Rules of a
 * specific user.
 * @param idToken   User token used to verify the authenticity of the user and get his id.
 * @param callback  Callback used to send a response
 */
exports.getDB9RulesByUser = function (idToken, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else if(uid){
            db9RuleController.getDB9RulesByUser(uid, callback);
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to db9RuleController to register a new DB9Rule
 * @param idToken      User token used to verify the authenticity of the user
 * @param jsonDB9Rule  Json representing a DB9Rule model
 * @param callback     Callback used to send a response
 */
exports.registerDB9Rule = function (idToken, jsonDB9Rule, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else{
            jsonDB9Rule.idOwner = uid;
            db9RuleController.registerDB9Rule(jsonDB9Rule, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and send to the CUBEE by mqqt a DB9Rule to be executed.
 * @param idToken    User token used to verify the authenticity of the user
 * @param db9RuleId  Id of the DB9Rule to be executed
 * @param cubeeId    Id of the CUBEE who will execute the DB9Rule
 * @param callback   Callback used to send a response
 */
exports.setDB9RuleToCubee = function (idToken, db9RuleId, cubeeId, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        } else if(uid){
            //SEND THIS LOGIC TO CONTROLLER
            db9RuleController.getRuleById(db9RuleId, function (error, rule) {
                if(error){
                    error.status = 400;
                    callback(error);
                }else if(rule){
                    cubeeController.getCubeeById(cubeeId, function (error, cubee) {
                        if (error) {
                            error.status = 400;
                            callback(error);
                        } else if (cubee) {
                            if(cubee.idOwner !== uid){
                                var err = new Error("O cubee não é do usuário informado");
                                err.status = 400;
                                callback(err);
                            }else{
                                var commandJson = {};
                                commandJson.appCommand = 5;
                                commandJson.db9rule = rule;
                                mqttClientController.sendCommandJson(cubeeId, commandJson);
                                cubee.db9RuleId = db9RuleId;
                                cubee.db9RuleHadSetFlag = false;
                                cubee.save(function (error, cubee) {
                                    if (error) {
                                        error.status = 400;
                                        callback(error);
                                    } else {
                                        callback(null, cubee);
                                    }
                                });
                            }
                        } else {
                            var CubeeNotFoundedError = new Error("Cant find this cubee");
                            CubeeNotFoundedError.status = 400;
                            callback(CubeeNotFoundedError);
                        }
                    });
                }else{
                    var err = new Error("Cant find this Rule");
                    err.status = 400;
                    callback(err);
                }

            });
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies the authenticity of the user, set a output in the CUBEE and delegates to mqttClientController
 * to send the AppComand
 * @param idToken  User token used to verify the authenticity of the user
 * @param output   Output to be setted in the CUBEE
 * @param cubeeId  Id of the CUBEE
 * @param callback Callback used to send a response
 */
exports.setOutputToCubee = function (idToken, output, cubeeId, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        } else if(uid){
            cubeeController.getCubeeById(cubeeId, function (error, cubee) {
                if (error) {
                    error.status = 400;
                    callback(error);
                } else if (cubee) {
                    var commandJson = {"appCommand": 6,
                        "db9Command": parseInt(output)};
                    mqttClientController.sendCommandJson(cubeeId, commandJson);
                    callback(null, cubee);
                } else {
                    var err = new Error("Cant find this cubee");
                    err.status = 400;
                    callback(err);
                }
            });
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to DB9RuleController to delete a DB9Rule.
 * @param idToken    User token used to verify the authenticity of the user
 * @param db9RuleId  Id of the DB9Rule to be deleted
 * @param callback   Callback used to send a response
 */
exports.deleteDB9Rule = function (idToken, db9RuleId, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else if(uid){
            db9RuleController.deleteDB9Rule(db9RuleId, callback);
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }

    });
};

/**
 * This function verifies the authenticity of the user and delegates to cubeeController to returns a CUBEE by the DB9Rule id.
 * @param idToken   User token used to verify the authenticity of the user
 * @param ruleId    Id of the DB9Rule
 * @param callback  Callback used to send a response
 */
exports.getCubeeByDb9 = function (idToken, ruleId, callback) {
    userController.getUid(idToken, function (error, uid) {
        if(error){
            callback(error);
        }else if(uid){
            cubeeController.getCubeeByDb9(ruleId, callback);
        }else{
            var err = new Error("Validation failed!");
            err.status = 400;
            callback(err);
        }

    });

};

/*------------------------------------------------------
---------------NOTIFICATION RELATED METHODS-------------
------------------------------------------------------*/
/**
 * This function verifies the authenticity of the user and delegates to notificationController to get all notifications
 * of a specific user
 * @param idToken   User token used to verify the authenticity of the user and get his id.
 * @param callback  Callback used to send a response
 */
exports.getNotificationByUser = function (idToken, callback) {
    userController.getUid(idToken, function (error, id) {
        if (error) {
            callback(error);
        } else {
            notificationController.getNotificationsByUser(id, function (error, notifications) {
                if (error) {
                    callback(error);
                } else if (notifications) {
                    callback(null, notifications);
                } else {
                    var unknownError = new Error("Unknown ERROR");
                    unknownError.status = 400;
                    callback(unknownError);
                }
            });
        }
    });
};

exports.getNotificationByUserWithFilter = function (idToken, filter, callback) {
    userController.getUid(idToken, function (error, id) {
        if (error) {
            callback(error);
        } else {
            notificationController.getNotificationsByUserWithFilter(id, filter,  function (error, notifications) {
                if (error) {
                    callback(error);
                } else if (notifications) {
                    callback(null, notifications);
                } else {
                    var unknownError = new Error("Unknown ERROR");
                    unknownError.status = 400;
                    callback(unknownError);
                }
            });
        };
    });
};

/**
 *  This function saves the registrationToken in the user model
 * @param registrationToken  Registration token of a user
 * @param idToken            User token used to verify the authenticity of the user and get his id.
 * @param callback           Callback used to send a response
 */
exports.saveRegistrarionToken = function (registrationToken, idToken, callback) {
    userController.getUid(idToken, function (error, id) {
        if (error) {
            callback(error);
        } else {
            userController.saveRegistrationToken(id, registrationToken, callback);
        }
    });
};

/*------------------------------------------------
---------------SECTOR RELATED METHODS-------------
------------------------------------------------*/
/**
 * This function verifies the authenticity of the user and delegates to sectorController to register a new sector
 * @param userToken      User token used to verify the authenticity of the user
 * @param sectorName     Name of the sector to be registered
 * @param listIdCubees   List od CUBEEs to be added in the new sector
 * @param callback       Callback used to send a response
 */
exports.registerSector = function (userToken, sectorName, listIdCubees, callback) {
    userController.getUid(userToken, function (error, uid) {
        if (uid) {
            sectorController.registerSector(sectorName, uid, function (error, sector) {
                if (error) {
                    callback(error);
                } else {
                    if (listIdCubees) {
                        cubeeController.setListCubeeSector(listIdCubees, sector._id, function (error) {
                            if (error) {
                                return callback(error);
                            } else {
                                callback(null, sector);
                            }
                        });
                    } else {
                        callback(null, sector);
                    }
                }
            });
        } else if (error) {
            callback(error);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to cubeeController to put a Cubee (or many CUBEEs)
 * in a specific Sector
 * @param idSector     Id of the sector who will recieve the CUBEEs
 * @param idToken      User token used to verify the authenticity of the user
 * @param listIdCubee  List of id of CUBEEs who will be added in the sector in question
 * @param callback     Callback used to send a response
 */
exports.registerCubeeInSector = function (idSector, idToken, listIdCubee, callback) {
    userController.getUid(idToken, function (error, uid) {
        if (error) {
            callback(error);
        } else if (uid) {
            cubeeController.setListCubeeSector(listIdCubee, idSector, callback);
        } else {
            var errorUnknow = new Error("Erro desconhecido");
            errorUnknow.status = 400;
            callback(errorUnknow);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to sectorController to return all sectors of a
 * specific User
 *
 * @param userToken  User token used to verify the authenticity of the user an get his id
 * @param callback   Callback used to send a response
 */
exports.getSectorsByUser = function (userToken, callback) {
    userController.getUid(userToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            sectorController.getSectorsByUser(uid, callback);
        }
    });
};

/**
 * This function verifies the authenticity of the user and delegates to sectorController to delete a specifc sector.
 *
 * @param userToken User token used to verify the authenticity of the user
 * @param idSector  Id of the sector to be deleted
 * @param callback  Callback used to send a response
 */
exports.deleteSector = function (userToken, idSector, callback) {
    userController.getUid(userToken, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            sectorController.deleteSector(uid, idSector, function (error, sector) {
                if (error) {
                    callback(error);
                } else {
                    cubeeController.resetCubeesSector(idSector, function (error) {
                        if (error) {
                            callback(error);
                        } else {
                            callback(null, sector);
                        }
                    });
                }
            });
        }
    });
};

/*--------------------------------------
---------------OTHERS/TESTS-------------
--------------------------------------*/
exports.getUsetTest = function (callback) {
    userController.getUserTest(callback);
};

exports.getCubeeTest = function (callback) {
    cubeeController.getCubeeTest(callback);
};

exports.getNewId = function (callback) {
    cubeeController.getNewId(callback);
};
