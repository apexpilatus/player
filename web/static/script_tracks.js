const controlElem = parent.document.getElementById("control");
const albumsElem = parent.document.getElementById("albums");
const currentElem = parent.document.getElementById("current");
const topElem = parent.document.getElementById("top");

function showtracks() {
    controlElem.hidden = false;
}

function updatecurrent(album) {
    currentElem.innerHTML = album;
}

function updatetop(dir) {
    if (topElem.innerHTML != dir) {
        albumsElem.src = "albums?up";
    }
}

function playflac(dirtrack) {
    fetch("playflac?" + dirtrack).then(resp => {
        if (resp.status == 200) {
            updatetop(dirtrack.split("&")[0]);
        }
    });
}
