const playerElem = document.getElementById("player");
const topalbumElem = document.getElementById("topalbum");
const albumsElem = document.getElementById("albums");

let track = 1;

function playflac(dirtrack) {
    fetch(location.origin + "/touch?" + dirtrack.split("&")[0]).then(resp => {
        if (resp.status == 200) {
            playerElem.src = location.origin + "/stream_album?" + dirtrack;
            if (topalbumElem.innerHTML != dirtrack.split("&")[0])
                albumsElem.src = location.origin + "/albums?scroll=0";
        }
    });
}

function gettracks(album) {
    fetch(location.origin + "/meta?album=" + album + "&meta=TITLE=&track=" + track).then(resp => {
        if (resp.status == 200) {
            resp.text().then(txt => alert(txt));
            track++;
            gettracks(album);
        }
    });
}

function getmeta(album) {
    gettracks(album);
}

function loadalbums(params) {
    albumsElem.src = location.origin + "/albums?" + params;
}