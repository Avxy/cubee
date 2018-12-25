var mongoose = require('mongoose');

var db9RuleSchema = mongoose.Schema({
    name: {
        type: String,
        required: true
    },idOwner: {
        type: String,
        required: true
    },
    nStates: {
        type: Number,
        required: true
    },
    states:{
        type: Array,
        ref: stateSchema,
        required: true,
        default: []
    }
});

exports.DB9Rule = mongoose.model('DB9Rule', db9RuleSchema);
var stateSchema = mongoose.Schema({
        time: {
            type: Number,
            required: true
        },
        value:{
            type: Number,
            required: true
        }
    });