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
                    const name = "hw:" + len;
                    fetch(location.origin + "/getvolume?name=" + name).then(resp => {
                        if (resp.status == 200) {
                            resp.text().then(txt => {
                                let vol = document.createElement("p");
                                let lab = document.createElement("label");
                                let br = document.createElement("br");
                                lab.setAttribute("for", name);
                                lab.innerHTML = txt;
                                let inp = document.createElement("input");
                                inp.type = "range";
                                inp.id = name;
                                inp.name = name;
                                inp.min = resp.statusText.split(";")[0];
                                inp.value = resp.statusText.split(";")[1];
                                inp.max = resp.statusText.split(";")[2];
                                inp.oninput = function () { setvolume(name); };
                                vol.appendChild(lab);
                                vol.appendChild(br);
                                vol.appendChild(inp);
                                document.body.appendChild(vol);
                            });
                        }
                    });
                }
            }
        }
    });
}
