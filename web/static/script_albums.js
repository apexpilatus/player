const controlElem = parent.document.getElementById("control");
const levelElem = parent.document.getElementById("level");
const tracksElem = parent.document.getElementById("tracks");
let current = "empty";

function gettracks(album) {
    if (current == album) {
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
        })
        fetch("playflac?" + album).then(resp => {
            if (resp.status == 200) {
                location.reload();
            }
        })
    } else {
        current = album.slice();
    }
    tracksElem.hidden = true;
    tracksElem.src = parent.window.location.href + "tracks?" + album;
}