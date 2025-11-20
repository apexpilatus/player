
const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");

function poweroff() {
    fetch(location.origin + "/poweroff");
}

function getcd() {
    location.assign(location.origin + "/tracks?scroll=0");
}

function playflac(dirtrack) {
    fetch(location.origin + "/playflac?" + dirtrack).then(resp => {
        if (resp.status != 200)
            playerElem.src = location.origin + "/stream_album?" + dirtrack;
        if (topalbumElem.innerHTML != dirtrack.split("&")[0]) {
            albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}

/*const controlElem = document.getElementById("control");
const albumsElem = document.getElementById("albums");
const scrollupElem = document.getElementById("scrollup");
const scrolldownElem = document.getElementById("scrolldown");
const selectedalbumElem = document.getElementById("selectedalbum");
const iconElem = document.getElementById("icon");
const positionElem = document.getElementById("position");

function updateposition() {
    positionElem.innerHTML = albumsElem.scrollTop;
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

*/
