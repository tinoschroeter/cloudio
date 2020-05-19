const functions = require('firebase-functions');
const admin = require('firebase-admin');
const axios = require('axios');

admin.initializeApp(functions.config().firebase);

// simple prometheus exporter
exports.metrics = functions.https.onRequest((req, res) => {
    return admin.database().ref('Data').once('value', (snapshot) => {
        let event = snapshot.val();
        res.set('Content-Type', 'text/plain');
        res.send(`# cloudio IoT Device
# TYPE cloudio_luftqualitaet gauge\ncloudio_luftqualitaet ${event.gas}
# TYPE cloudio_temp gauge\ncloudio_temp ${event.tmp}
# TYPE cloudio_luftfeuchte gauge\ncloudio_luftfeuchte ${event.hum}
# TYPE cloudio_led gauge\ncloudio_led ${event.led}
            `);
    });
});

// web api
const cors = require('cors')({origin: true}); // disable cors 
exports.api = functions.https.onRequest((req, res) => {
    return admin.database().ref('Data').once('value', (snapshot) => {
        let event = snapshot.val();
        cors(req, res, () => {
            res.set('Content-Type', 'text/json');
            res.send(`{ 
                "gas": "${event.gas}",
                    "tmp": "${event.tmp}", 
                    "hum": "${event.hum}", 
                    "time": "${event.time}", 
                    "led": "${event.led}",
                    "gas_alarm": "${event.gas_alarm}",
                    "tmp_alarm_up": "${event.tmp_alarm_up}",
                    "tmp_alarm_down": "${event.tmp_alarm_down}",
                    "hum_alarm_up": "${event.hum_alarm_up}",
                    "hum_alarm_down": "${event.hum_alarm_down}"
            }`);
        });
    });
});

// slack integration
function setSlackStatus(value) {
    admin.database().ref('Data/slack_alarm').set(value);
}

exports.slack = functions.database.ref('Data').onWrite((snapshot, context) => {
    return admin.database().ref('Data').once('value', (snapshot) => {
        let value = snapshot.val();
        let slackHook = value.slack_hook;
        let slackAlarm = (value.slackAlarm === "false");
        let slackActive = (value.slackActive === "true")
        let valueToHeigh = (value.gas < 100);
        if(value.gas > 0 && value.gas < value.gasAlarm && slackAlarm && valueToHeigh && slackActive) {
            axios.post(slackHook, {
                text: '> Es wird Zeit zum lueften ' + value.gas + '%'
            })
                .catch((error) => {
                    console.error(error)
                })
            setSlackStatus("true");
        } else if(value.gas > 0 && value.gas > value.gasAlarm + 20) {
            setSlackStatus("false");
        }
    });
});

// cron job 
exports.cron = functions.pubsub.schedule('every 5 minutes').onRun((context) => {
    console.log('This will be run every 5 minutes!');
});
