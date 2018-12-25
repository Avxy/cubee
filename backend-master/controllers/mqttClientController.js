/**
 * Created by matheus on 03/10/17.
 */
var index = require('../index'),
    featureController = require('./featureController'),
    db9Controller = require('./db9RuleController');

/**
 * Receives a cubee and send his appCommand via mqtt to topic
 * /cubee/(cubee id)/command
 * @param cubee cubee to send app Command
 */
exports.findAndSendAppCommand = function (cubee) {
    var command = {};
    var options = {};
    options.qos = 1;
    if (cubee) {
        db9Controller.checkForDB9Rule(cubee, function (checkDb9Error, db9Rule) {
            if (db9Rule) {
                command.appCommand = 5;
                command.db9rule = db9Rule;
                index.clientMqtt.publish("cubee/" + cubee._id + "/command", JSON.stringify(command), options);
            } else {
                featureController.getCommandFromModule(cubee, function (getCommandError, commandFromModel) {
                    if (getCommandError) {
                        index.winston.error(getCommandError.message + cubee);
                    } else if (commandFromModel) {
                        command = commandFromModel;
                        index.clientMqtt.publish("cubee/" + cubee._id + "/command", JSON.stringify(command), options);

                    }

                });
            }
        });

    }
};

/**
 * Send a command via mqtt to a cubee
 * @param cubeeId cubee to send a command
 * @param command command to send, must be a string
 */
exports.sendCommand = function (cubeeId, command) {
    var options = {};
    options.qos = 1;
    console.log(command);
    if (cubeeId) {
        try {
            index.clientMqtt.publish("cubee/" + cubeeId + "/command", command, options);
        } catch (e) {
            index.winston.error(e.message);
        }
    }
};


/**
 * Send a command via mqtt to a cubee
 * @param cubeeId cubee to send a command
 * @param commandJson command to send, in a JSON format
 */
exports.sendCommandJson = function (cubeeId, commandJson) {
    console.log(commandJson.toString());
    this.sendCommand(cubeeId, JSON.stringify(commandJson));
};

/**
 * Send a command via mqtt to a cubee
 * @param cubeeId cubee to send a command
 * @param command command to send, must be a string
 */
exports.teste = function (cubeeId, command) {
    var options = {};
    options.qos = 1;
    if (cubeeId) {
        index.clientMqtt.publish("cubee/command/" + cubeeId, command, options);
    }
};

/**
 * Send a message to cubee via mqtt
 * Topic used is composed by a passed cubee Id and topic
 * @param cubeeID cubee Id to compose topic
 * @param topic string to compose topic
 * @param message message to send
 */
exports.sendMessageToCubee = function (cubeeID, topic, message) {
    index.clientMqtt.publish("cubee/" + cubeeID + topic, message);
};

/**
 * Subscribe to some topics, setup mqtt communication
 */
exports.subscribeToTopics = function () {
    index.clientMqtt.subscribe({
        "cubee/command": 1,
        "cubee/states": 1,
        "cubee/register": 1,
        "cubee/alarm": 1,
        "cubee/measurement": 1
    });
};

/**
 * Receive a message via mqtt from subscribed topics
 * @param topic String to describe the topic
 * @param payload message received
 */
exports.handleMessage = function (topic, payload) {
    console.log(payload.toString() + " at topic " + topic);
    var split = topic.split("/");
    var subTopicString = split[1];

    var payloadInJson = safelyParseJSON(payload.toString());

    var commandTopic = "command",
        statesTopic = "states",
        registerTopic = "register",
        alarmTopic = "alarm",
        measurementTopic = "measurement";

    if (payloadInJson) {
        switch (subTopicString) {
            case statesTopic:
                statesSubscribeMessage(payloadInJson);
                break;
            case measurementTopic:
                measurementSubscribeMessage(payloadInJson);
                break;
            case registerTopic:
                registerSubscribeMessage(payloadInJson);
                break;
            case alarmTopic:
                alarmSubscribeMessage(payloadInJson);
                break;
            case commandTopic:
                commandSubscribeMessage(payloadInJson);
                break;
            default:
                index.winston.info("MQTT SubTopic not found");

        }
    }
};

var statesSubscribeMessage = function (msgJson) {
    //exports.setInformationsCubee = function(idCubee, state, signal){
    featureController.setCubeeState(msgJson.idCubee, msgJson.state, function (error, success) {
        if (error) {
            index.winston.error("Erro em state: " + error.toString());
        } else {
            index.winston.info("State success:" + success.toString());
        }
    });
};

var measurementSubscribeMessage = function (msgJson) {
    //exports.saveMeasurement = function (idCubee, measure, callback) {
    featureController.saveMeasurement(msgJson.idCubee, msgJson.measurement, function (error, success) {
        if (error) {
            index.winston.error("Erro em measurement: " + error.toString());
        } else {
            index.winston.info(success.toString());
        }
    });
};

var registerSubscribeMessage = function (msgJson) {
    //exports.registerCubee = function (cubeeName, cubeeId, oldIdCubee, idOwner, callback) {
    featureController.registerCubee(msgJson.cubeeName, msgJson.idCubee, msgJson.oldIdCubee, msgJson.userId, function (error, success) {
        if (error) {
            index.winston.error("Erro em register: " + error.toString());
        } else {
            index.winston.info(success.toString());
        }
    });
};

var alarmSubscribeMessage = function (msgJson) {
    //exports.sendAlarmsFromCubee = function (idCubee, alarms, callback) {
    featureController.sendAlarmsFromCubee(msgJson.idCubee, msgJson.alarms, function (error, success) {
        if (error) {
            index.winston.error("Erro em alarm: " + error.toString());
        } else {
            index.winston.info(success.toString());
        }
    });
};

var commandSubscribeMessage = function (msgJson) {
    //exports.setInformationsCubee = function(idCubee, state, signal){
    featureController.setCubeeSignal(msgJson.idCubee, msgJson.signal, function (error, success) {
        if (error) {
            index.winston.error("Erro em command: " + error.toString());
        } else {
            index.winston.info(success.toString());
        }
    });
};

function safelyParseJSON(json) {
    // This function cannot be optimised, it's best to
    // keep it small!
    var parsed;

    try {
        parsed = JSON.parse(json);
    } catch (e) {
        // Oh well, but whatever...
    }

    return parsed; // Could be undefined!
}