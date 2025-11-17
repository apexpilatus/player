const controlElem = getElementById("control");
const albumsElem = getElementById("albums");
const scrollupElem = getElementById("scrollup");
const scrolldownElem = getElementById("scrolldown");
const playerElem = getElementById("player");
const topalbumElem = getElementById("topalbum");
const selectedalbumElem = getElementById("selectedalbum");

function updatetop() {
    if (playerElem.src.indexOf("stream_cd") < 0 && playerElem.src.split("?")[1].split("&")[0] != topalbumElem.innerHTML)
        albumsElem.src = window.location.href + "albums?up";
}

function getalbums() {
    scrollupElem.hidden = true;
    scrolldownElem.hidden = true;
    albumsElem.src = location.origin + "/albums";
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