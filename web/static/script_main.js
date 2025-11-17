const controlElem = document.getElementById("control");
const albumsElem = document.getElementById("albums");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");
const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const selectedalbumElem = document.getElementById("selectedalbum");
const iconElem = document.getElementById("icon");

function updatetop() {
    if (playerElem.src.indexOf("stream_cd") < 0 && playerElem.src.split("?")[1].split("&")[0] != topalbumElem.innerHTML)
        albumsElem.src = location.origin + "/albums?up";
}

function loaddefault() {
    if (iconElem.title != "default")
        location = location.origin
}

function getalbums() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = location.origin + "/albums";
}

function scrollup() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = location.origin + "/albums?up";
}

function scrolldown() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = location.origin + "/albums?down";
}

function getcd() {
    selectedalbumElem.innerHTML = "cd";
    controlElem.hidden = true;
    controlElem.src = location.origin + "/cdcontrol";
}

function poweroff() {
    fetch(location.origin + "/poweroff");
}