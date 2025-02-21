const volumeElem = document.getElementById("volume");
const powerElem = document.getElementById("poweroff");
const getcdElem = document.getElementById("getcd");
const controlElem = document.getElementById("control");
const levelElem = document.getElementById("level");
const albumsElem = document.getElementById("albums");
const tracksElem = document.getElementById("tracks");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");
const hideTimeout = 3000;
let hideTimeId = setTimeout(function () {
    powerElem.hidden = false;
    getcdElem.hidden = false;
}, hideTimeout);
let canSetVol = false;
const volumeTimeout = 100;
let volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);

function getalbums() {
    scrollupElem.hidden = true;
    volumeElem.hidden = true;
    tracksElem.hidden = true;
    albumsElem.src = window.location.href + "albums";
}

function scrolldown() {
    scrolldownElem.hidden = true;
    volumeElem.hidden = true;
    tracksElem.hidden = true;
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
            clearTimeout(hideTimeId);
            volumeElem.innerHTML = "&#9738";
            controlElem.max = resp.statusText.split("_")[2];
            controlElem.min = resp.statusText.split("_")[0];
            controlElem.value = resp.statusText.split("_")[1];
            levelElem.max = resp.statusText.split("_")[2];
            levelElem.value = resp.statusText.split("_")[1];
            hideTimeId = setTimeout(hidecontrol, hideTimeout);
            showcontrol()
        } else {
            volumeElem.innerHTML = "&#9739";
        }
    });
}

function setlevel() {
    levelElem.value = controlElem.value;
    if (canSetVol) {
        canSetVol = false;
        clearTimeout(volumeTimeId);
        setvolume();
        volumeTimeId = setTimeout(function () { canSetVol = true }, volumeTimeout);
    }
}

function setvolume() {
    let level = controlElem.value;
    fetch("setvolume&" + level).then(resp => {
        clearTimeout(hideTimeId);
        if (resp.status != 200) {
            hidecontrol();
            volumeElem.innerHTML = "&#9739";
        } else
            hideTimeId = setTimeout(hidecontrol, hideTimeout);
    });
}

function setdate() {
    fetch("setdate?" + new Date().getTime());
}

function getcd() {
    tracksElem.hidden = true;
    tracksElem.src = parent.window.location.href + "cdcontrol";
    albumsElem.src = window.location.href + "albums";
}

function poweroff() {
    fetch("poweroff");
}