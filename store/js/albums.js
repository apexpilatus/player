const iconElem = parent.document.getElementById("icon");
const topalbumElem = parent.document.getElementById("topalbum");

function gettracks(album) {
        if (iconElem.href != location.origin + "/picture?album=" + album)
                parent.location.assign(location.origin + "/?" + "album=" + album + "&scroll=" + pageYOffset);
}

function updatetop(album) {
        topalbumElem.innerHTML = album;
}

function listalbums(albs) {
        const albums = albs.split(";");
        let len = albums.length;
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

function loaded(scroll) {
        scrollTo(0, scroll);
        document.body.className = "reg";
}
