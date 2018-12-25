var User = require('../models/User').User;
var mongoDbAlert = require('../models/Alert');
var admin = require('firebase-admin');
var ruleController = require('./ruleController');
var index = require('../index');

// Set the message as high priority and have it expire after 24 hours.
var options = {
    priority: "high",
    timeToLive: 60 * 60 * 24
};

/**
 * This function sends a notification to a specific user
 * @param idOwner    Id of the user who will receive the notification
 * @param idCubee    Id of the CUBEE who was related the notification
 * @param typeAlert  The type of the notification
 * @param title      The title of the notification
 * @param body       The text who will appears on the notification
 * @param level      Level of criticality of the notification
 */
exports.sendNotification = function (idOwner, idCubee, typeAlert, title, body, level) {
    User.findById(idOwner, function (error, user) {
            if (error) {
                index.winston.error("Error to retrive user registrationToken when sending message: " + title +" To:"+ idOwner);
            } else if(user) {
                saveNotification(idOwner, idCubee,typeAlert, title, body, level);
                var payload = {
                    notification: {
                        title: title + " - " + typeAlert,
                        body: body,
                        tag: idCubee
                    }
                };
                var registrationToken = user.registrationToken;


                //send notification
                if (registrationToken) {
                    admin.messaging().sendToDevice(registrationToken, payload, options)
                        .then(function (response) {
                            index.winston.info("Successfully sent message:", response);
                            //idOwner, previousIdCubee, typeAlert
                            ruleController.searchRulesForCubee(idOwner, idCubee, typeAlert);
                        })
                        .catch(function (error2) {
                            index.winston.error("Error sending message:", error2);
                        });
                }else{
                    var errorRegistration = new Error("User don't have Registration Token");
                    errorRegistration.status = 400;
                    index.winston.error(errorRegistration.message);
                }

            }else{
                var unknownError = new Error("Unknown Error");
                unknownError.status = 400;
                index.winston.error("Error in sending notification: " + unknownError.message);
            }
        }
    );
};

/**
 * This function returns all the notifications of a specific user
 * @param id        Id of the user
 * @param callback  Callback used to send a response
 */
exports.getNotificationsByUser = function (id, callback) {
    mongoDbAlert.Alert.find({idOwner: id}, function (error, notifications) {
        if (error) {
            callback(error);
        } else {
            callback(null, notifications);
        }
    }).sort({date: -1});
};

/**
 * This function deletes a specific notification
 * @param idAlarm   Id of the notification
 * @param callback  Callback used to send a response
 */
exports.deleteNotification = function (idAlarm, callback) {
    mongoDbAlert.Alert.remove({_id: idAlarm}, function (error, success) {
        if (error) {
            callback(error);
        } else {
            callback(null, success);
        }
    });
};

exports.getNotificationsByUserWithFilter = function (id, filter, callback) {
    if (filter && filter.cubeeIds.length > 0 && filter.level.length > 0 && "level" in filter && "cubeeIds" in filter) {
        mongoDbAlert.Alert.find({
            idOwner: id,
            level: {$in: filter.level},
            idCubee: {$in: filter.cubeeIds}
        }).sort({date: -1}).exec(function (error, notifications) {
            if (error) {
                callback(error);
            } else {
                callback(null, notifications);
            }
        });
    } else if(filter && filter.level.length > 0 && "level" in filter && "cubeeIds" in filter){
        mongoDbAlert.Alert.find({
            idOwner: id,
            level: {$in: filter.level}
        }).sort({date: -1}).exec(function (error, notifications) {
            if (error) {
                callback(error);
            } else {
                callback(null, notifications);
            }
        })
    }else if(filter && filter.cubeeIds.length > 0 && "level" in filter && "cubeeIds" in filter){
        mongoDbAlert.Alert.find({
            idOwner: id,
            idCubee: {$in: filter.cubeeIds}
        }).sort({date: -1}).exec(function (error, notifications) {
            if (error) {
                callback(error);
            } else {
                callback(null, notifications);
            }
        })
    }else{
        mongoDbAlert.Alert.find({
            idOwner: id
        }).sort({date: -1}).exec(function (error, notifications) {
            if (error) {
                callback(error);
            } else {
                callback(null, notifications);
            }
        });
    }
};


/**
 * This function deletes all notification from the database
 * @param uid
 * @param callback  Callback used to send a response
 */
//Todo: check com xqn
exports.deleteAllNotifications = function (uid, callback) {
    mongoDbAlert.Alert.remove({idOwner : uid }, function (error, success) {
        if (error) {
            callback(error);
        } else {
            callback(null, success);
        }
    });
};

/**
 * This function saves a notification on the database
 * @param idOwner     Id of the owner of the notification
 * @param idCubee     Id of the CUBEE related to the notification
 * @param typeAlert   Type of the notification
 * @param title       Title of the notification
 * @param body        The text who will appears on the notification
 * @param level       Level of criticality of the notification
 */
var saveNotification = function (idOwner, idCubee, typeAlert, title,  body, level) {
    new mongoDbAlert.Alert({
        'idOwner': idOwner,
        'idCubee': idCubee,
        'typeAlert': typeAlert,
        'title': title,
        'body': body,
        'level' : level,
        'date': new Date()

    }).save(function (error) {
        if (error) {
            index.winston.error("Error saving a notification: " + error);
        }
    });
};
