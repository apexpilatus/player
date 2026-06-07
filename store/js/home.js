const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");
const artistElem = document.getElementById("artist");
const albumElem = document.getElementById("album");
const tracksElem = document.getElementById("tracks");
const playerElem = document.getElementById("player");

function playnext(album, track) {
    playerElem.src = location.origin + "/fetch?album=" + album + "&track=" + track;
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200)
            playerElem.onended = function () { playnext(album, track + 1); };
        else
            location = location.origin;
    });
}

function play(album, track) {
    fetch(location.origin + "/touch?album=" + album).then(resp => {
        if (resp.status == 200) {
            playerElem.src = location.origin + "/fetch?album=" + album + "&track=" + track;
            playerElem.onended = function () { playnext(album, track + 1); };
            if (topalbumElem.innerHTML != album)
                albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}

function gettracks(album, track) {
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => {
                let tr = document.createElement("tr");
                tr.onclick = function () { play(album, track); };
                let num = document.createElement("td");
                if (track < 10)
                    num.innerHTML = "&nbsp;&nbsp;" + track;
                else
                    num.innerHTML = track;
                num.className = "trackber";
                tr.append(num);
                let title = document.createElement("td");
                title.innerHTML = txt;
                title.className = "tracktitle";
                tr.append(title);
                tracksElem.appendChild(tr);
                gettracks(album, track + 1);
            });
        }
    });
}

function gettitle(album, track) {
    fetch(location.origin + "/meta?album=" + album + "&meta=ARTIST=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => artistElem.innerHTML = txt);
            getalbum(album, track);
        } else if (track < 100) {
            gettitle(album, track + 1);
        }
    });
}

function getalbum(album, track) {
    fetch(location.origin + "/meta?album=" + album + "&meta=ALBUM=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => albumElem.innerHTML = txt);
            gettracks(album, track);
        }
    });
}

function getmeta(album) {
    gettitle(album, 1);
}

function loadalbums(scroll) {
    if (scroll != null)
        albumsElem.src = location.origin + "/albums?scroll=" + scroll;
    else
        albumsElem.src = location.origin + "/albums";
}