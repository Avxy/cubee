var mongoose = require('mongoose');
var taskSchema = mongoose.Schema({
        idEvent: {
            type: mongoose.Schema.ObjectId,
            ref: 'Event',
            required: true
        },
        idCubee: {
            type: String,
            required: true
        },
        cubeeName: {
            type: String,
            required: true
        },
        previousTaskId: {
            type: String
            //required: true,
        },
        appCommand: {
            type: Number,
            required: true
        },
        dateTask: {
            type: Date,
            required: true
        },
        done: {
            type: Boolean,
            required: true,
            default: false
        }
    });
exports.Task = mongoose.model('Task', taskSchema);
