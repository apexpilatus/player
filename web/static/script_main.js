const volumeElem = document.getElementById("volume");
const powerElem = document.getElementById("poweroff");
const getcdElem = document.getElementById("getcd");
const controlElem = document.getElementById("control");
const levelElem = document.getElementById("level");
const albumsElem = document.getElementById("albums");
const tracksElem = document.getElementById("tracks");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");
const timeout = 3000;
let timeId = null;

function getalbums() {
    albumsElem.src = window.location.href + "albums";
}

function scrolldown() {
    albumsElem.src = window.location.href + "albums?down";
}

function hidecontrol() {
    volumeElem.hidden = false;
    powerElem.hidden = false;
    getcdElem.hidden = false;
    scrollupElem.hidden = false;
    scrolldownElem.hidden = false;
    controlElem.hidden = true;
    levelElem.hidden = true;
}

function showcontrol() {
    volumeElem.hidden = true;
    powerElem.hidden = true;
    getcdElem.hidden = true;
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    controlElem.hidden = false;
    levelElem.hidden = false;
}

function getvolume() {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (timeId != null) {
                clearTimeout(timeId);
            }
            volumeElem.innerHTML = "&#9738";
            controlElem.max = resp.statusText.split("_")[2];
            controlElem.min = resp.statusText.split("_")[0];
            controlElem.value = resp.statusText.split("_")[1];
            levelElem.max = resp.statusText.split("_")[2];
            levelElem.value = resp.statusText.split("_")[1];
            timeId = setTimeout(hidecontrol, timeout);
            showcontrol()
        } else {
            volumeElem.innerHTML = "&#9739";
        }
    })
}

function setvolume() {
    let level = controlElem.value;
    fetch("setvolume&" + level).then(resp => {
        if (resp.status == 200) {
            levelElem.value = level;
            clearTimeout(timeId);
            timeId = setTimeout(hidecontrol, timeout);
        } else {
            hidecontrol();
            volumeElem.innerHTML = "&#9739";
        }
    })
}

function setdate() {
    fetch("setdate?" + new Date().getTime()).then(resp => {
        if (resp.status != 200) {
            window.alert("time was not updated");
        }
    });
}

function getcd() {
    tracksElem.hidden = true;
    tracksElem.src = parent.window.location.href + "cdcontrol";
}

function poweroff() {
    fetch("poweroff");
}