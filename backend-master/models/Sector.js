var mongoose = require('mongoose');
var sectorSchema = mongoose.Schema({
        name: {
            type: String,
            required: true
        },
        idOwner: {
            type: String,
            required: true
        },
        date: {
            type: Date,
            default: Date.now
        }
    });
exports.Sector = mongoose.model('Sector', sectorSchema);
