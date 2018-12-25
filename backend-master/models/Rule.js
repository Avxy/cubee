var mongoose = require('mongoose');
var ruleTaskSchema = require('./RuleTask.js');

var ruleSchema = mongoose.Schema({
        idOwner: {
            type: String,
            required: true
        },
        name: {
            type: String,
            required: true,
            unique: true

        },
        ruleTasks:{
            type: Array,
            ref: ruleTaskSchema.RuleTask,
            require: true,
            default: []
        }
    });
exports.Rule = mongoose.model('Rule', ruleSchema);