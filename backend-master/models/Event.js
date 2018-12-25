var mongoose = require('mongoose');

var eventSchema = mongoose.Schema({
        name: {
            type: String,
            required: true,
            unique: true
        },
        idOwner: {
            type: String,
            required: true
        },
        available: {
            type: Boolean,
            required: true
        }
    });
exports.Event = mongoose.model('Event', eventSchema);
