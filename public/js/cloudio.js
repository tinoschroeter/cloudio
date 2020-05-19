let data, url, toggle=true;

function preload() {
    url ='https://us-central1-tino-test-41bd3.cloudfunctions.net/api';
    data = loadJSON(url, "json");
}

function setup() {
    frameRate(60);
    noCanvas();
}

function draw() {
    if(frameCount % 60 === 0) {
        if(data.hum > 0) {
            document.getElementById('luft').innerHTML = data.gas + ' %';
            document.getElementById('temp').innerHTML = data.tmp + ' C';
            document.getElementById('feucht').innerHTML = data.hum + ' %H';
            data = loadJSON(url, "json");
        }

        if(data.gas > data.gas_alarm) {
            document.getElementById('cloudio').innerHTML = 'Fenster auf!';    
        } else if(data.tmp_alarm_down < data.tmp) {
            document.getElementById('cloudio').innerHTML = 'Fenster zu!';
        } else {
            if(toggle) {
                document.getElementById('cloudio').innerHTML = 'Cloudio';
                toggle = false;
            } else {
                document.getElementById('cloudio').innerHTML = 'Cloudio +';
                toggle = true;
            }
        }
    }
}
