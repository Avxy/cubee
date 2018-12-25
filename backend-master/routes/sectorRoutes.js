/**
 * Created by adson on 14/09/17.
 */
var sectorRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /sector
 * @apiName getSectors
 * @apiGroup Sector
 *
 * @apiDescription This route returns all the sectors of a specific User
 *
 * @apiHeader (Sector) {String} idToken FirebaseToken for the User owner
 * @apiHeaderExample {json} Request-Example:
 *      {"userToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} Sector  an array of SectorModels
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: User not found"
 *  }
 */
sectorRouter.get('/', function (req, res) {
    var idToken = req.header('userToken');
    featureController.getSectorsByUser(idToken, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

sectorRouter.post('/delete', function (req, res) {
    var idSector = req.body.idSector;
    var idToken = req.body.idToken;



    featureController.deleteSector(idToken, idSector, function (error, resp) {
        if (error) {
            console.log(error);
            res.status(error.status).send(error.message);
        } else {
            res.send(resp);
        }
    });
});


sectorRouter.post('/register', function (req, res) {
    var sectorName = req.body.sectorName;
    var userToken = req.body.userToken;
    var listIdCubees = req.body.listIdCubees;

    if (listIdCubees) {
        listIdCubees = JSON.parse(listIdCubees);
    }
    featureController.registerSector(userToken, sectorName, listIdCubees, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send(resp);
        }
    });
});

sectorRouter.post('/registercubees', function (req, res) {
    var idSector = req.body.idSector;
    var idToken = req.body.idToken;
    var listIdCubees = JSON.parse(req.body.listIdCubees);

    featureController.registerCubeeInSector(idSector, idToken, listIdCubees, function (error) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info('success');
            res.send({status: "ok"});
        }
    });
});

sectorRouter.post('/unregistercubees', function (req, res) {
    var idToken = req.body.idToken;
    var listIdCubees = JSON.parse(req.body.listIdCubees);
    var idSector = null;

    featureController.registerCubeeInSector(idSector, idToken, listIdCubees, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.send({status: 'cubees unregistred successfully'});
        }
    });
});

var sectorController = require('../controllers/sectorController');
sectorRouter.get('/test', function (req, res) {
    sectorController.teste(function (error, sector) {
        if(sector){
            index.winston.info(sector);
        }else{
            index.winston.error(error);
        }
    });
});

module.exports = sectorRouter;