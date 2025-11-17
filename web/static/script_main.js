const controlElem = document.getElementById("control");
const albumsElem = document.getElementById("albums");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");
const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const selectedalbumElem = document.getElementById("selectedalbum");

function updatetop() {
    if (playerElem.src.indexOf("stream_cd") < 0 && playerElem.src.split("?")[1].split("&")[0] != topalbumElem.innerHTML)
        albumsElem.src = window.location.href + "albums?up";
}

function getalbums() {
    alert(window.location.origin)
    /*scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums";*/
}

function scrollup() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums?up";
}

function scrolldown() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = window.location.href + "albums?down";
}

function getcd() {
    selectedalbumElem.innerHTML = "cd";
    controlElem.hidden = true;
    controlElem.src = window.location.href + "cdcontrol";
}

function poweroff() {
    fetch(window.location.href + "poweroff");
}