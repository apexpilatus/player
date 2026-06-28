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
    fetch(location.origin + "/getcards").then(resp => {
        if (resp.status == 200) {
            const cards = resp.statusText.split(" ");
            let len = cards.length;
            if (len > 0) {
                for (--len; len >= 0; len--) {
                    fetch(location.origin + "/getvolume?name=hw:" + len).then(resp => {
                        if (resp.status == 200) {
                            resp.text().then(txt => {
                                let vol = document.createElement("p");
                                vol.innerHTML = txt;
                                document.body.appendChild(vol);
                            });
                        }
                    });
                }
            }
        }
    });
}
