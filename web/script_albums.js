let current = "empty";

function gettracks(album) {
    if (current == album) {
        fetch("getvolume")
        .then(resp => {
            if (resp.status == 200) {
            if (resp.statusText.split("_")[1] == resp.statusText.split("_")[2]) {
                fetch("setvolume&" + resp.statusText.split("_")[0])
            }
            }
        })
        fetch("play?" + album)
            .then(resp => {
                if (resp.status == 200) {
                    location.reload();
                }
            })
    } else {
        current = album.slice();
        parent.document.getElementById("tracks").hidden = true;
        parent.document.getElementById("tracks").src = parent.window.location.href + "tracks?" + album;
    }
}
