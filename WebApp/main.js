var devices = new Array();

const urlParams = new URLSearchParams(decodeURIComponent(window.location.search));

var isApp = true;
try {
    if (Android === undefined) {
        isApp = false;
    }
} catch (error) {
    isApp = false;
}

function loadConfig() {
    if (localStorage.hasOwnProperty('devices') || urlParams.has("devices")) {
        if (localStorage.hasOwnProperty('devices')) {
            devices = JSON.parse(localStorage.getItem('devices'));
        }
        if (urlParams.has("devices")) {
            devices = JSON.parse(urlParams.get('devices'));
            localStorage.setItem('devices', JSON.stringify(devices));
            window.history.replaceState({}, document.title, "/");
        }
        currentPage = 'dashboard.html';
        $('#page').load('dashboard.html');
        document.getElementById("dashboard.html-li").classList.add("active");
    } else {
        $('#page').load('helloworld.html');
    }
}

function createDevice(name, ip, key) {
    devices.push({
        "name": name,
        "ip": ip,
        "key": key
    });
    localStorage.setItem('devices', JSON.stringify(devices));
}

function renderDevices() {
    //console.log(devices);
    for (var i = 0; i < devices.length; i++) {
        var device = devices[i];
        addDevice(device.ip, device.name, i);
    }
}

function sendRequst(url, deviceid) {
    console.log(url);
    $.get(url)
        .done(function (data) {
            console.log(data);
        })
        .fail(function (data) {
            $.toast({
                title: 'Error',
                content: `An error occurred while contacting <strong>${devices[deviceid].name}</strong>`,
                type: 'error',
                delay: 2000,
                pause_on_hover: false
            });
        });
}

function addDevice(ip, name, deviceid) {
    var box = `
    <div class="col-md-auto mb-3 mt-3">
    <div class="card text-center" style="width: 18rem;">
        <div class="card-header">
            <h5 class="card-title">${name} <span id="status-${deviceid}" class="badge badge-success">Online</span></h5>
        </div>
        <div class="card-body">
            <div id="picker${deviceid}"></div><br>
            <label for="customRange1">Speed</label>
            <input type="range" class="custom-range" id="customRange1" min="300" onchange="sendRequst('https://${ip}/setspeed?speed='+this.value, ${deviceid})" max="10000">
            <button type="button" class="btn btn-primary" onclick="sendRequst('https://${ip}/ledoff', ${deviceid})">Off</button>
            <button type="button" class="btn btn-primary" onclick="sendRequst('https://${ip}/rainbow', ${deviceid})">Rainbow</button>
        </div>
    </div>
    </div>
    `;
    var boxdiv = document.createElement('div');
    boxdiv.classList.add("mres");
    boxdiv.innerHTML = box;
    document.getElementById("container").appendChild(boxdiv);

    new iro.ColorPicker(`#picker${deviceid}`, {
        width: 200,
        color: "#f00",
        display: "inline-block"
    }).on('color:change', function (color) {
        sendRequst(`https://${ip}/setled?r=${color.rgb.r}&g=${color.rgb.g}&b=${color.rgb.b}`, deviceid);
    });

    ping();

    var pingInterval = setInterval(function () {
        if (currentPage != "dashboard.html") {
            clearInterval(pingInterval);
        }
        ping();
    }, 15000);

    function ping() {
        var element = document.getElementById("status-" + deviceid);
        $.get(`https://${ip}/ping`)
            .done(function (data) {
                if (data.status == "success" && data.action == "ping") {
                    element.innerHTML = "Online";
                    element.classList.add("badge-success");
                    element.classList.remove("badge-danger");
                } else {
                    element.innerHTML = "Invalid";
                    element.classList.remove("badge-success");
                    element.classList.add("badge-warning");
                }
            })
            .fail(function (data) {
                element.innerHTML = "Offline";
                element.classList.remove("badge-success");
                element.classList.add("badge-danger");
            });
    }
}