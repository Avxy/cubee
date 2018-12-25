/**
 * Created by adson on 26/04/17.
 */
var express = require('express'),
    server = module.exports = express(),
    bodyParser = require('body-parser'),
    firebaseAdmin = require('firebase-admin'),
    environmentVariabels = require('./environmentVariables'),
    serviceAccount = environmentVariabels.fireBaseKeys;

firebaseAdmin.initializeApp({
    credential: firebaseAdmin.credential.cert(serviceAccount)
});


exports.getFirebaseAdmin = function () {
  return firebaseAdmin;
};

server.listen(8080);
server.use(bodyParser.json());

//ROUTES
var cubeeRoutes = require('../routes/cubeeRoutes'),
    userRoutes = require('../routes/userRoutes'),
    sectorRoutes = require('../routes/sectorRoutes'),
    alarmRoutes = require('../routes/alarmRoutes'),
    ruleRoutes = require('../routes/ruleRoutes'),
    eventRoutes = require('../routes/eventRoutes'),
    taskRoutes = require('../routes/taskRoutes'),
    db9Routes = require('../routes/db9Routes');

server.use('/cubee', cubeeRoutes);
server.use('/user', userRoutes);
server.use('/sector', sectorRoutes);
server.use('/alarm', alarmRoutes);
server.use('/rule', ruleRoutes);
server.use('/event', eventRoutes);
server.use('/task', taskRoutes);
server.use('/db9', db9Routes);
