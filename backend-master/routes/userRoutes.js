/**
 * Created by adson on 05/09/17.
 */
var userRouter  = require('express').Router();
var featureController = require('../controllers/featureController');
var index = require('../index');

/**
 * @api {get} /user
 * @apiName getUser
 * @apiGroup User
 *
 * @apiDescription This route returns the User by idToken
 *
 * @apiHeader (User) {String} idToken FirebaseToken for the User
 * @apiHeaderExample {json} Request-Example:
 *      {"idToken": "çlasdhfÇIHDAFASfçOIHASDFÇLKHGÇKLhgKLGHLKGLYPIOuOIUçUGHÇLkjHÇuhÇOUyouya0f897A07çoAIHSF"}
 *
 * @apiSuccess {json} user The User requested by the idToken
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "5aJnPYBYJkdB1ycAyrCkrWsZzaN2",
 *          "name" : "Cubee1",
 *          "email" : "cubee1@cubee.com",
 *          "telephone" : "111111",
 *          "__v" : 0
 *      }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid token"
 *  }
 */
userRouter.get('/', function (req, res) {
    var idToken = req.header("idToken");

    featureController.getUser(idToken, function (error, resp) {
        if (error) {
            // index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            // index.winston.info(resp.toString());
            res.send(resp);
        }
    });

});

/**
 * @api {post} /user/register
 * @apiName registerUser
 * @apiGroup User
 *
 * @apiDescription This route realizes the register of the user by email and password
 *
 * @apiParam {String} name Mandatory Name of the User
 * @apiParam {String} email Mandatory Email of the User
 * @apiParam {String} telephone Mandatory Telephone of the User
 * @apiParam {String} password Mandatory Password of the User
 *
 * @apiSuccess {json} registred user
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "5aJnPYBYJkdB1ycAyrCkrWsZzaN2",
 *          "name" : "Cubee1",
 *          "email" : "cubee1@cubee.com",
 *          "telephone" : "111111",
 *          "__v" : 0
 *      }
 *
 * @apiError ValidationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "User validation failed"
 *  }
 */
userRouter.post('/register', function (req, res) {
    var name = req.body.name;
    var email = req.body.email;
    var telephone = req.body.telephone;
    var password = req.body.password;

    featureController.registerEPUser(name, email, telephone, password, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.json(resp);
        }
    });
});

/**
 * @api {post} /user/edit
 * @apiName editUser
 * @apiGroup User
 *
 * @apiDescription This route edits a specific user
 *
 * @apiParam {String} uid        Optional  Id of the User
 * @apiParam {String} name       Optional  Name of the User
 * @apiParam {String} email      Optional  Email of the User
 * @apiParam {String} telephone  Optional  Telephone of the User
 * @apiParam {String} password   Optional  Password of the User
 *
 * @apiSuccess {json} user The User edited
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "_id" : "5aJnPYBYJkdB1ycAyrCkrWsZzaN2",
 *          "name" : "Cubee2",
 *          "email" : "cubee1@cubee.com",
 *          "telephone" : "111111",
 *          "__v" : 0
 *      }
 *
 * @apiError AuthenticationError
 *
 * @apiErrorExample {json} Error-Response:
 *  HTTP/1.1 400 Bad Request
 *  {
 *      "error" : "Authentication Error: invalid password"
 *  }
 */
userRouter.post('/edit', function (req, res) {
    var idToken = req.body.idToken;
    var name = req.body.name;
    var email = req.body.email;
    var telephone = req.body.telephone;
    var password = req.body.password;

    featureController.editUser(idToken, name, email, telephone, password, function (error, resp) {
        if (error) {
            index.winston.error(error);
            res.status(error.status).send(error.message);
        } else {
            index.winston.info(resp.toString());
            res.json(resp);
        }
    });
});

userRouter.post('/registrationToken', function (req, res) {
    var registrationToken = req.body.registrationToken;
    var idToken = req.body.idToken;

    featureController.saveRegistrarionToken(registrationToken, idToken, function (error, success) {
        if(error){
            index.winston.error(error);
            res.send(error);
        }else{
            index.winston.info(success);
            res.send(success);
        }
    });
});

module.exports = userRouter;