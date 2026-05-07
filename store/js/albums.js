const iconElem = parent.document.getElementById("icon");
const playerElem = parent.document.getElementById("player");
const topalbumElem = parent.document.getElementById("topalbum");

let scroll = 0;

function gettracks(album) {
        if (iconElem.href != location.origin + "/picture?album=" + album)
                parent.location.assign(location.origin + "/?" + "album=" + album + "&scroll=" + pageYOffset);
        else
                playerElem.src = "stream?" + album;
}

function updatetop(album) {
        topalbumElem.innerHTML = album;
}

function listalbums(albs) {
        const albums = albs.split("/");
        let len = albums.length;
        document.body.style.filter = "grayscale(100%)";
        document.body.onload = function () { loaded(); }
        updatetop(albums[--len]);
        for (len; len >= 0; len--) {
                let img = document.createElement("img");
                img.src = "picture?album=" + albums[len];
                const album = albums[len];
                img.onclick = function () { gettracks(album); };
                img.alt = "picture";
                document.body.appendChild(img);
        }
}

function loaded() {
        scrollTo(0, scroll);
        document.body.style.filter = "none";
}
