const iconElem = parent.document.getElementById("icon");
const topalbumElem = parent.document.getElementById("topalbum");

function gettracks(album) {
        if (iconElem.href != location.origin + album)
                parent.location.assign(location.origin + "/tracks?" + "album=" + album + "&scroll=" + pageYOffset);
}

function updatetop(album) {
        topalbumElem.innerHTML = album;
}
