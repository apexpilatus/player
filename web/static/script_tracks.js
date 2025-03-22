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

function playflac(dirtrack) {
    fetch("playflac?" + dirtrack).then(resp => {
        if (resp.status == 200 && topElem.innerHTML != dirtrack.split("&")[0]) {
            albumsElem.src = parent.window.location.href + "albums";
        }
    });
}