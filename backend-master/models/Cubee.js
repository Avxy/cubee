var mongoose = require('mongoose');
var mqttClientController = require('./../controllers/mqttClientController');

var cubeeSchema = mongoose.Schema({
        _id: {
            type: String,
            required: true
        },
        idOwner: {
            type: String,
            required: true
        },
        idSector: {
            type: String,
            default: null
        },
        cubeeState: {
            type: Boolean,
            required: true,
            default: false
        },
        cubeeBtn: {
            type: Boolean,
            required: false,
            default: false
        },
        validated: {
            type: Boolean,
            required: true,
            default: false
        },
        upperThreshold: {
            type: Number,
            required: false
        },
        lowerThreshold: {
            type: Number,
            required: false
        },
        name: {
            type: String,
            required: true,
            default: false
        },
        db9RuleId:{
            type: String,
            required: false,
            default: ""
        },
        db9RuleHadSetFlag:{
            type: Boolean,
            required: false,
            default: false
        }
    });
module.exports = mongoose.model('Cubee', cubeeSchema);

