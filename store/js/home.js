const albumsElem = document.getElementById("albums");
const artistElem = document.getElementById("artist");
const albumElem = document.getElementById("album");
const tracksElem = document.getElementById("tracks");
const playerElem = document.getElementById("player");

function playnext(album, tracks, track) {
    if (track < tracks.length) {
        playerElem.src = location.origin + "/fetch?album=" + album + "&file=" + tracks[track];
        if (track + 1 < tracks.length)
            playerElem.onended = function () { playnext(album, tracks, track + 1); };
        else
            playerElem.onended = null;
    }
}

function play(album, tracks, track) {
    if (track < tracks.length) {
        playerElem.src = location.origin + "/fetch?album=" + album + "&file=" + tracks[track];
        if (track + 1 < tracks.length)
            playerElem.onended = function () { playnext(album, tracks, track + 1); };
    }
}

function getmeta(album) {
    fetch(location.origin + "/files?album=" + album).then(resp => {
        if (resp.status == 200) {
            resp.text().then(tracks => getartist(album, tracks.split("\r\n")));
        }
    });
}

function getartist(album, tracks) {
    fetch(location.origin + "/meta?album=" + album + "&tag=ARTIST=&file=" + tracks[0]).then(resp => {
        if (resp.status == 200) {
            resp.text().then(artist => artistElem.innerHTML = artist);
        }
    });
    getalbum(album, tracks);
}

function getalbum(album, tracks) {
    fetch(location.origin + "/meta?album=" + album + "&tag=ALBUM=&file=" + tracks[0]).then(resp => {
        if (resp.status == 200) {
            resp.text().then(title => { if (title != artistElem.innerHTML) albumElem.innerHTML = title });
        }
    });
    gettracks(album, tracks, 0);
}

function gettracks(album, tracks, track) {
    if (track < tracks.length) {
        let tr = document.createElement("tr");
        tr.onclick = function () { play(album, tracks, track); };
        let trnum = document.createElement("td");
        trnum.className = "tracknumber";
        tr.append(trnum);
        let trname = document.createElement("td");
        trname.className = "tracktitle";
        tr.append(trname);
        tracksElem.appendChild(tr);
        fetch(location.origin + "/meta?album=" + album + "&tag=TITLE=&file=" + tracks[track]).then(resp => {
            if (resp.status == 200) {
                resp.text().then(title => {
                    fetch(location.origin + "/meta?album=" + album + "&tag=TRACKNUMBER=&file=" + tracks[track]).then(resp => {
                        if (resp.status == 200)
                            resp.text().then(num => {
                                trnum.innerHTML = num;
                                trname.innerHTML = title;
                            });
                    });
                });
            }
        });
        gettracks(album, tracks, track + 1);
    }
}

function loadalbums(scroll) {
    if (scroll != null)
        albumsElem.src = location.origin + "/albums?scroll=" + scroll;
    else
        albumsElem.src = location.origin + "/albums";
}
