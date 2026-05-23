const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");
const artistElem = document.getElementById("artist");
const albumElem = document.getElementById("album");
const tracksElem = document.getElementById("tracks");

let track = 1;

function play(album, tracknum) {
    fetch(location.origin + "/touch?album=" + album).then(resp => {
        if (resp.status == 200) {
            playerElem.src = location.origin + "/stream?album=" + album + "&track=" + tracknum;
            if (playerElem.onended == null)
                playerElem.onended = function () { location = location.origin; };
            if (topalbumElem.innerHTML != album)
                albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}

function gettracks(album) {
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => {
                let tr = document.createElement("tr");
                const tracknum = track;
                tr.onclick = function () { play(album, tracknum); }
                let num = document.createElement("td");
                if (track < 10)
                    num.innerHTML = "&nbsp;&nbsp;" + track;
                else
                    num.innerHTML = track;
                num.className = "tracknumber";
                tr.append(num);
                let title = document.createElement("td");
                title.innerHTML = txt;
                title.className = "tracktitle";
                tr.append(title);
                tracksElem.appendChild(tr);
                track++;
                gettracks(album);
            });
        }
    });
}

function gettitle(album) {
    fetch(location.origin + "/meta?album=" + album + "&meta=ARTIST=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => artistElem.innerHTML = txt);
            getalbum(album);
        } else if (track < 100) {
            track++;
            gettitle(album);
        }
    });
}

function getalbum(album) {
    fetch(location.origin + "/meta?album=" + album + "&meta=ALBUM=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => albumElem.innerHTML = txt);
            gettracks(album);
        }
    });
}

function getmeta(album) {
    gettitle(album);
}

function loadalbums(scroll) {
    if (scroll != null)
        albumsElem.src = location.origin + "/albums?scroll=" + scroll;
    else
        albumsElem.src = location.origin + "/albums";
}
