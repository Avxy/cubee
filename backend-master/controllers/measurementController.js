/**
 * Created by adson on 03/08/17.
 */

var Measurement = require('../models/Measurement').Measurement;
var Cubee = require('../models/Cubee');
var ObjectId = require('mongodb').ObjectID;
var index = require('../index');

var notificationController = require('./notificationController');

/**
 * This function register a new measurment in the database
 * @param idCubee         Id of the cubee related to the measurement
 * @param currentMeasure  Measure to be saved
 * @param callback        Callback used to send a response
 */
exports.saveMeasurement = function (idCubee, currentMeasure, callback) {
    var now = new Date();
    var currentHour = now.getHours();
    var currentMinute = now.getMinutes();

    now.setHours(0);
    now.setMinutes(0);
    now.setSeconds(0);
    now.setMilliseconds(0);

    Cubee.findById(idCubee, function (error, cubee) {
        if (cubee) {
            Measurement.find({date: now, idCubee: idCubee}, function (error, measurement) {
                if (error) {
                    error.status = 400;
                    callback(error);
                } else {
                    sendAlarmMeasurement(idCubee, currentMeasure);
                    if (!measurement[0]) {
                        new Measurement({
                            date: now,
                            idCubee: idCubee,
                            hours:[
                                {hour:currentHour, minutes : [
                                    {minute: currentMinute, measure: currentMeasure}
                                ]
                                }
                            ]
                        }).save(function (error, meas) {
                            if(error){
                                callback(error);
                            }else{
                                callback(null, "status_ok");
                            }
                        });

                    } else {
                        var hourRegistered = false;
                        measurement[0].hours.forEach(function (hourDb) {
                            if (currentHour === hourDb.hour) {

                                hourRegistered = true;
                                hourDb.minutes.push({
                                    minute: currentMinute,
                                    measure:currentMeasure
                                });
                                measurement[0].save(function (error, meas) {
                                    if(error){
                                        callback(error);
                                    }else{
                                        callback(null, "status_ok");
                                    }
                                });

                            }
                        });

                        if (!hourRegistered) {
                            measurement[0].hours.push({
                                hour: currentHour, minutes: [
                                    {minute: currentMinute, measure: currentMeasure}
                                ]
                            });
                            measurement[0].save(function (error) {
                                if (error) {
                                    callback(error);
                                } else {
                                    callback(null, {status: "status_ok"});
                                }
                            });
                        }
                    }
                }
            });
        } else {
            var err = new Error("cubee not found");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function verifies if any measurement exceeded the threshold, verifies the type of them(upper or lower) and
 * delegates to notificationController to send a notification to the owner of the CUBEE(User)
 * @param idCubee      Id of the cubee
 * @param measurement  measurement recieved
 */
var sendAlarmMeasurement = function (idCubee, measurement) {
    Cubee.findById(idCubee, function (error, cubee) {
        if (error) {
            index.winston.error(error.message);
        }
        else if (cubee) {
            if ("lowerThreshold" in cubee && "upperThreshold" in cubee) {
                if (measurement < cubee.lowerThreshold) {
                    notificationController.sendNotification(cubee.idOwner, cubee._id, LOWER_THRESHOLD_ALERT_TYPE, "Medição menor que o esperado!",
                        "A medição do " + cubee.name + " recebida está abaixo do limiar inferior configurado.", "HIGH");
                }
                if (measurement > cubee.upperThreshold) {
                    notificationController.sendNotification(cubee.idOwner, cubee._id, UPPER_THRESHOLD_ALERT_TYPE, "Medição maior que o esperado!",
                        "A medição do " + cubee.name + " recebida está acima do limiar superior configurado.", "HIGH");
                }
            }
        }
    });
};

/**
 * This function returns, group by minutes,  all the measurements of a specific CUBEE who belongs to a time interval
 * @param idCubee    Id of the CUBEE in question
 * @param day        Number representing the day requested
 * @param startHour  The initial hour of the time interval
 * @param endHour    The finish hour of the time interval
 * @param callback   Callback used to send a response
 */
exports.getCubeeMeasurementsInMinutes = function (idCubee, day, startHour, endHour, callback) {
    Measurement.aggregate([
        {$match: {idCubee:idCubee, date: day}},
        // {$match: {idCubee : ObjectId(idCubee)}}
        {$unwind: '$hours'},
        {
            $match: {
                '$and': [{'hours.hour': {'$gte': startHour}},
                    {'hours.hour': {'$lte': endHour}}]
            }
        },
        {$project: {_id: 0, hours: 1}}
    ]).exec(function (error, result) {
            console.log("result   " + result[0]);
            var hours = [];
            var measurements = [];
            var start = startHour;

            for (var i = 0; i < endHour - startHour + 1; i++) {
                for (var minute = 0; minute < 60; minute++) {
                    hours.push(start + ":" + minute);
                    measurements.push(0);
                }
                start++;
            }
            if (result[0]) {
                var hour = result[0].hours.hour;
                result[0].hours.minutes.forEach(function (minn) {
                    var minut = minn.minute;
                    var dateString = hour + ":" + minut;

                    var index = hours.indexOf(dateString);
                    measurements[index] = minn.measure;

                });
            }
            var labelsAndValues = {keys: hours, values: measurements};
            callback(null, labelsAndValues);
        });

};
/**
 * This function returns, group by hours,  all the measurements of a specific CUBEE who belongs to a time interval
 * @param idCubee   Id of the CUBEE in question
 * @param startDay  Number representing the initial day of the time interval
 * @param startHour The initial hour of the time interval
 * @param endDay    Number representing the initial day of the time interval
 * @param endHour   The finish hour of the time interval
 * @param callback  Callback used to send a response
 */
exports.getCubeeMeasurementsInHours = function (idCubee, startDay, startHour, endDay, endHour, callback) {
    var diffMilisseconds = endDay - startDay;
    var diffSeconds = diffMilisseconds / 1000;
    var diffMinutes = diffSeconds / 60;
    var diffHours = diffMinutes / 60;
    var diffDays = (diffHours / 24) + 1;
    var dateString = "/";

    var date = new Date(startDay);
    var keys = [];
    var values = [];

    var index = 0;
    for (var i = 0; i < diffDays; i++) {
        var start = 0;
        var end = 24;
        if (i === 0) {
            start = startHour;
        }
        if (i === diffDays - 1) {
            end = endHour;
        }
        dateString = date.getDate() + "/" + (date.getMonth() + 1);
        date.setTime(date.getTime() + (24 * 60 * 60 * 1000));
        for (var j = start; j <= end; j++) {
            keys[index] = dateString + " " + j + "h";
            values[index] = 0;
            index++;
        }
    }

    Measurement.find({idCubee: idCubee, date: {$gte: startDay, $lte: endDay}}, function (error, measurements) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            var dateString = "";
            var index;
            measurements.forEach(function (measurement) {
                dateString = measurement.date.getDate() + "/" + (measurement.date.getMonth() + 1);
                measurement.hours.forEach(function (h) {
                    index = keys.indexOf(dateString + " " + h.hour + "h");
                    values[index] = h.sumHour;
                });
            });
            var labelsAndValues = {keys: keys, values: values};
            callback(null, labelsAndValues);
        }
    });
};

/**
 * This function returns, group by days,  all the measurements of a specific CUBEE who belongs to a time interval
 * @param idCubee   Id of the CUBEE in question
 * @param startDay  Number representing the initial day of the time interval
 * @param endDay    Number representing the initial day of the time interval
 * @param callback  Callback used to send a response
 */
exports.getCubeeMeasurementsInDays = function (idCubee, startDay, endDay, callback) {
    var diffMilisseconds = endDay - startDay;
    var diffSeconds = diffMilisseconds / 1000;
    var diffMinutes = diffSeconds / 60;
    var diffHours = diffMinutes / 60;
    var diffDays = (diffHours / 24) + 1;
    var dateString = "/";

    var date = new Date(startDay);
    var keys = [];
    var values = [];

    for (var i = 0; i < diffDays; i++) {
        dateString = date.getDate() + "/" + (date.getMonth() + 1);
        date.setTime(date.getTime() + (24 * 60 * 60 * 1000));
        keys[i] = dateString;
        values[i] = 0;
    }

    Measurement.find({idCubee: idCubee, date: {$gte: startDay, $lte: endDay}}, function (error, measurements) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            var dateString = "";
            var index;
            measurements.forEach(function (measurement) {
                dateString = measurement.date.getDate() + "/" + (measurement.date.getMonth() + 1);
                index = keys.indexOf(dateString);
                values[index] = measurement.hour.sum;
            });

            var labelsAndValues = {keys: keys, values: values};
            callback(null, labelsAndValues);
        }
    });
};
/**
 * This function returns, group by months,  all the measurements of a specific CUBEE who belongs to a time interval
 * @param idCubee   Id of the CUBEE in question
 * @param startDay  Number representing the initial day of the time interval
 * @param endDay    Number representing the initial day of the time interval
 * @param callback  Callback used to send a response
 */
exports.getCubeeMeasurementsInMonths = function (idCubee, startDay, endDay, callback) {
    var start = new Date(startDay);
    var end = new Date(endDay);
    start.setDate(1);

    end.setDate(1);
    end.setMonth(end.getMonth() + 1);
    end.setDate(end.getDate() - 1);

    var dateString = "/";
    var diffMilisseconds = endDay - startDay;
    var diffSeconds = diffMilisseconds / 1000;
    var diffMinutes = diffSeconds / 60;
    var diffHours = diffMinutes / 60;
    var diffDays = diffHours / 24;
    var diffMonths = (diffDays / 30);

    var date = new Date(startDay);
    var keys = [];
    var values = [];

    for (var i = 0; i < diffMonths; i++) {
        dateString = date.getMonth() + 1 + "/" + (date.getFullYear());
        date.setMonth(date.getMonth() + 1);
        keys[i] = dateString;
        values[i] = 0;
    }

    Measurement.aggregate([
        {$match: {idCubee: idCubee, date: {$gte: start, $lt: end}}},
        {$sort: {date: -1}},
        {$unwind: '$hours'},
        {$unwind: '$hours.minutes'},
        {$group: {_id: {month: {$month: "$date"}, year: {$year: "$date"}}, measure: {$sum: "$hours.minutes.measure"}}}
    ]).exec(function (error, result) {
        if (error) {
            callback(error);
        } else {
            result.forEach(function (measureMonth) {
                var stringKey = measureMonth._id.month + "/" + measureMonth._id.year;
                var index = keys.indexOf(stringKey);
                values[index] = measureMonth.measure;
            });

            var labelsAndValues = {keys: keys, values: values};
            callback(null, labelsAndValues);
        }
    });
};

/**
 * This function resets the measurments in the database in 30 days
 * @param callback  Callback used to send a response
 */
exports.resetMeasurement = function (callback) {
    var DAYS_TO_RESET = 30;

    var timeToReset = new Date();
    timeToReset.setDate(timeToReset.getDate() - DAYS_TO_RESET);

    var start = timeToReset.setHours('0, 0, 0, 0');
    var end = timeToReset.setHours('23, 59, 59, 59');

    Measurement.remove({date: {$gte: start, $lt: end}}, function (error, mesurements) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, mesurements);
        }
    });
};
