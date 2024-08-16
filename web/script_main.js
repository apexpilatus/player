const volumeElem = document.getElementById("volume");
const powerElem = document.getElementById("poweroff");
const control1Elem = document.getElementById("control1");
const control2Elem = document.getElementById("control2");
const albumsElem = document.getElementById("albums");
const timeout = 3000;
let timeId = null;

function getalbums() {
    albumsElem.src = window.location.href + "albums";
}

function hidecontrol() {
    volumeElem.hidden = false;
    powerElem.hidden = false;
    control1Elem.hidden = true;
    control2Elem.hidden = true;
}

function setcontrol(controlElem, resp) {
            controlElem.hidden = false;
            controlElem.max = resp.statusText.split("_")[2];
            controlElem.min = resp.statusText.split("_")[0];
            controlElem.value = resp.statusText.split("_")[1];
}

function getvolume() {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (timeId != null) {
                clearTimeout(timeId);
            }
            volumeElem.innerHTML = "&#9738";
            volumeElem.hidden = true;
            powerElem.hidden = true;
            setcontrol(control1Elem, resp);
            setcontrol(control2Elem, resp);
            timeId = setTimeout(hidecontrol, timeout);
        } else {
            volumeElem.innerHTML = "&#9739";
        }
    })
}

function setvolume(control1, control2) {
    let level = control1.value;
    fetch("setvolume&" + level).then(resp => {
        if (resp.status == 200) {
            control2.value = level;
            clearTimeout(timeId);
            timeId = setTimeout(hidecontrol, timeout);
        } else {
            hidecontrol();
            volumeElem.innerHTML = "&#9739";
        }
    })
}

function setvolume1() {
setvolume(control1Elem, control2Elem);
}

function setvolume2() {
setvolume(control2Elem, control1Elem);
}
