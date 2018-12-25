var mongoose = require('mongoose');
var userSchema = mongoose.Schema({
        _id: {
            type: String,
            required: true,
            unique: true
        },
        name: {
            type: String,
            required: true,
            trim: true
        },
        email: {
            type: String,
            required: true
        },
        telephone: {
            type: String
        },
        registrationToken: {
            type: String
        }
    });
exports.User = mongoose.model('User', userSchema);
