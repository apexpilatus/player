function showtracks() {
    parent.document.getElementById("tracks").hidden = false;
}

function play(dirtrack) {
    fetch("getvolume").then(resp => {
        if (resp.status == 200) {
            if (resp.statusText.split("_")[1] == resp.statusText.split("_")[2]) {
                fetch("setvolume&" + resp.statusText.split("_")[0])
            }
        }
    })
    fetch("play?" + dirtrack);
}
