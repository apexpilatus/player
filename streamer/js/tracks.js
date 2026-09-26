const topalbumElem = parent.document.getElementById("topalbum");
const albumsElem = parent.document.getElementById("albums");

function play(album, tracknum) {
    fetch(location.origin + "/stream?album=" + album + "&track=" + tracknum).then(resp => {
        if (resp.status == 200) {
            fetch(location.origin + "/touch?album=" + album).then(resp => {
                if (resp.status == 200) {
                    if (topalbumElem.innerHTML != album)
                        albumsElem.src = location.origin + "/albums?scroll=0";
                }
            });
        }
    });
}

function getmeta(album) {
    fetch(location.origin + "/files?album=" + album).then(resp => {
        if (resp.status == 200) {
            addEventListener('wheel', (event) => {
                event.preventDefault();
                document.documentElement.scrollLeft += event.deltaY;
            }, { passive: false });
            resp.text().then(tracks => gettracks(album, tracks.split("\r\n"), 0));
        }
    });
}

function gettracks(album, tracks, track) {
    if (track < tracks.length)
        fetch(location.origin + "/meta?album=" + album + "&tag=TRACKNUMBER=&file=" + tracks[track]).then(resp => {
            if (resp.status == 200) {
                resp.text().then(num => {
                    let tr = document.createElement("b");
                    tr.innerHTML = num;
                    tr.onclick = function () { play(album, track); };
                    document.body.appendChild(tr);
                    gettracks(album, tracks, track + 1);
                });
            }
        });
}