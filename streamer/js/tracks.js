let track = 1;

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
            document.body.appendChild(tr);
            track++;
            gettracks(album);
        }
    });
}