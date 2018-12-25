/**
 * Created by adson on 28/04/17.
 */
var User = require('../models/User').User;

var firebaseController = require('./firebaseController');

/**
 * This function registers a user by the Facebook method
 *
 * @param idToken  FirebaseToken of the user
 * @param callback Callback used to send a response
 */
exports.registerFbUser = function (idToken, callback){
    if(idToken){
        firebaseController.getUid(idToken, function (error, uid) {
            if(error){
                callback(error);
            }else{
                firebaseController.getUserRecordById(uid, function (error, userRecord) {
                    if(error){
                        callback(error);
                    }else{
                        createOrUpdateUser(userRecord, callback);
                    }
                });
            }
        });

    }else{
        var error = new Error("Authentication Error: Empty token");
        error.status = 400;
        callback(error);
    }


};

/**
 * This function registers a user by the email and password method
 *
 * @param name      User name
 * @param email     User email
 * @param telephone User telephone
 * @param password  User password
 * @param callback  Callback used to send a response
 */
exports.registerEPUser = function (name, email, telephone, password, callback) {
    firebaseController.createFirebaseUser(name, email, password, function (error, uid) {
        if (error) {
            callback(error);
        } else {
            new User({
                _id: uid,
                name: name,
                email: email,
                telephone: telephone
            }).save(function (error, user) {
                if (error) {
                    error.status = 400;
                    callback(error);
                } else {
                    callback(null, user);
                }
            });
        }
    });
};

/**
 * This function edits a user by his id;
 * @param uid        User id
 * @param name       User name
 * @param email      User name
 * @param telephone  User email
 * @param password   User password
 * @param callback   Callback used to send a response
 */
exports.editUser = function (uid, name, email, telephone, password, callback) {
    var toUpdateFirebase = {uid: uid};
    var toUpdateDb = {uid: uid};

    if (name !== "") {
        toUpdateFirebase.displayName = name;
        toUpdateDb.name = name;
    }

    if (email !== "") {
        toUpdateFirebase.email = email;
        toUpdateDb.email = email;
    }

    if (password !== "") {
        toUpdateFirebase.password = password;
    }

    if (telephone !== "") {
        toUpdateDb.telephone = telephone;
    }

    firebaseController.updateFirebaseUser(uid, toUpdateFirebase, function (error) {
        if(error){
            callback(error);
        }else{
            createOrUpdateUser(toUpdateDb, false, callback);
        }
    });
};

/**
 * This function returns a user by his id
 *
 * @param uid      User id
 * @param callback Callback used to send a response
 */
exports.getUser = function (uid, callback) {
   User.findById(uid, function (error, user){
      if(error){
          error.status = 400;
          callback(error);
      }else if(user){
          callback(null, user);
      }else{
          var err = new Error("User not founded");
          err.status = 400;
          callback(err);
      }
   });
};

/**
 * This function returns a user id by his FirebaseToken
 *
 * @param token    User FirebaseToken
 * @param callback Callback used to send a response
 */
exports.getUid = function (token, callback) {
    firebaseController.getUid(token, function (error, uid) {
        if(error){
            callback(error);
        }else if(uid){
            User.findById(uid, function (error, user) {
                if(error){
                    var err = new Error("Authentication Error. User not found in our database.");
                    err.status = 400;
                    callback(err);
                }else if(user){
                    callback(null, uid);
                }else{
                    var UserNotFoundedError = new Error("User not founded");
                    UserNotFoundedError.status = 400;
                    callback(UserNotFoundedError);
                }
            });
        }else{
            var errorUnknow = new Error("Erro desconhecido");
            errorUnknow.status = 400;
            callback(errorUnknow);
        }
    });
};

exports.setupDB = function (callback) {
    User.remove({});

    User.dropDatabase(function (error) {
        if(error){
            callback(error);
        }
    });

    callback(null, "Success");
};


/**
 * This function create or update a user, if exists.
 *
 * @param toUpdate           Params to update in the user
 * @param createIfNotExists  Indicative flag
 * @param callback           Callback used to send a response
 */
function createOrUpdateUser(toUpdate, createIfNotExists, callback) {

    User.update({_id: toUpdate.uid}, toUpdate, {upsert: createIfNotExists}, function (error) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            User.findById(toUpdate.uid, function (error, user) {
                if (error) {
                    callback(error);
                }else if(user){
                    callback(null, user);
                }else{
                    var err = new Error("User not found");
                    err.status = 400;
                    callback(err);
                }
            });
        }
    });
}

/**
 * This function save the FirebaseRegistrationToken in the user Model
 * @param id                Id of the user
 * @param registrationToken FirebaseToken
 * @param callback          Callback used to send a response
 */
exports.saveRegistrationToken = function (id, registrationToken, callback) {
    User.findById(id, function (err, user) {
        if (err) {
            callback(err);
        } else if (user) {
            user.registrationToken = registrationToken;
            user.save(function (error) {
               if(error){
                   callback(error);
               }else{
                   callback(null, user.name + "updated with registrationToken");
               }
            });
        } else {
            var userNotFoundedError = new Error("User not founded");
            userNotFoundedError.status = 400;
            callback(userNotFoundedError);
        }
    });
};