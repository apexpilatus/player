const controlElem = parent.document.getElementById("control");
const albumsElem = parent.document.getElementById("albums");
const selectedalbumElem = parent.document.getElementById("selectedalbum");
const topalbumElem = parent.document.getElementById("topalbum");
const playerElem = parent.document.getElementById("player");

function showtracks() {
    controlElem.className = "cdcontrol";
    controlElem.hidden = false;
}

function updateselected(album) {
    selectedalbumElem.innerHTML = album;
}

function updatetop(dir) {
    if (topalbumElem.innerHTML != dir) {
        albumsElem.src = parent.window.location.href + "albums?up";
    }
}

function playflac(dirtrack) {
    fetch(parent.window.location.href + "playflac?" + dirtrack).then(resp => {
        if (resp.status != 200)
            playerElem.src = parent.window.location.href + "stream_album?" + dirtrack;
        else
            updatetop(dirtrack.split("&")[0]);
    });
}