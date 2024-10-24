const controlElem = parent.document.getElementById("control");
const levelElem = parent.document.getElementById("level");
const tracksElem = parent.document.getElementById("tracks");
const albumsElem = parent.document.getElementById("albums");
let notPlaying = true;

function showtracks() {
    tracksElem.hidden = false;
}

function playflac(dirtrack) {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (resp.statusText.split("_")[1] == resp.statusText.split("_")[2]) {
                fetch("setvolume&" + resp.statusText.split("_")[0]);
                if (!controlElem.hidden) {
                    controlElem.value = resp.statusText.split("_")[0];
                }
                if (!levelElem.hidden) {
                    levelElem.value = resp.statusText.split("_")[0];
                }
            }
        }
    });
    fetch("playflac?" + dirtrack).then(resp => {
        if (resp.status == 200 && notPlaying) {
            notPlaying = false;
            albumsElem.src = parent.window.location.href + "albums";
        }
    });
}