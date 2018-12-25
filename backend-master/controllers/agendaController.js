var mqttClientController = require('./mqttClientController.js');
var featureController = require('./featureController');
var index = require('../index');

/**
 * This function defines a behavior for a job registered in agenda lib (npm)
 * Checks for events
 * @param agenda  Api reference for Agenda (npm)
 */
exports.defineJobs = function (agenda) {
    agenda.define('check for events', {priority: 'high', concurrency: 10}, function(job, done) {
        var data = job.attrs.data;
        featureController.getCommandTask(data.idCubee, function (error, command) {
            if (error) {
                index.winston.error(error.message);
            } else if (command) {
                mqttClientController.sendCommandJson(data.idCubee, command);
            } else {
                var unknownError = new Error("Unknown error at task search");
                unknownError.status = 400;
                index.winston.error(unknownError.message);
            }
            done();
        });

    });

};

/**
 * This function schedule a job already defined
 * @param date  Date of schedule
 * @param id    Id of the CUBEE related to the schedule
 */
exports.setAgenda = function (date, id) {
    var date = Date.parse(date);
    index.jobsAgenda.schedule(new Date(date), 'check for events', {idCubee: id});
    index.jobsAgenda.start();

};