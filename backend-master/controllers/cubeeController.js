/**
 * Created by adson on 26/04/17.
 */
var Cubee = require('../models/Cubee');
var Sector = require('../models/Sector').Sector;
var Measurement = require('../models/Measurement').Measurement;
var mongoDb = require('./dbController');
var mqttClientController = require('../controllers/mqttClientController');

/**
 * This function fowards to the CUBEE registration and do the error handling
 * provenient by the database
 * @param name       CUBEE name
 * @param idCubee    CUBEE id
 * @param oldIdCubee Old CUBEE id
 * @param idOwner    CUBEE owner id
 * @param callback   Callback used to send a response
 */
exports.registerCubee = function (name, idCubee, oldIdCubee, idOwner, callback) {
    if (oldIdCubee) {
        Cubee.findById(oldIdCubee, function (error, cubee) {
            if (!cubee) {
                DBRegisterCubee(name, idOwner, idCubee, callback);
            } else if (error) {
                error.status = 400;
                error.message = "erro400";
                callback(error);
            } else {
                var erroCubeeExist = new Error("CUBEE já existente");
                erroCubeeExist.message = "CUBEE já existente";
                erroCubeeExist.status = 420;
                callback(erroCubeeExist);
            }
        });
    } else {
        DBRegisterCubee(name, idOwner, idCubee, callback);
    }
};

/**
 * This function do the registration of the CUBEE on the database
 * @param name
 * @param idOwner
 * @param idCubee
 * @param callback
 * @constructor
 */
var DBRegisterCubee = function (name, idOwner, idCubee, callback) {
    new Cubee({
        '_id': idCubee,
        'name': name,
        'idOwner': idOwner
    }).save(function (error, cubee) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, cubee);
        }
    });
};

/**
 * This function returns the list of CUBEES by the id of the owner
 *
 * @param OwnerId   CUBEE owner id
 * @param callback  Callback used to send a response
 */
exports.getCubeesByOwner = function (OwnerId, callback) {
    Cubee.find({idOwner: OwnerId}, function (error, cubees) {

        if (error) {
            error.status = 400;
            callback(error);
        } else if (cubees) {
            callback(null, cubees);
        } else {
            var err = new Error("There is no CUBEE in our database");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function returns the list of CUBEES by the id of the sector
 *
 * @param idOwner   CUBEE owner id
 * @param idSector  CUBEE sector id
 * @param callback  Callback used to send a response
 */
exports.getCubeesBySector = function (idOwner, idSector, callback) {
    Sector.findById(idSector, function (error, sector) {
        if (error) {
            error.status = 400;
            callback(error);
        } else if (sector) {
            if (sector.idOwner === idOwner) {
                Cubee.find({idSector: idSector}, function (error, cubees) {
                    if (error) {
                        error.status = 400;
                        callback(error);
                    } else {
                        callback(null, cubees);
                    }
                });
            } else {
                var err = new Error("This sector don't belong to this user");
                err.status = 400;
                callback(error);
            }
        } else {
            var cantFindSectorerror = new Error("Can't find the sector");
            cantFindSectorerror.status = 400;
            callback(cantFindSectorerror);
        }
    });
};

/**
 * This function returns the list of CUBEES by the id of the sector
 * @param idOwner   CUBEE owner id
 * @param idSector  CUBEE sector id
 * @param callback  Callback used to send a response
 */
exports.getCubeesWithNoSector = function (idOwner, callback) {
    Cubee.find({idOwner: idOwner, idSector: null}, function (error, cubees) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, cubees);
        }
    });
};

/**
 * This function sets the sector of a cubee for null, turning it into a cubee with no sector.
 * @param idSector  Sector id
 * @param callback  Callback used to send a response
 */
exports.resetCubeesSector = function (idSector, callback) {
    Cubee.find({idSector: idSector}, function (error, cubees) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            cubees.forEach(function (cubee) {
                cubee.idSector = null;
                cubee.save();
            });
            callback(null, "ok");
        }
    });
};

/**
 * This is a local function to set the idSector of a specific CUBEE.
 *
 * @param idCubee   CUBEE id to be setted
 * @param idSector  Sector id to be setted in the CUBEE
 * @param callback  Callback used to send a response
 */
var setCubeeSectorLocal = function (idCubee, idSector, callback) {
    Cubee.findById(idCubee, function (error, cubee) {
        if(error){
            callback(error);
        }else if(cubee){
            if(idSector === "no_sector"){
                cubee.idSector = null;
            }else{
                cubee.idSector = idSector;
            }
            cubee.save();
            callback(null, "sucess");
        }else{
            var errorUnknow = new Error("Error Unknow");
            errorUnknow.status = 400;
            callback(errorUnknow);
        }
    });
};

/**
 * the export of the function to set the idSector of a specific CUBEE
 * @type {setCubeeSectorLocal}
 */
exports.setCubeeSector = setCubeeSectorLocal;

/**
 * This function set the idSector of a list of CUBEEs. All CUBEEs will have the same idSector
 *
 * @param listIdCubee  Array of CUBEEs
 * @param idSector     idSector to be setted in the list of CUBEEs
 * @param callback     Callback used to send a response
 */
exports.setListCubeeSector = function(listIdCubee, idSector, callback){
    this.getCubeeById(listIdCubee[0], function (error, cubee){
        if(error){
            return callback(error);
        }else if(cubee){
            for(var i=0; i < listIdCubee.length; i++){
                setCubeeSectorLocal(listIdCubee[i], idSector, function (error) {
                    if(error){
                        for (var j=0; j <= i; j++){
                            this.setCubeeSector(listIdCubee[j], cubee.idSector, function (error){
                                if(error){
                                    console.log(error.message);
                                }
                            });
                        }
                        return callback(error);
                    }
                });
            }
            callback(null, listIdCubee);
        }
    });
};


/**
 * This function returns a Cubee by id
 *
 * @param idCubee   CUBEE id
 * @param callback  Callback used to send a response
 */
exports.getCubeeById = function (idCubee, callback) {
    Cubee.findById(idCubee, function (error, cubee) {
        if (error) {
            error.status = 400;
            callback(error);
        } else if (cubee) {
            callback(null, cubee);
        } else {
            var errNull = new Error("Can't Find Cubee");
            errNull.status = 400;
            callback(errNull);
        }
    });
};

/**
 * This function returns all cubees with a specific db9 rule
 *
 * @param ruleId    Id of the db9Rule
 * @param callback  Callback used to send a response
 */
exports.getCubeeByDb9 = function (ruleId, callback) {
    if(ruleId){
        Cubee.find({db9RuleId: ruleId}, function (error, cubees) {
            if (error) {
                error.status = 400;
                callback(error);
            } else {
                callback(null, cubees);
            }
        });
    }else{
        var err = new Error("No rule ID! Bad request!");
        err.status = 400;
        callback(err);
    }
};



/**
 * This function set the flag cubeeState of a specific CUBEE
 *
 * @param  cubeeFinded  CUBEE to set the state
 * @param  state        state to set in the CUBEE
 * @param  callback     Callback used to send a response
 */
exports.setCubeeState = function (cubeeFinded, state, callback) {
        if (cubeeFinded) {
            if(cubeeFinded.cubeeState != state){
                cubeeFinded.cubeeState = state;
                cubeeFinded.save(function (error, cubee) {
                    if (error) {
                        console.log('Error saving CUBEE with updated CUBEE state ' + state);
                        error.status = 400;
                        callback(error);
                    } else {
                        console.log('CUBEE successfully saved with state ' + state);
                        console.log(cubee);
                        callback(null, cubee);
                    }
                });
            }
        }
};


/**
 * This function set the flag cubeeSignal of a specific CUBEE
 *
 * @param idCubee  CUBEE id
 * @param signal   Signal to set in the CUBEE
 * @param callback Callback used to send a response
 */
exports.setCubeeSignal = function (idCubee, signal, callback) {
    Cubee.findById(idCubee, function (error, cubeeFinded) {
        if (error) {
            error.status = 400;
            callback(error);
        } else if (cubeeFinded) {
            cubeeFinded.cubeeBtn = !cubeeFinded.cubeeBtn;
            cubeeFinded.save(function (error, cubee) {
                if (error) {
                    error.status = 400;
                    callback(error);
                } else {
                    callback(null, cubee);
                }
            });
        } else {
            var errorCubeeNotFinded = new Error("CUBEE Not found");
            errorCubeeNotFinded.status = 400;
            callback(errorCubeeNotFinded);
        }
    });
};

/**
 * This function returns the owner id of a specific CUBEE
 *
 * @param idCubee CUBEE id
 * @param callback Callback used to send a response
 */
exports.getOwnerId = function (idCubee, callback) {
    Cubee.findById(idCubee, function (error, cubeeFinded) {
            if (error) {
                error.status = 400;
                callback(error);
            } else if (cubeeFinded) {
                callback(null, cubeeFinded);
            } else {
                var errorCubeeNotFinded = new Error("CUBEE Not found");
                errorCubeeNotFinded.status = 400;
                callback(errorCubeeNotFinded);
            }
        }
    )
    ;
}
;

/**
 * This function set the flag appCommand of a specific CUBEE
 *
 * @param idCubee   CUBEE id
 * @param command   command to set in the CUBEE
 * @param callback  Callback used to send a response
 */
exports.setCubeeAppCommand = function (idCubee, command, callback) {
    Cubee.findById(idCubee, function (error, cubeeFinded) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            if (cubeeFinded) {
                var commandJson = {};
                commandJson.appCommand = parseInt(command);
                callback(null, cubeeFinded);
                mqttClientController.sendCommandJson(idCubee, commandJson);
            } else {
                var errNull = new Error("Cubee not fouded");
                errNull.status = 400;
                callback(errNull);
            }
        }
    });
};

/**
 * This function deletes a CUBEE by his id.
 * @param idCubee  CUBEE id
 * @param idOwner  CUBEE owner id
 * @param callback Callback used to send a response
 */
exports.deleteCubee = function (idCubee, idOwner, callback) {
    Cubee.findById(idCubee, function (error, cubee) {
        if (error) {
            callback(error);
        } else {
            if (cubee && cubee.idOwner === idOwner) {
                Cubee.remove({_id: idCubee}, function (error2, success) {
                    if (error2) {
                        callback(error2);
                    } else {
                        removeMeasurementByCubee(idCubee, function (error) {
                            if(error){
                                callback(error);
                            }
                            else {
                                callback(null, success);
                            }
                        });
                    }
                });
            } else {
                var err = new Error("Delete Error. This cubee don't beloongs to this user.");
                err.status = 400;
                callback(error);
            }
        }
    });
};

/**
 * This function revoces the measurements of a specific CUBEE
 *
 * @param idCubee   id CUBEE
 * @param callback  Callback used to send a response
 */
var removeMeasurementByCubee = function (idCubee, callback) {
    Measurement.remove({idCubee: idCubee}, function (error) {
        if(error){
            error.status = 400;
            callback(error);
        }else{
            callback(null);
        }
    });
};

/**
 * This function sets the upper and lower threshold of a CUBEE
 *
 * @param uid              Id of the owner (User)
 * @param idCubee          Id of the CUBEE
 * @param upperThreshold   number representing the upper threshold
 * @param lowerThreshold   number representing the lower threshold
 * @param callback         Callback used to send a reponse
 */
exports.setThreshold = function (uid, idCubee, upperThreshold, lowerThreshold, callback) {
    Cubee.findById(idCubee, function (error, cubee){
        if(error){
            error.status = 400;
            callback(error);
        }else if(cubee){
            if(cubee.idOwner === uid) {
                if (upperThreshold) {
                    cubee.upperThreshold = upperThreshold;
                }
                if (lowerThreshold) {
                    cubee.lowerThreshold = lowerThreshold;
                }
                cubee.save(function (error, cubee) {
                    if (error) {
                        error.status = 400;
                        callback(error);
                    } else {
                        callback(null, cubee);
                    }
                });
            }else{
                var err = new Error("This cubee don't belongs to this user");
                err.status = 400;
                callback(err);
            }
        }else{
            var cubeeNotFoundedError = new Error("Cubee not founded");
            cubeeNotFoundedError.status = 400;
            callback(cubeeNotFoundedError);
        }
    });
};

/**
 * This function returns an Id, who be used to create a new CUBEE
 *
 * @param callback Callback used to send a response
 */
exports.getNewId = function (callback) {
   callback(mongoDb.getNewId());
};