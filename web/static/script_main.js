
const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");

function getcd() {
    location.assign(location.origin + "/tracks?scroll=0");
}

function playflac(dirtrack) {
    fetch(location.origin + "/playflac?" + dirtrack).then(resp => {
        if (resp.status == 200) {
            playerElem.src = location.origin + "/stream_album?" + dirtrack;
        if (topalbumElem.innerHTML != dirtrack.split("&")[0])
            albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}

function playcd(track) {
    fetch(location.origin + "/playcd?" + track).then(resp => {
        if (resp.status == 200)
            playerElem.src = location.origin + "/stream_cd?" + track;
    });
}
