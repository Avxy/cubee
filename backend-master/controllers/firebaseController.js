/**
 * Created by adson on 03/05/17.
 */
var firebaseAdmin = require('firebase-admin');
var auth = firebaseAdmin.auth();

/**
 * This function returns defineJobs user id by the FirebaseToken
 *
 * @param idToken   User FirebaseToken
 * @param callback  Callback used to send defineJobs response
 */
exports.getUid = function (idToken, callback) {
    auth.verifyIdToken(idToken).then(function (decodedToken) {
        var uid = decodedToken.uid;
        callback(null, uid);
    }).catch(function (error) {
        error.status = 400;
        callback(error);
    });
};

/**
 * This function returns defineJobs FirebaseUser by the User ID
 *
 * @param uid       User id
 * @param callback  Callback used to send defineJobs response
 */
exports.getUserRecordById = function (uid, callback) {
    auth.getUser(uid).then(function (userRecord) {
        callback(null, userRecord);
    }).catch(function (error) {
        console.log(error.message);
        var err = new Error('Authentication Error: User not found');
        err.status = 400;
        callback(err);
    });
};

/**
 * This function create defineJobs FirebaseUser
 *
 * @param name      User name
 * @param email     User email
 * @param password  User password
 * @param callback  Callback used to send defineJobs response
 */
exports.createFirebaseUser = function (name, email, password, callback) {
    auth.createUser({
        email: email,
        emailVerified: false,
        password: password,
        displayName: name,
        photoURL: "http://www.example.com/12345678/photo.png",
        disabled: false
    })
        .then(function(userRecord) {
            // See the UserRecord reference doc for the contents of userRecord.
            callback(null, userRecord.uid);
        }).catch(function(error) {
            error.status = 400;
            callback(error);
        });
};

/**
 * This function updates defineJobs FirebaseUser
 *
 * @param uid       User id
 * @param toUpdate  Params to update
 * @param callback  Callback used to send defineJobs response
 */
exports.updateFirebaseUser = function (uid, toUpdate, callback) {
    auth.updateUser(uid, toUpdate).then(function (userRecord) {
        callback(null, userRecord);

    }).catch(function (error) {
        console.log(error.message);
        var err = new Error('Register Error. FirebaseUpdate Error');
        err.status = 400;
        callback(err);
    });
};
