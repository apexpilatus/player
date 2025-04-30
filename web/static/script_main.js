const controlElem = document.getElementById("control");
const albumsElem = document.getElementById("albums");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");

function getalbums() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums";
}

function scrolldown() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums?down";
}

function getvolume() {
    controlElem.hidden = true;
    controlElem.src = window.location.href + "getvolume";
}

function setdate() {
    fetch("setdate?" + new Date().getTime());
}

function getcd() {
    controlElem.hidden = true;
    controlElem.src = window.location.href + "cdcontrol";
}

function poweroff() {
    fetch("poweroff");
}