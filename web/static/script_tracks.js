const controlElem = parent.document.getElementById("control");
const albumsElem = parent.document.getElementById("albums");
const selectedalbumElem = parent.document.getElementById("selectedalbum");
const topalbumElem = parent.document.getElementById("topalbum");
const playerElem = parent.document.getElementById("player");

function showtracks() {
    controlElem.className = "tracks";
    controlElem.hidden = false;
}

function updateselected(album) {
    selectedalbumElem.innerHTML = album;
}

function updatetop(dir) {
    if (topalbumElem.innerHTML != dir) {
        albumsElem.src = location.origin + "/albums?up";
    }
}

function playflac(dirtrack) {
    fetch(location.origin + "/playflac?" + dirtrack).then(resp => {
        if (resp.status != 200)
            playerElem.src = location.origin + "/stream_album?" + dirtrack;
        else
            updatetop(dirtrack.split("&")[0]);
    });
}