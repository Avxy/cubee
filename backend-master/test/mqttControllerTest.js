var assert = require('chai').assert,
    request = require('supertest'),
    mqttClientController = require('../controllers/mqttClientController');

describe('MqttControllerTest',function () {
    it('Should register a cubee', function () {
        // registerCube(name, idCubee, oldIdCubee, idOwner, callback);
        var payload = {};
        payload.cubeeName = "Teste Cubee";
        payload.cubeeId = "Teste Cubee ID";
        payload.oldIdCubee = "";
        payload.idOwner = "IDOWNER";
        var result = mqttClientController.handleMessage("cubee/register", payload);
        assert.isOK(result, "ta ok ");
    })
});