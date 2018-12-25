var mongoose = require('mongoose');

var alertSchema = mongoose.Schema({
        idOwner: {
            type: String,
            required: true
        },
        idCubee: {
            type: mongoose.Schema.ObjectId,
            ref: 'Cubee',
            required: true
        },
        typeAlert: {
            type: String,
            required: true
        },
        title: {
            type: String,
            required: true
        },
        body: {
            type: String,
            required: true
        },
        level:{
            type:String,
            required: true
        },
        date: {
            type: Date,
            required: true
        }
    });
exports.Alert = mongoose.model('Alert', alertSchema);