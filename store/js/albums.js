const iconElem = parent.document.getElementById("icon");
const topalbumElem = parent.document.getElementById("topalbum");
const playerElem = parent.document.getElementById("player");

let scroll = null;

function gettracks(album) {
        if (iconElem != null && iconElem.href != location.origin + "/picture?album=" + album)
                parent.location.assign(location.origin + "/?album=" + album + "&scroll=" + pageXOffset);
        else if (playerElem != null)
                playerElem.src = location.origin + "/stream?album=" + album;
}

function updatetop(album) {
        if (topalbumElem != null)
                topalbumElem.innerHTML = album;
}

function setscroll(value) {
        scroll = value;
}

function listalbums(albs) {
        const albums = albs.split("/");
        let len = albums.length;
        if (len > 0) {
                document.body.style.filter = "grayscale(100%)";
                document.body.onload = function () { loaded(); };
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
}

function loaded() {
        if (scroll == null)
                scroll = document.body.scrollWidth / 2;
        scrollTo(scroll, 0);
        document.body.style.filter = "none";
        addEventListener('wheel', (event) => {
                event.preventDefault();
                document.documentElement.scrollLeft += event.deltaY;
        }, { passive: false });
}
