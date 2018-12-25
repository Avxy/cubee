/**
 * Created by adson on 26/04/17.
 */
var server = require('./config/serverConfig.js'),
    schedule = require('node-schedule'),
    winstonLog = require('winston'),
    mqtt = require('mqtt'),
    agenda = require('agenda');

//CONTROLLERS
var featureController = require('./controllers/featureController.js'),
    agendaController = require('./controllers/agendaController.js'),
    mqttClientController = require('./controllers/mqttClientController.js');


var mqttOptions = {
    host: 'localhost',
    port: 1883,
    connectTimeout: 10*1000,
    keepalive: 120,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    resubscribe: true
};

var client = mqtt.connect(mqttOptions);
module.exports.clientMqtt = client;

var mongoConnectionString = 'mongodb://127.0.0.1/agenda';
var connectionOpts = {db: {address: mongoConnectionString}};
var agendaAPI = new agenda(connectionOpts);
module.exports.jobsAgenda = agendaAPI;


var winston = new (winstonLog.Logger)({
    transports: [
        new (winstonLog.transports.Console)({
            name: 'info-console',
            level: 'warn'
        }),
        new (winstonLog.transports.File)({
            name: 'info-file',
            filename: 'filelog-info.log',
            level: 'info'
        }),
        new (winstonLog.transports.File)({
            name: 'error-file',
            filename: 'filelog-error.log',
            level: 'error'
        })
    ]
});
exports.winston = winston;

client.on('connect', function () {
    mqttClientController.subscribeToTopics();
});

client.on('message', mqttClientController.handleMessage); //on message received in any topic

agendaController.defineJobs(agendaAPI);
/**
 * @api {get} /
 * @apiGroup System
 *
 * @apiSuccess {String} status Authorized Access Message
 *
 * @apiSuccessExample {String} Success
 *      HTTP/1.1 200 OK
 *      "Server on"
 */
server.get('/', function (req, res) {
    winston.info('Server Check');
    res.end('Server on');
    winston.info('Server ON');
});

server.get('/tests', function (req, res) {
    featureController.saveMeasurement("59f1faf7f70b2f752363530e", 3, function (error, resp) {
        if (error) {
            console.log(error);
            res.status(error.status).send(error.message);
        } else {
            res.send(resp);
        }
    });
});


var resetMeasurement = schedule.scheduleJob('0, 0, 0, *, *, *', function () {
    featureController.resetMeasurement(function (error, resp) {
        if (error) {
            winston.error('WARNING! Error to reset measurements');
        } else {
            winston.info('Measurements sucessfully reseteds');
        }
    });
});