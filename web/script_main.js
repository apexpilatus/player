const volumeElem = document.getElementById("volume");
const powerElem = document.getElementById("poweroff");
const controlElem = document.getElementById("control");
const levelElem = document.getElementById("level");
const albumsElem = document.getElementById("albums");
const timeout = 3000;
let timeId = null;

function getalbums() {
    albumsElem.src = window.location.href + "albums?" + new Date().getTime();
}

function hidecontrol() {
    volumeElem.hidden = false;
    powerElem.hidden = false;
    controlElem.hidden = true;
    levelElem.hidden = true;
}

function getvolume() {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (timeId != null) {
                clearTimeout(timeId);
            }
            volumeElem.hidden = true;
            volumeElem.innerHTML = "&#9738";
            powerElem.hidden = true;
            controlElem.hidden = false;
            controlElem.max = resp.statusText.split("_")[2];
            controlElem.min = resp.statusText.split("_")[0];
            controlElem.value = resp.statusText.split("_")[1];
            levelElem.hidden = false;
            levelElem.max = resp.statusText.split("_")[2];
            levelElem.value = resp.statusText.split("_")[1];
            timeId = setTimeout(hidecontrol, timeout);
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
