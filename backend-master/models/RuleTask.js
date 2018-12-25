var mongoose = require('mongoose');
var ruleTaskSchema = mongoose.Schema({
        idCubee: {
            type: mongoose.Schema.ObjectId,
            ref: 'Cubee',
            required: true
        },
        cubeeName: {
            type: String,
            required: true
        },
        previousIdCubee: {
            type: String,
            required: false
        },
        typeAlert: {
            type: String,
            required: false
        },
        taskCommand:{
            type: Number,
            required: false
        }
    });
exports.RuleTask = mongoose.model('RuleTask', ruleTaskSchema);