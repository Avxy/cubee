/**
 * Created by matheus on 03/10/17.
 */
var mosca = require('mosca');


var ascoltatore = {
    //using ascoltatore
    type: 'mongo',
    url: 'mongodb://localhost:27017/mqtt',
    pubsubCollection: 'ascoltatori',
    mongo: {}
};

var settings = {
    port: 1883,
    backend: ascoltatore
};

var server = new mosca.Server(settings);

server.on('ready', setup);

server.on('published', function (packet, client) {
    console.log("Published :=", packet);
    try {
        console.log("PAYLOAD :=", packet.payload.toString());
    } catch (e) {
        // Oh well, but whatever...
    }
});

// fired when the mqtt server is ready
function setup() {
    console.log('Mosca server is up and running');
}

//EXTRA LOGS:
/*server.on("error", function (err) {
    console.log(err);
});

server.on('clientConnected', function (client) {
    console.log('Client Connected \t:= ', client.id);
});

server.on('subscribed', function (topic, client) {
    console.log("Subscribed :=", client.packet);
});

server.on('unsubscribed', function (topic, client) {
    console.log('unsubscribed := ', topic);
});

server.on('clientDisconnecting', function (client) {
    console.log('clientDisconnecting := ', client.id);
});

server.on('clientDisconnected', function (client) {
    console.log('Client Disconnected     := ', client.id);
});*/

