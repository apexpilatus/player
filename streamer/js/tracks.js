let track = 1;
const topalbumElem = parent.document.getElementById("topalbum");

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
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200) {
            addEventListener('wheel', (event) => {
                event.preventDefault();
                document.documentElement.scrollLeft += event.deltaY;
            }, { passive: false });
            gettracks(album);
        } else if (track < 100) {
            track++;
            getmeta(album);
        }
    });
}

function gettracks(album) {
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200) {
            let tr = document.createElement("b");
            tr.innerHTML = track;
            const tracknum = track;
            tr.onclick = function () { play(album, tracknum); };
            document.body.appendChild(tr);
            track++;
            gettracks(album);
        }
    });
}