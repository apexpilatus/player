const controlElem = parent.document.getElementById("control");
const levelElem = parent.document.getElementById("level");
const tracksElem = parent.document.getElementById("tracks");
const volumeElem = parent.document.getElementById("volume");
const scrollupElem = parent.document.getElementById("scrollup");
const scrolldownElem = parent.document.getElementById("scrolldown");
const currentElem = parent.document.getElementById("current");

function gettracks(album) {
    if (currentElem.innerHTML == album) {
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
        fetch("playflac?" + album).then(resp => {
            if (resp.status == 200) {
                window.location.assign(parent.window.location.href + "albums");
            }
        });
    }
    tracksElem.hidden = true;
    tracksElem.src = parent.window.location.href + "tracks?" + album;
}

function showscroll(initscroll) {
    if (initscroll == "down")
        window.scrollTo(0, document.body.scrollHeight);
    if (controlElem.hidden) {
        volumeElem.hidden = false;
        scrollupElem.hidden = false;
        scrolldownElem.hidden = false;
    }
}