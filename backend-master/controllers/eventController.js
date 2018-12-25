/**
 * Created by adson on 19/07/17.
 */
var Event = require('../models/Event').Event;
var Task = require('../models/Task').Task;
var mongoDb = require('./dbController');
var moment = require('moment');
var agendaController = require('./agendaController.js');

/**
 * This function register a new Event
 *
 * @param uid        Id of the owner (User)
 * @param jsonEvent  Json representing the evento to be registred
 * @param callback   Callback used to send a response
 */
exports.registerEvent = function (uid, jsonEvent, callback) {
    if(uid === jsonEvent.idOwner){
        new Event({
            available: jsonEvent.available,
            idOwner: jsonEvent.idOwner,
            name: jsonEvent.name

        }).save(function (error, event) {
            if (error) {
                callback(error);
            } else if (event) {
                var taskList = jsonEvent.taskList;
                for (var i = 0; i < jsonEvent.taskList.length; i++) {
                    taskList[i].idEvent = event._id;
                    taskList[i]._id = mongoDb.getNewId();
                    taskList[i].dateTask = new Date(taskList[i].dateTask);
                    agendaController.setAgenda(taskList[i].dateTask, taskList[i].idCubee);

                    if (i === 0) {
                        taskList[i].previousTaskId = null;
                    } else {
                        taskList[i].previousTaskId = taskList[i - 1]._id;
                    }
                }

                Task.insertMany(taskList, function (error, list) {
                    if (error) {
                        console.log(require('util').inspect(error));
                        console.log(taskList);
                        callback(error);
                    } else {
                        callback(null, list);
                    }
                });
            }
        });
    }else{
        var err = new Error("This event don't belong to this user");
        err.status = 400;
        callback(err);
    }
};

/**
 * This function returns all events of a specific User
 *
 * @param idOwner  Id of the owner (User)
 * @param callback Callback used to send a response
 */
exports.getEventsByUser = function (idOwner, callback) {
    Event.find({idOwner: idOwner}, function (error, events) {
        if (events) {
            callback(null, events);
        } else if (error) {
            error.status = 400;
            callback(error);
        } else {
            var err = new Error("There's no event registred by this user");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This function returns all tasks who belongs a specific event
 *
 * @param idEvent  id of the Event
 * @param callback Callback used to send a reponse
 */
exports.getTasksByEvent = function (idEvent, callback) {
    Task.find({idEvent: idEvent}, function (error, tasks) {
        if (tasks){
            callback(null, tasks);
        } else if (error) {
            error.status = 400;
            callback(error);
        } else {
            var err = new Error("There's no tasks registred by this event");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This functions deletes a specific event
 *
 * @param idOwner  id of the owner (User)
 * @param idEvent  id of the Event
 * @param callback Callback used to send a response
 */
exports.deleteEvent = function (idOwner, idEvent, callback) {
    Event.findById(idEvent, function (error, event) {
        if (error) {
            error.status = 400;
            callback(error);
        } else if (event && event.idOwner === idOwner) {
            Event.remove({_id: idEvent}, function (error) {
                if (error) {
                    error.status = 400;
                    callback(error);
                } else {
                    callback(null, event);
                }
            });
        } else {
            var err = new Error("This event can't be deleted");
            err.status = 400;
            callback(err);
        }
    });
};

/**
 * This funcion delets all events of a specific CUBEE
 *
 * @param idOwner  id of the owner (User)
 * @param idCubee  id of the CUBEE in question
 * @param callback Callback used to send a response
 */
exports.deleteEventByCubee = function (idOwner, idCubee, callback) {
    Task.find({idCubee: idCubee}, function (error, task) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            for (var t in task) {
                var idEvent = task[t].idEvent;
                Event.remove({_id: idEvent}, function (error) {
                    if (error) {
                        error.status = 400;
                    }
                });

                Task.remove({idEvent: idEvent}, function (error) {
                    if (error) {
                        error.status = 400;
                    }
                });
            }
            callback(null, task);
        }
    });
};

/**
 * This function returns all events of an CUBEE
 * @param idOwner  id of the owner (User)
 * @param idCubee  id of the CUBEE
 * @param callback Callback used to send a reponse
 */
exports.findEventsByCubee = function (idOwner, idCubee, callback) {
    Task.find({idCubee: idCubee}, function (error, tasks) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            var idEvents = [];
            for (var i in tasks) {
                idEvents[i] = tasks[i].idEvent;
            }
            callback(null, idEvents);
        }
    });
};

/**
 * This function deletes all tasks who belongs to an event
 *
 * @param idEvent   Id of the event in question
 * @param callback  Callback used to send a response
 */
exports.deleteTasksByEvent = function (idEvent, callback) {
    Task.remove({idEvent: idEvent}, function (error) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, "ok");
        }
    });
};

/**
 * This function returns the command of a task for a specific CUBEE, if existis any task to be done. Otherwise, returns a error message
 *
 * @param idCubee  Id of the CUBEE in question
 * @param callback Callback used to send a response
 */
exports.getCommandTask = function (idCubee, callback) {
    var now = Date.parse(new Date());
    var end = new Date(now + 3600); //NOW + 3 SEC
    var begin = new Date(now - 3600);//NOW - 3 SEC
    Task.findOne({idCubee: idCubee, dateTask: {$gte: begin, $lte: end}, done: false}, function (error, task) {
        if (error) {
            error.status = 400;
            callback(task);
        } else if (task) {
            task.done = true;
            task.save(function (error, successTask) {
                if (error) {
                    callback(error);
                } else {
                    callback(null, successTask);
                }
            });
        } else {
            var taskDontFindedError = Error("Task not Found");
            taskDontFindedError.status = 400;
            callback(taskDontFindedError);
        }

    });
};

/**
 * This function returns all the task of a specific CUBEE
 *
 * @param idCubee   Id of the CUBEE to get the tasks
 * @param callback  Callback used to send a response
 */
exports.getTaskByCubee = function (idCubee, callback) {
    Task.find({idCubee: idCubee}, function (error, tasks) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, tasks);
        }
    });
};

/**
 * This function returns all events of a specific CUBEE
 *
 * @param uid      Id of the owner of the CUBEE (User)
 * @param idCubee  Id of the CUBEE
 * @param callback Callback used to send a response
 */
exports.getEventsByCubee = function (uid, idCubee, callback) {
    Task.find({idCubee: idCubee}).populate('idEvent').exec(function (error, tasks) {
        if (error) {
            error.status = 400;
            callback(error);
        } else if (tasks) {
            var idEvents = [];
            var add = 0;
            for (var i in tasks) {
                if("idOwner" in tasks[i].idEvent){
                    if(tasks[i].idEvent.idOwner !== uid){
                        var err = new Error("This task don't belong to this user");
                        err.status = 400;
                        return callback(err);
                    }
                    if (tasks[i].idEvent.available === true) {
                        idEvents[add++] = (tasks[i].idEvent._id);
                    }
                }
            }
            console.log("TESTE:" + idEvents);
            callback(null, idEvents);
        } else {
            var unknowError = Error("Unknow error");
            unknowError.status = 400;
            callback(unknowError);
        }
    });
};

/**
 * This function set the events to "available" or "unavailable", representing with a flag
 *
 * @param flagOfAvailable  Flag who represents the availability of an Event. True for "available" or False for "unavailable"
 * @param listOfIdEvents   List with Ids of events to be setted
 * @param callback         Callback used to send a response
 */
exports.setAvailableEvents = function (flagOfAvailable, listOfIdEvents, callback) {
    Event.find({_id : { $in : listOfIdEvents}}, function (error, events) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            events.forEach(function (event) {
                event.available = flagOfAvailable;
                event.save(function (error) {
                    if (error) {
                        callback(error);
                    }
                });
            });
            callback(null, "ok");
        }
    });
};

/**
 * This function delete events by Id
 *
 * @param listOfIdEvents  List of id of events to delete.
 * @param callback        Callback used to send a response
 */
exports.deleteEvents = function (listOfIdEvents, callback) {
    listOfIdEvents.forEach(function (eventId) {
        Task.remove({idEvent: eventId}, function (error) {
            if (error) {
                error.status = 400;
                return callback(error);
            }
        });
    });
    Event.remove({_id : {$in : listOfIdEvents}}, function (error) {
        if (error) {
            error.status = 400;
            callback(error);
        } else {
            callback(null, "ok");
        }
    });
};

/**
 * This function returns all the tasks of events in a list
 *
 * @param listIdEvents  List of id of Events
 * @param callback      Callback used to send a response.
 */
exports.getTasksByManyEvents = function (listIdEvents, callback) {
    Task.aggregate([{$match: {idEvent: {$in: listIdEvents}}},
        {
            $group: {
                _id: "$idEvent",
                //,total: {$sum : 1}}
                tasks: {$push: {_id: "$_id", dateTask: "$dateTask", cubeeName: "$cubeeName", idEvent: "$idEvent"}}
            }
        }], function (error, item) {

            if (error) {
                error.status = 400;
                callback(error);
            } else if (item) {
                callback(null, item);
            } else {
                var unknowError = Error("Unknow error");
                unknowError.status = 400;
                callback(unknowError);
            }
    });
};




