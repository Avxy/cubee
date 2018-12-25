var mongoose = require('mongoose');


var minuteSchema = mongoose.Schema({
    minute: {
        type: Number,
        required: true
    },
    measure:{
        type: Number,
        required: true
    }
});
exports.Minute = mongoose.model('Minute', minuteSchema);

var hourSchema = mongoose.Schema({
    hour: {
        type: Number,
        required: true
    },
    minutes: {
        type: [minuteSchema],
        required:true,
        default: []
    }
},{
    toObject: {
        virtuals : true
    }
},{
    toJSON: {
        virtuals : true
    }
});
hourSchema.virtual('sumHour').get(function () {
    var sum = 0;
    this.minutes.forEach(function (min) {
        sum += min.measure;
    });
    return sum;
});
exports.Hour = mongoose.model('Hour', hourSchema);

var measurementSchema = mongoose.Schema({
    date: {
        type: Date,
        required: true
    },
    idCubee: {
        type: String,
        required: true
    },
    hours:{
        type: [hourSchema],
        require: true,
        default: []
    }
},{
    toObject: {
        virtuals : true
    }
},{
    toJSON: {
        virtuals : true
    }
});
measurementSchema.virtual('hour.sum').get(function () {
    var sum = 0;
    this.hours.forEach(function (hour){
        sum += hour.sumHour;
    });
    return sum;
});
exports.Measurement = mongoose.model('Measurement', measurementSchema);

