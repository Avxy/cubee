/**
 * Created by adson_silva on 17/09/17.
 */
var taskRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

taskRouter.get('/byEvent', function (req, res) {
    var idToken = req.header('idToken');
    var idEvent = req.header('idEvent');
    featureController.getTasksByEvent(idToken, idEvent, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

taskRouter.get('/byCubee', function (req, res) {
    var idCubee = req.header('idCubee');
    var idToken = req.header('idToken');

    featureController.getTasksByCubee(idToken, idCubee, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});


module.exports = taskRouter;