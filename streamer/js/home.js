const albumsElem = document.getElementById("albums");
const tracksElem = document.getElementById("tracks");

function loadalbums(scroll) {
    if (scroll != null)
        albumsElem.src = location.origin + "/albums?scroll=" + scroll;
    else
        albumsElem.src = location.origin + "/albums";
}

function getmeta(album) {
    tracksElem.src = location.origin + "/tracks?album=" + album;
}

function getvolume() {
    fetch(location.origin + "/getvolume").then(resp => {
        if (resp.status == 200) {
            alert(resp.statusText);
        }
    });
}