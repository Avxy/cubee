/**
 * Created by adson on 27/04/17.
 */
var environmentVariables = require('../config/environmentVariables');

var ObjectId = require('mongodb').ObjectId;
var mongoose = require('mongoose').connect(environmentVariables.db_string);
var db = mongoose.connection;


db.on('error', console.error.bind(console, 'Erro ao conectar no banco'));

exports.getNewId = function () {
    var objectId = new ObjectId();
    return objectId.toHexString();
};



