const controlElem = document.getElementById("control");
const currentElem = document.getElementById("current");
const albumsElem = document.getElementById("albums");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");

function getalbums() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums";
}

function scrollup() {
    if (currentElem.innerHTML == "volume")
        getalbums();
    else
        window.location.assign(window.location.href);
}

function scrolldown() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums?down";
}

function getcd() {
    currentElem.innerHTML = "cd";
    controlElem.hidden = true;
    controlElem.src = window.location.href + "cdcontrol";
}

function poweroff() {
    fetch("poweroff");
}