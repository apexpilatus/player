const control1Elem = parent.document.getElementById("control1");
const control2Elem = parent.document.getElementById("control2");
const tracksElem = parent.document.getElementById("tracks");

function showtracks() {
    tracksElem.hidden = false;
}

function play(dirtrack) {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (resp.statusText.split("_")[1] == resp.statusText.split("_")[2]) {
                fetch("setvolume&" + resp.statusText.split("_")[0]);
                if (!control1Elem.hidden) {
                    control1Elem.value = resp.statusText.split("_")[0];
                }
                if (!control2Elem.hidden) {
                    control2Elem.value = resp.statusText.split("_")[0];
                }
            }
        }
    })
    fetch("play?" + dirtrack);
}
