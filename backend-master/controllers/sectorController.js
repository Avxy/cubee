/**
 * Created by adson on 03/07/17.
 */
var Sector = require('../models/Sector').Sector;

/**
 * This function registers a new sector on the database
 * @param sectorName  Name of the sector
 * @param idOwner     Id of the owner (user) of the sector
 * @param callback    Callback used to send a response
 */
exports.registerSector = function (sectorName, idOwner, callback) {
    new Sector({
        name: sectorName,
        idOwner: idOwner
    }).save(function (error, sector){
        if(sector){
            callback(null, sector);
        }else if(error){
            error.status = 400;
            callback(error);
        }else{
            var err = new Error("Can't register this sector");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function returns all sectors of a specific user
 * @param idOwner  Id of the user in question
 * @param callbak  Callback used to send a response
 */
exports.getSectorsByUser = function (idOwner, callbak) {
    Sector.find({idOwner: idOwner}, function (error, sectors) {
       if(sectors) {
           callbak(null, sectors);
       }else if(error){
           error.status = 400;
           callbak(error);
       }else{
           var err = new Error("There's no sector registred by this user");
           err.status = 400;
           callbak(err);
       }
    });
};

/**
 * This function deletes a specific sector
 * @param idOwner   Id of the owner of the sector to be deleted
 * @param idSector  Id of the sector to be deleted
 * @param callback  Callback used to send a response
 */
exports.deleteSector = function (idOwner, idSector, callback){
    Sector.findById(idSector, function (error, sector) {
        if(error){
            error.status = 400;
            callback(error);
        }else if(sector && sector.idOwner === idOwner){
            Sector.remove({_id: idSector}, function (error) {
                if(error){
                    error.status = 400;
                    callback(error);
                }else{
                    callback(null, sector);
                }
            });
        }else{
            var err = new Error("This sector can't be deleted");
            err.status = 400;
            callback(err);
        }
    });
};